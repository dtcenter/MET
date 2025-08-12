#! /bin/bash

source ${GITHUB_WORKSPACE}/.github/jobs/bash_functions.sh

DOCKERHUB_TAG=$(get_dockerhub_tag)

# Scan the image
if ! cve_scan_image ${DOCKERHUB_TAG}; then
  echo "ERROR: Critical CVEs found!"
  exit 1
fi
