#! /bin/bash

# utility function to print a log file, or the end of it, inside a collapsible
# group. Does nothing when no file is named, the file does not exist, or it is
# empty.
function print_log_file {
  local logfile=$1
  local tail_lines=$2

  if [[ -z "${logfile}" || ! -s "${logfile}" ]]; then
    return 0
  fi

  if [[ -n "${tail_lines}" ]]; then
    echo "::group::Last ${tail_lines} lines of ${logfile}; see the logs artifact for the rest"
    tail -n ${tail_lines} "${logfile}"
  else
    echo "::group::Output of the failed command, from ${logfile}"
    cat "${logfile}"
  fi
  echo "::endgroup::"
}

# utility function to run a command and log the time it took to run
#
# Usage: time_command [-l logfile] [-t lines] command [arguments...]
#
#   -l logfile  append the command output to this file instead of writing it to
#               the console, and print the file if the command fails
#   -t lines    when printing the log after a failure, print only this many
#               lines from the end of it rather than the whole file
#
# The log file is passed as an argument rather than through a global variable so
# that the caller still has the path after the command returns.
#
# ::group:: and ::endgroup:: create collapsible log groups on GitHub Actions
function time_command {
  local logfile=
  local tail_lines=
  while [[ $# -gt 0 ]]; do
    local option=$1
    # -l and -t each consume an argument. Without this check a missing one
    # leaves shift 2 with nothing to shift, so it fails without shifting and
    # this loop spins forever.
    if [[ "${option}" == "-l" || "${option}" == "-t" ]] && [[ $# -lt 2 ]]; then
      echo "ERROR: time_command option ${option} requires an argument" >&2
      return 1
    fi
    case "${option}" in
      -l) logfile=$2; shift 2 ;;
      -t) tail_lines=$2; shift 2 ;;
      *)  break ;;
    esac
  done

  local start_seconds=$SECONDS
  echo "::group::RUNNING: $*"

  local error
  # write output to the log file if one was named
  if [[ -z "${logfile}" ]]; then
      "$@"
      error=$?
  else
      echo "Logging to ${logfile}"
      # use the POSIX redirection form rather than &>>, which bash 3.2 cannot
      # parse, so that these scripts can be run on a Mac
      "$@" >> "${logfile}" 2>&1
      error=$?
  fi

  local duration=$(( SECONDS - start_seconds ))
  echo "TIMING: Command took $(printf '%02d' $(($duration / 60))):$(printf '%02d' $(($duration % 60))) (MM:SS): '$*'"
  echo "::endgroup::"
  if [[ ${error} -ne 0 ]]; then
    echo "ERROR: '$*' exited with status = ${error}"
    print_log_file "${logfile}" "${tail_lines}"
  fi
  return $error
}

# utility function to construct the DockerHub tag name to be used,
# replacing slashes with underscores in the branch name, and
# omitting the leading 'v' from the DockerHub version

function get_dockerhub_tag {
  echo ${DOCKERHUB_REPO}:$(echo ${SOURCE_BRANCH} | sed 's%/%_%g' | sed 's%^v%%g' )
}

# utility function to scan a Docker image for vulnerabilities
function cve_scan_image {
  local image=$1

  echo "Scanning image ${image}"
  local scan_logfile="${GITHUB_WORKSPACE}/CVE_Scan_$(echo "${image}" | sed 's%[/,:]%_%g').log"
  time_command -l "${scan_logfile}" grype "${image}"
  N_CRITICAL=$(grep "Critical" "${scan_logfile}" | wc -l)
  if [[ ${N_CRITICAL} -gt 0 ]]; then
    echo "WARNING: Found ${N_CRITICAL} Critical CVEs for image ${image} in ${scan_logfile}"
    echo
    egrep "SEVERITY|Critical" ${scan_logfile}
    echo
  fi
}
