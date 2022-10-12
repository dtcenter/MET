#! /bin/bash

# If pull request, use GitHub head ref and add -PR to end
# Otherwise use GitHub ref

if [ "${GITHUB_EVENT_NAME}" == "pull_request" ] ; then
  branch_name=${GITHUB_HEAD_REF}-PR
else
  branch_name=${GITHUB_REF}
fi

branch_name=${branch_name#"refs/heads/"}

echo "branch_name=${branch_name}" >> $GITHUB_OUTPUT
echo branch_name: $branch_name
