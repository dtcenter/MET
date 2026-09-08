#! /bin/bash

# set constants for the MET base repository names (regular and unit test)
MET_BASE_REPO_NAME=met-base
MET_BASE_UNIT_REPO_NAME=met-base-unit-test

# set MET base repo to regular repo by default for compilation check
met_base_repo=${MET_BASE_REPO_NAME}

# default MET base tag
met_base_tag=13.0

# override the MET base tag if set
if [[ -n "${met_base_tag_override}" ]]; then
  met_base_tag=${met_base_tag_override}
fi

# if running unit tests, use unit_test MET base image
if [[ -n "${RUN_UNIT_TESTS}" && "${RUN_UNIT_TESTS}" == "true" ]]; then
  met_base_repo=${MET_BASE_UNIT_REPO_NAME}
fi

# add -dev to the MET base repo if the tag does not start with a number
# this assumes that met-base and met-base-unit-test tags start with a number and
# development tags, e.g. branch names, do not
if [[ ! ${met_base_tag} =~ ^[0-9] ]]; then
  met_base_repo=${met_base_repo}-dev
fi

echo "met_base_repo=${met_base_repo}" >> $GITHUB_OUTPUT
echo "met_base_tag=${met_base_tag}" >> $GITHUB_OUTPUT

echo met_base_repo: $met_base_repo
echo met_base_tag: $met_base_tag
