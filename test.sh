#!/bin/bash

# debug statements
if [ "$1" = "--debug" ]; then
  set -x
fi

all_passed=true

function should_fail() {
  result=$?

  echo -n "==> $1 ($(caller))"

  if [ $result -eq 0 ]; then
    echo "FAILURE"
    all_passed=false
  else
    echo
  fi
}

function should_succeed() {
  result=$?

  echo -n "==> $1 ($(caller))"

  if [ $result -ne 0 ]; then
    echo "FAILURE"
    all_passed=false
  else
    echo
  fi
}

tmp_file=$(mktemp)
tmp_file2=$(mktemp)

function delete_tmp() {
  rm $tmp_file $tmp_file2
}

echo "some text" > $tmp_file
output=$( ./simpsh --rdonly $tmp_file --verbose --wronly $tmp_file \
  --command 0 1 1 sort --wronly $tmp_file )
test "$output" = "$(printf -- "--wronly $tmp_file\n--command 0 1 1 sort\n\
--wronly $tmp_file")"
should_succeed "verbose has the correct output"


output=$(./simpsh --rdonly $tmp_file --wronly $tmp_file2 --command 0 -1 1 2>&1)
should_fail "negative file descriptors should fail"
[[ "$output" =~ "file descriptor" ]]
should_succeed "negative file descriptors should report and error"


output=$(./simpsh --rdonly $tmp_file --wronly $tmp_file2 --command 0 aa 1 2>&1)
should_fail "nonnumber file descriptors should fail"
[[ "$output" =~ "file descriptor" ]]
should_succeed "nonnumber file descriptors should report and error"



delete_tmp

if $all_passed; then
  echo "Success"
  exit 0
else
  echo "Some tests failed"
  exit 1
fi
