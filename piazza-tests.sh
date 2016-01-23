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


./simpsh --rdonly cantpossiblyexist 2>&1 |
  grep "No such file or directory" > /dev/null
should_succeed "reports missing file";


./simpsh --rdonly Makefile | grep "No such file" > /dev/null;
should_fail "does not report file that exists"


./simpsh --verbose --command 1 2 3 echo foo 2>/dev/null >&2
should_fail "using a non existent file descriptor reports an error"


> $tmp
> $tmp2
output=$(./simpsh \
  --verbose \
  --rdonly $tmp \
  --wronly $tmp2 \
  --command 1 2 3 echo foo \
  --command 0 1 1 echo foo 2>&1)

grep foo $tmp2 > /dev/null
should_succeed "commands after failing commands should succeed"


./simpsh --verbose --command 2>&1 | grep "Not enough arguments" > /dev/null
should_succeed "empty command should not work"


./simpsh --verbose --command 1 2 3 2>&1 | grep "Not enough arguments" > /dev/null
should_succeed "command requires at least 4 arguments"


command="--command 0 1 1 echo foo"
./simpsh \
  --rdonly $tmp \
  --wronly $tmp \
  --verbose \
  $command \
  2>&1 | grep -- "$command" > /dev/null
should_succeed "verbose outputs all command options"


./simpsh --rdonly $tmp --wronly $tmp --command 0 1 1 echo foo
grep foo $tmp > /dev/null;
should_succeed "command can write to write only file"


> $tmp
> $tmp2
./simpsh --rdonly $tmp --wronly $tmp2 --command 0 1 1 ls -l
sleep .1 # simulate --wait
grep Makefile $tmp2 > /dev/null
should_succeed "ls command can write to write only file"


> $tmp
big_string=123456789
for i in `seq 5`; do
  big_string+="$big_string"
done
./simpsh --rdonly $tmp2 --wronly $tmp \
  --command 0 1 1 echo "$big_string"
grep $big_string $tmp > /dev/null;
should_succeed "path command can write long strings to write only file"


> $tmp
./simpsh --rdonly $tmp --command 0 0 0 echo foo 2>/dev/null
should_fail "shouldn't be able to write to read only file"
grep foo $tmp > /dev/null
should_fail "shouldn't be able to write to read only file"


echo "foo" > $tmp
# the cat of $tmp should be empty and not add another line to tmp
./simpsh --wronly $tmp --command 0 0 0 cat $tmp 2>/dev/null
should_fail "shouldn't be able to read write only file"
cat $tmp | wc -l | grep 1 > /dev/null
should_succeed "shouldn't be able to read write only file"

echo foo > $tmp
echo bar > $tmp2
./simpsh --rdonly $tmp --rdonly $tmp2 --command 0 1 1 cat 2>/dev/null
should_fail "shouldn't be able to write to read only file"

echo foo > $tmp
echo bar > $tmp2

# cat of $tmp should end up in the $tmp2
./simpsh --rdonly $tmp --wronly $tmp2 \
  --command 0 1 1 cat
cat $tmp2 | grep foo > /dev/null && wc -l $tmp2 | grep 1 > /dev/null
should_succeed "wronly should overwrite file"


#./simpsh --rdonly $tmp --command 0 0 0 echo foo
#echo "$?" | grep "1" > /dev/null
#should_succeed "exit status of failed subcommand should be the exit status of simpsh"


# NOTE: the bin/exit.sh provides an interesting exist value that should trump
# the first command's exit value of 1
#echo "exit 5" > exit.sh
#./simpsh --rdonly $tmp --command 0 0 0 echo foo \
#  --command 0 0 0 bash exit.sh
#echo "$?" | grep "5" > /dev/null
#should_succeed "max exit status of failed subcommand should be the exit status of simpsh"
#rm exit.sh

# TODO: test that verbose outputs each of the options in the right order
# TODO: test with larger number file descriptors

rm $tmp $tmp2

if $all_passed; then
  echo "Success"
  exit 0
else
  echo "Some tests failed"
  exit 1
fi
