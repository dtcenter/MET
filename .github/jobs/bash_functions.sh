#! /bin/bash

# utility function to run command get log the time it took to run
# if CMD_LOGFILE is set, send output to that file and unset var
function time_command {
  local start_seconds=$SECONDS
  echo "RUNNING: $*"

  local error
  # pipe output to log file if set
  if [ "x$CMD_LOGFILE" == "x" ]; then
      "$@"
      error=$?
  else
      echo "Logging to ${CMD_LOGFILE}"
      "$@" &>> $CMD_LOGFILE
      error=$?
      unset CMD_LOGFILE
  fi

  local duration=$(( SECONDS - start_seconds ))
  echo "TIMING: Command took `printf '%02d' $(($duration / 60))`:`printf '%02d' $(($duration % 60))` (MM:SS): '$*'"
  if [ ${error} -ne 0 ]; then
    echo "ERROR: '$*' exited with status = ${error}"
  fi
  return $error
}

# utility function to construct the DockerHub tag name to be used,
# replacing slashes with underscores in the branch name

function get_dockerhub_tag {
  echo ${DOCKERHUB_REPO}:$(echo ${SOURCE_BRANCH} | sed 's%/%_%g')
}
