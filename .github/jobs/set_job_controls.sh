#! /bin/bash

run_compile=true

# check commit messages for skip or force keywords
if grep -q "ci-skip-compile" <<< "$commit_msg"; then
  run_compile=false
fi

echo ::set-output name=run_compile::$run_compile
