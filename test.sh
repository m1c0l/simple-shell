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
    exit 1
  else
    echo
    return 0
  fi
}

function should_succeed() {
  result=$?

  echo -n "==> $1 ($(caller))"

  if [ $result -ne 0 ]; then
    echo "FAILURE"
    all_passed=false
    exit 1
  else
    echo
    return 0
  fi
}

# Valgrind
function check_valgrind() {
  valgrind="valgrind --leak-check=full --show-leak-kinds=all"
  simpsh="$1"
  output=$($valgrind $simpsh 2>&1)
  [[ "$output" =~ "no leaks are possible" ]]
  return $?
}


tmp=$(mktemp)
tmp2=$(mktemp)
tmp3=$(mktemp)
text="some text"


# --verbose
echo $text > $tmp
output=$( ./simpsh --rdonly $tmp --verbose --wronly $tmp \
  --command 0 1 1 sort --wronly $tmp )
test "$output" = "$(printf -- "--wronly $tmp\n--command 0 1 1 sort\n\
--wronly $tmp")"
should_succeed "verbose has the correct output"


# --command: invalid file descriptors
output=$(./simpsh --rdonly $tmp --wronly $tmp2 --command 0 -1 1 cat 2>&1)
should_fail "negative file descriptors should fail"
[[ "$output" =~ "file descriptor" ]]
should_succeed "negative file descriptors should report and error"

output=$(./simpsh --rdonly $tmp --wronly $tmp2 --command 0 aa 1 cat 2>&1)
should_fail "nonnumber file descriptors should fail"
[[ "$output" =~ "file descriptor" ]]
should_succeed "nonnumber file descriptors should report and error"

output=$(./simpsh --rdonly $tmp --wronly $tmp2 --command 0 1a 1 cat 2>&1)
should_fail "nonnumber file descriptors should fail"
[[ "$output" =~ "file descriptor" ]]
should_succeed "nonnumber file descriptors should report and error"

# proper redirection
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


# valgrind
check_valgrind "./simpsh --rdonly $tmp --wronly $tmp2 --wronly $tmp3 --verbose \
  --command 0 1 2 cat --rdonly in --command 0 1 2 ls -l --wronly out"
should_succeed "Valgrind reports no memory leaks"

check_valgrind "./simpsh --rdonly $tmp --wronly $tmp2 --wronly $tmp3 --verbose \
  --command 0 1 2 cat --rdonly in --command sd0 1 2 ls -l --wronly out"
should_succeed "No memory leaks on bad file descriptor"

check_valgrind "./simpsh --rdonly $tmp --wronly $tmp2 --wronly $tmp3 --verbose \
  --command 0 1 2 cat --rdonly in --command 1 1 2 ls -l --wronly out"
should_succeed "No memory leaks when reading write-only file"

check_valgrind "./simpsh --rdonly $tmp --wronly $tmp2 --command 0 1"
should_succeed "No memory leaks when not enough --command args"

check_valgrind "./simpsh --rdonly $tmp --wronly $tmp2 $tmp3"
should_succeed "No memory leaks when too many arguments to --wronly"


# --rdwr
output=$(./simpsh --rdwr $tmp --command 0 0 0 cat 2>&1)
should_succeed "Can read and write files opened with --rdwr"

echo $text > $tmp
output=$(./simpsh --rdwr $tmp --trunc --wronly $tmp2 --command 0 1 1 cat)
should_succeed "Can read from read-write file"
test "$(cat $tmp2)" = "$(cat $tmp)"
should_succeed "Can read from read-write file"

echo $text > $tmp
output=$(./simpsh --rdonly $tmp --trunc --rdwr $tmp2 --command 0 1 1 cat)
should_succeed "Can write to read-write file"
test "$(cat $tmp2)" = "$(cat $tmp)"
should_succeed "Can write to read-write file"


# --trunc
echo $text > $tmp
output=$(./simpsh --trunc --rdonly $tmp)
should_succeed "Can open a file with --trunc --rdonly"
test "$(cat $tmp)" = ""
should_succeed "Opening a file with --trunc truncates it"

echo $text > $tmp
output=$(./simpsh --trunc --wronly $tmp)
should_succeed "Can open a file with --trunc --wronly"
test "$(cat $tmp)" = ""
should_succeed "Opening a file with --trunc truncates it"


# --creat
rm $tmp
output=$(./simpsh --rdonly $tmp 2>&1)
should_fail "Cannot open a nonexistant file with --rdonly"
[[ "$output" =~ "No such file or directory" ]]
should_succeed "Cannot open a nonexistant file with --rdonly"
output=$(./simpsh --creat --rdonly $tmp 2>&1)
should_succeed "Can open a nonexistant file with --creat --rdonly"
[[ "$output" =~ "No such file or directory" ]]
should_fail "Can open a nonexistant file with --creat --rdonly"
test -e $tmp
should_succeed "File exists after opening with --creat"


# --append
echo $text | tee $tmp > $tmp2
output=$(./simpsh --rdonly $tmp --append --wronly $tmp2 --command 0 1 1 cat)
should_succeed "Using correct syntax with --append"
test "$(cat $tmp $tmp)" = "$(cat $tmp2)" # $tmp2 contains 2 copies of $tmp
should_succeed "writing to --append file appends correctly"


output=$(./simpsh --verbose --append --creat --dsync --rdwr $tmp)
should_succeed "Opening a file can take several oflags"
test "$(printf -- "--append\n--creat\n--dsync\n--rdwr $tmp")" = "$output"
should_succeed "Opening a file can take several oflags"

# oflag syntax
rm $tmp
output=$(./simpsh --rdonly $tmp --creat 2>&1)
should_fail "Trailing oflags are ignored"
output=$(./simpsh --creat --rdonly $tmp2 --wronly $tmp 2>&1)
should_fail "Oflags only apply to next opened file"
echo $text > $tmp
output=$(./simpsh --trunc --creat --verbose --rdonly $tmp)
should_succeed "--creat works when not directly before next opened file"
test "$(cat $tmp)" = ""
should_succeed "--trunc works when not directly before next opened file"


# --close
output=$(./simpsh --rdonly $tmp --wronly $tmp2 --close 0 \
  --command 0 1 1 cat 2>&1)
should_fail "Using a --close'd file is an error"
output=$(./simpsh --rdonly $tmp --wronly $tmp2 --wronly $tmp3 --close 2 \
  --command 0 1 1 cat)
should_succeed "Using --close on a unused file is fine"
output=$(./simpsh --rdonly $tmp --wronly $tmp2 --wronly $tmp3 \
  --command 0 1 1 cat --close 0 --close 1 --close 2 2>&1)
should_succeed "Can close files after using them"


rm $tmp $tmp2 $tmp3

if $all_passed; then
  echo "Success"
  exit 0
else
  echo "Some tests failed"
  exit 1
fi
