#!/bin/bash

EXIT_ON_FAILURE=true

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
    if $EXIT_ON_FAILURE; then
      rm $tmp $tmp2 $tmp3 $tmp4
      exit 1
    fi
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
    if $EXIT_ON_FAILURE; then
      rm $tmp $tmp2 $tmp3 $tmp4
      exit 1
    fi
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
tmp4=$(mktemp)
text=$(printf "a\nB\nd\nc")


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
random="weoifjdklfs"
output=$(./simpsh --rdonly $tmp --trunc --wronly $tmp2 --trunc --wronly $tmp3 \
  --command 0 1 2 ls $tmp $random --wait 2>&1)
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


# Signals

output=$(./simpsh --default 2>&1)
test $? -eq 1
should_succeed "--default should err for no argument"

output=$(./simpsh --default 1 2 2>&1)
test $? -eq 1
should_succeed "--default should err for extra argument"

output=$(./simpsh --catch 2>&1)
test $? -eq 1
should_succeed "--catch should err for no argument"

output=$(./simpsh --catch 1 2 2>&1)
test $? -eq 1
should_succeed "--catch should err for extra argument"

output=$(./simpsh --ignore 2>&1)
test $? -eq 1
should_succeed "--ignore should err for no argument"

output=$(./simpsh --ignore 1 2 2>&1)
test $? -eq 1
should_succeed "--ignore should err for extra argument"


output=$(./simpsh --default 0 2>&1)
test $? -eq 1
should_succeed "--default should fail for non-existent signal 0"

output=$(./simpsh --default 65 2>&1)
test $? -eq 1
should_succeed "--default should fail for non-existent signal > 64"

output=$(./simpsh --catch 0 2>&1)
test $? -eq 1
should_succeed "--catch should fail for non-existent signal 0"

output=$(./simpsh --catch 65 2>&1)
test $? -eq 1
should_succeed "--catch should fail for non-existent signal > 64"

output=$(./simpsh --ignore 0 2>&1)
test $? -eq 1
should_succeed "--ignore should fail for non-existent signal 0"

output=$(./simpsh --ignore 65 2>&1)
test $? -eq 1
should_succeed "--ignore should fail for non-existent signal > 64"



output=$(./simpsh --abort 2>&1)
test $? -eq 139
should_succeed "--abort causes segmentation fault"

output=$(./simpsh --default 11 --abort 2>&1)
test $? -eq 139
should_succeed "--default 11 uses --abort's default behaviour"

output=$(./simpsh --catch 11 --abort 2>&1)
test $? -eq 11
should_succeed "--catch 11 catches --abort, returns correctly"
[[ "$output" =~ "11 caught" ]]
should_succeed "--catch 11 catches --abort, outputs correctly"

output=$(./simpsh --default 11 --catch 11 --abort 2>&1)
test $? -eq 11
should_succeed "--default 11 and then --catch 11 catches --abort, returns correctly"
[[ "$output" =~ "11 caught" ]]
should_succeed "--default 11 and then --catch 11 catches --abort, outputs correctly"

output=$(./simpsh --ignore 11 --abort 2>&1)
test $? -eq 0
should_succeed "--ignore 11 ignores --abort"

output=$(./simpsh --abort --ignore 11 2>&1)
test $? -eq 139
should_succeed "--ignore 11 after --abort still aborts"

output=$(./simpsh --ignore 11 --catch 11 --abort 2>&1)
test $? -eq 11
should_succeed "--ignore 11 and then --catch 11 catches --abort, returns correctly"
[[ "$output" =~ "11 caught" ]]
should_succeed "--ignore 11 and then --catch 11 catches --abort, outputs correctly"

output=$(./simpsh --default 11 --catch 11 --abort 2>&1)
test $? -eq 11
should_succeed "--default 11 and then --catch 11 catches --abort, returns correctly"
[[ "$output" =~ "11 caught" ]]
should_succeed "--default 11 and then --catch 11 catches --abort, outputs correctly"

output=$(./simpsh --catch 9 --ignore 10 --abort 2>&1)
test $? -eq 139
should_succeed "--catch 9 and --ignore 10 don't stop --abort"

# ========================================================================

# Pipes
./simpsh --pipe --pipe --pipe
should_succeed "Can create a pipe"
output=$(./simpsh --pipe --wronly $tmp --command 0 2 2 cat)
should_succeed "Can read from read end of pipe"
output=$(./simpsh --pipe --rdonly $tmp --command 2 1 1 cat)
should_succeed "Can write to write end of pipe"
output=$(./simpsh --pipe --rdonly $tmp --command 1 2 2 cat 2>&1)
should_fail "Cannot write to read end of pipe"
output=$(./simpsh --pipe --rdonly $tmp --command 1 1 1 cat 2>&1)
should_fail "Cannot read from write end of pipe"

echo $text > $tmp
cmd1="cat"
cmd2="tr a-z A-Z"
output=$(./simpsh --rdonly $tmp --trunc --wronly $tmp2 --pipe \
  --command 0 3 3 $cmd1 --command 2 1 1 $cmd2 --wait)
should_succeed "Can use a basic pipe command"
test "$(cat $tmp2)" = "$($cmd1 < $tmp | $cmd2)"
should_succeed "Piped command has correct output"

echo $text > $tmp
cmd1="cat"
cmd2="tr A-Z a-z"
output=$(./simpsh --rdonly $tmp --trunc --wronly $tmp2 --pipe \
  --command 0 3 3 $cmd1 --command 2 1 1 $cmd2 --wait)
should_succeed "Can use a basic pipe command"
test "$(cat $tmp2)" = "$($cmd1 < $tmp | $cmd2)"
should_succeed "Piped command has correct output"

printf "1\n2\n3\n" > $tmp4
output=$(./simpsh \
  --rdonly $tmp \
  --pipe \
  --pipe \
  --creat --trunc --wronly $tmp2 \
  --creat --trunc --wronly $tmp3 \
  --command 3 5 6 tr A-Z a-z \
  --command 0 2 6 sort \
  --command 1 4 6 cat $tmp4 - \
  --wait)
should_succeed "Can handle Eggert's example"
test "$(cat $tmp2)" = "$(sort < $tmp | cat $tmp4 - | tr A-Z a-z)"
should_succeed "Has correct output for Eggert's example"

check_valgrind "./simpsh --rdonly $tmp --pipe --pipe --creat --trunc \
  --wronly $tmp2 --creat --trunc --wronly $tmp3 --command 3 5 6 tr A-Z a-z \
  --command 0 2 6 sort --command 1 4 6 cat $tmp4 - --wait"
should_succeed "No memory leaks on Eggert's example"



rm $tmp $tmp2 $tmp3 $tmp4

if $all_passed; then
  echo "Success"
  exit 0
else
  echo "Some tests failed"
  exit 1
fi
