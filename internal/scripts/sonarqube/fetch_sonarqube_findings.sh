#!/bin/bash
#
# Fetch SonarQube findings (issues) for a branch via the Web API
#=======================================================================
#
# The sonar-scanner CLI only uploads an analysis report to the SonarQube
# server -- it does not print or return the resulting findings. The
# server processes that report asynchronously (the "Compute Engine" task)
# and only afterward are issues available, via the SonarQube dashboard or
# its Web API.
#
# This script pulls the findings for a branch via the Web API and writes
# them to local files for review. It is intended to be run locally by a
# developer with their own SONAR_TOKEN. Do NOT wire this into CI or
# publish its output as a build artifact -- unlike the dashboard (which
# requires a SonarQube login), CI artifacts on a public repo are visible
# to anyone who can view the workflow run.
#
# The /api/issues/search endpoint refuses to page past 10000 results
# (page * page_size > 10000 is rejected), regardless of how many issues
# actually match, because it is backed by Elasticsearch's default
# index.max_result_window. When a branch has more than 10000 matching
# issues, this script automatically splits the query by severity, and
# (if a single severity is still too large) further by type, fetching
# each slice separately and merging the results back together.
#
# Deep pagination against Elasticsearch (large "from" offsets) gets more
# expensive the deeper you page, and this endpoint is backed by an
# embedded, often memory-constrained Elasticsearch node. Hammering it
# with many such requests back-to-back has been observed to destabilize
# a SonarQube server. To keep this script a good citizen:
#   - Requests are rate-limited (SONAR_REQUEST_DELAY between requests).
#   - The page size defaults to a modest value, not the API's max of 500.
#   - SONAR_SEVERITIES / SONAR_TYPES let you fetch only what you need
#     right now (e.g. just BLOCKER) instead of every issue on the branch,
#     which avoids deep pagination altogether for small slices.
#
# Usage: fetch_sonarqube_findings.sh branch [outdir]
#    where "branch"  specifies the sonar.branch.name that was analyzed
#          "outdir"  specifies the output directory (default: .)
#
# Required Environment Variables:
#   SONAR_HOST_URL
#   SONAR_TOKEN
#
# Optional Environment Variables:
#   SONAR_COMPONENT_KEY   (default: MET)
#   SONAR_SEVERITIES      Comma-separated subset of BLOCKER, CRITICAL,
#                         MAJOR, MINOR, INFO to fetch (default: all)
#   SONAR_TYPES           Comma-separated subset of BUG, VULNERABILITY,
#                         CODE_SMELL to fetch (default: all)
#   SONAR_PAGE_SIZE       Issues per request, max 500 (default: 100)
#   SONAR_REQUEST_DELAY   Seconds to sleep between requests (default: 1)
#
# Requires: curl, jq
#
#=======================================================================

function usage {
  echo
  echo "USAGE: $(basename $0) branch [outdir]"
  echo "   where \"branch\" specifies the sonar.branch.name that was analyzed"
  echo "         \"outdir\" specifies the output directory (default: .)"
  echo
  return 0
}

# Check for arguments
if [[ $# -lt 1 ]]; then usage; exit 1; fi

BRANCH=$1
OUTDIR=${2:-.}
COMPONENT_KEY=${SONAR_COMPONENT_KEY:-MET}
PAGE_SIZE=${SONAR_PAGE_SIZE:-100}
REQUEST_DELAY=${SONAR_REQUEST_DELAY:-1}
MAX_RESULT_WINDOW=10000

# SonarQube's fixed sets of severity and type values, used only to split
# up an *unfiltered* query that turns out to exceed MAX_RESULT_WINDOW.
SEVERITIES=(BLOCKER CRITICAL MAJOR MINOR INFO)
TYPES=(BUG VULNERABILITY CODE_SMELL)

# Check required environment variables
if [[ -z "$SONAR_HOST_URL" ]]; then
  echo "ERROR: $(basename $0) -> \$SONAR_HOST_URL not defined!" >&2
  exit 1
fi
if [[ -z "$SONAR_TOKEN" ]]; then
  echo "ERROR: $(basename $0) -> \$SONAR_TOKEN not defined!" >&2
  exit 1
fi
if ! command -v jq >/dev/null 2>&1; then
  echo "ERROR: $(basename $0) -> jq is required but was not found in PATH" >&2
  exit 1
fi

mkdir -p ${OUTDIR}
SAFE_BRANCH=$(echo ${BRANCH} | sed 's%/%_%g')
JSON_FILE=${OUTDIR}/sonarqube_findings_${SAFE_BRANCH}.json
TXT_FILE=${OUTDIR}/sonarqube_findings_${SAFE_BRANCH}.txt
RAW_FILE=$(mktemp)
> ${RAW_FILE}

BASE_QUERY="componentKeys=${COMPONENT_KEY}&branch=${BRANCH}&resolved=false"

# Fold a caller-requested severity/type filter directly into every
# request (the API accepts comma-separated values for both params). This
# both narrows the result set and, when it's small enough, sidesteps
# the deep-pagination splitting logic below entirely.
FILTERED=0
if [[ -n "${SONAR_SEVERITIES}" ]]; then
  BASE_QUERY="${BASE_QUERY}&severities=${SONAR_SEVERITIES}"
  FILTERED=1
fi
if [[ -n "${SONAR_TYPES}" ]]; then
  BASE_QUERY="${BASE_QUERY}&types=${SONAR_TYPES}"
  FILTERED=1
fi

# api_get extra_params -> prints the JSON response on stdout, returns
# non-zero on failure (after printing the server's error body to stderr).
# Rate-limited by SONAR_REQUEST_DELAY to avoid overloading the server.
function api_get {
  local extra="$1"
  local url="${SONAR_HOST_URL}/api/issues/search?${BASE_QUERY}&${extra}"
  local response

  sleep ${REQUEST_DELAY}

  response=$(curl -s -f -u "${SONAR_TOKEN}:" "${url}")
  if [[ $? -ne 0 ]]; then
    echo "ERROR: $(basename $0) -> request failed: ${url}" >&2
    curl -s -u "${SONAR_TOKEN}:" "${url}" >&2
    return 1
  fi
  echo "${response}"
}

# get_total extra_params -> prints the total match count for that filter
function get_total {
  local response
  response=$(api_get "$1&ps=1&p=1") || return 1
  echo "${response}" | jq '.total'
}

# fetch_all_pages extra_params label -> pages fully through a filter that
# is already known to match no more than MAX_RESULT_WINDOW issues,
# appending each page's raw JSON response to RAW_FILE
function fetch_all_pages {
  local extra="$1"
  local label="$2"
  local page=1
  local fetched=0
  local total=-1
  local response n

  while :; do
    response=$(api_get "${extra}&ps=${PAGE_SIZE}&p=${page}") || return 1

    if [[ ${total} -eq -1 ]]; then
      total=$(echo "${response}" | jq '.total')
    fi

    echo "${response}" >> ${RAW_FILE}

    n=$(echo "${response}" | jq '.issues | length')
    fetched=$(( fetched + n ))
    echo "  [${label}] page ${page}: ${n} issue(s), ${fetched}/${total}"

    if [[ ${n} -eq 0 || ${fetched} -ge ${total} ]]; then
      break
    fi
    page=$(( page + 1 ))
  done
}

# fetch_partition extra_params label split_dim -> fetches a filter,
# recursively splitting by severity and then type if it exceeds
# MAX_RESULT_WINDOW; split_dim is the next dimension to split by if needed
function fetch_partition {
  local extra="$1"
  local label="$2"
  local split_dim="$3"
  local total sev typ

  total=$(get_total "${extra}") || return 1

  if [[ "${total}" -eq 0 ]]; then
    return 0
  fi

  if [[ "${total}" -le ${MAX_RESULT_WINDOW} ]]; then
    echo "[${label}] ${total} issue(s)"
    fetch_all_pages "${extra}" "${label}"
    return $?
  fi

  case "${split_dim}" in
    severity)
      echo "[${label}] ${total} issue(s) exceeds the ${MAX_RESULT_WINDOW} API limit; splitting by severity"
      for sev in "${SEVERITIES[@]}"; do
        fetch_partition "${extra}&severities=${sev}" "${label}/${sev}" "type" || return 1
      done
      ;;
    type)
      echo "[${label}] ${total} issue(s) exceeds the ${MAX_RESULT_WINDOW} API limit; splitting by type"
      for typ in "${TYPES[@]}"; do
        fetch_partition "${extra}&types=${typ}" "${label}/${typ}" "none" || return 1
      done
      ;;
    none)
      echo "WARNING: [${label}] has ${total} issue(s), still exceeding the ${MAX_RESULT_WINDOW} API limit after splitting by severity and type. Only the first ${MAX_RESULT_WINDOW} will be fetched -- some findings will be MISSING from the output." >&2
      fetch_all_pages "${extra}" "${label}"
      ;;
    *)
      echo "ERROR: $(basename $0) -> fetch_partition() called with unknown split_dim: '${split_dim}'" >&2
      return 1
      ;;
  esac
}

echo "Fetching SonarQube findings for component '${COMPONENT_KEY}' branch '${BRANCH}' from ${SONAR_HOST_URL}"
if [[ ${FILTERED} -eq 1 ]]; then
  echo "Filter: severities=[${SONAR_SEVERITIES:-all}] types=[${SONAR_TYPES:-all}]"
fi

# When the caller already narrowed the query with SONAR_SEVERITIES /
# SONAR_TYPES, don't also auto-split by severity/type -- that filter is
# already baked into BASE_QUERY, and appending another severities= or
# types= param on top of it would conflict. Just warn if it's still too
# big rather than fetching everything to find a further split.
if [[ ${FILTERED} -eq 1 ]]; then
  fetch_partition "" "all" "none"
else
  fetch_partition "" "all" "severity"
fi
STATUS=$?
if [[ ${STATUS} -ne 0 ]]; then
  rm -f ${RAW_FILE}
  exit ${STATUS}
fi

# Merge all of the pages into a single JSON file, de-duplicating issues
# by key since overlapping facets are not used but a retry could add
# a page twice
jq -s '{ issues: ([.[].issues[]] | unique_by(.key)) } | . + { total: (.issues | length) }' ${RAW_FILE} > ${JSON_FILE}
rm -f ${RAW_FILE}

TOTAL_FETCHED=$(jq '.total' ${JSON_FILE})
echo "Wrote ${TOTAL_FETCHED} issue(s) to ${JSON_FILE}"

# Write a flat, sortable text summary: severity, rule, file, line, message
jq -r '.issues[] | [.severity, .rule, .component, (.line // "-" | tostring), .message] | @tsv' ${JSON_FILE} \
  | sort > ${TXT_FILE}

echo "Wrote ${TXT_FILE}"
