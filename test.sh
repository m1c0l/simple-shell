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

tmp=$(mktemp)
tmp2=$(mktemp)
tmp3=$(mktemp)

echo "some text" > $tmp
output=$( ./simpsh --rdonly $tmp --verbose --wronly $tmp \
  --command 0 1 1 sort --wronly $tmp )
test "$output" = "$(printf -- "--wronly $tmp\n--command 0 1 1 sort\n\
--wronly $tmp")"
should_succeed "verbose has the correct output"


output=$(./simpsh --rdonly $tmp --wronly $tmp2 --command 0 -1 1 2>&1)
should_fail "negative file descriptors should fail"
[[ "$output" =~ "file descriptor" ]]
should_succeed "negative file descriptors should report and error"


output=$(./simpsh --rdonly $tmp --wronly $tmp2 --command 0 aa 1 2>&1)
should_fail "nonnumber file descriptors should fail"
[[ "$output" =~ "file descriptor" ]]
should_succeed "nonnumber file descriptors should report and error"

output=$(./simpsh --rdonly $tmp --wronly $tmp2 --command 0 1a 1 2>&1)
should_fail "nonnumber file descriptors should fail"
[[ "$output" =~ "file descriptor" ]]
should_succeed "nonnumber file descriptors should report and error"

random="reiujfsdkf"
output=$(./simpsh --rdonly $tmp --wronly $tmp2 --wronly $tmp3 \
  --command 0 1 2 ls $tmp $random 2>&1)
should_succeed "valid call should return 0"
test "$(cat $tmp2)" = "$tmp"
should_succeed "writes to stdout correctly"
[[ "$(cat $tmp3)" =~ "No such file or directory" ]]
should_succeed "writes to stderr correctly"
test "$(cat $tmp3)" = "$tmp"
should_fail "stderr should not be in stdout file"
[[ "$(cat $tmp2)" =~ "No such file or directory" ]]
should_fail "stdout should not be in stderr file"



rm $tmp $tmp2 $tmp3

if $all_passed; then
  echo "Success"
  exit 0
else
  echo "Some tests failed"
  exit 1
fi
