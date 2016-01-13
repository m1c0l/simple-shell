#!/bin/bash

# debug statements
if [ "$1" = "--debug" ]; then
  set -x
fi

function should_fail() {
  result=$?

  echo -n "==> $1 ($(caller | grep -oE '[0-9]+'))"

  if [ $result -lt 1 ]; then
    echo "FAILURE"
    delete_tmp
    exit 1
  else
    echo
  fi
}

function should_succeed() {
  result=$?

  echo -n "==> $1 ($(caller | grep -oE '[0-9]+'))"

  if [ $result -gt 0 ]; then
    echo "FAILURE"
    delete_tmp
    exit 1
  else
    echo
  fi
}

tmp_file=$(mktemp)
tmp_file2=$(mktemp)

function delete_tmp() {
  rm $tmp_file $tmp_file2
}

./simpsh --rdonly cantpossiblyexist 2>&1 |
  grep "No such file or directory" > /dev/null
should_succeed "reports missing file";


./simpsh --rdonly Makefile | grep "No such file" > /dev/null;
should_fail "does not report file that exists"


./simpsh --verbose --command 1 2 3 echo foo 2>/dev/null >&2
should_fail "using a non existent file descriptor should report the error"


> $tmp_file
> $tmp_file2
(./simpsh \
  --verbose \
  --rdonly $tmp_file \
  --wronly $tmp_file2 \
  --command 1 2 3 echo foo 2>&1 \
  --command 0 1 1 echo foo ) 2>/dev/null 1>/dev/null

grep foo $tmp_file2 > /dev/null
should_succeed "commands after failing commands should succeed"


./simpsh --verbose --command 2>&1 | grep "Not enough arguments" > /dev/null
should_succeed "empty command should not work"


./simpsh --verbose --command 1 2 3 2>&1 | grep "Not enough arguments" > /dev/null
should_succeed "command requires at least 4 arguments"


command="--command 0 1 1 echo foo"
./simpsh \
  --rdonly $tmp_file \
  --wronly $tmp_file \
  --verbose \
  $command \
  2>&1 | grep -- "$command" > /dev/null
should_succeed "verbose outputs all command options"


./simpsh --rdonly $tmp_file --wronly $tmp_file --command 0 1 1 echo foo
grep foo $tmp_file > /dev/null;
should_succeed "command can write to write only file"


> $tmp_file
> $tmp_file2
./simpsh --rdonly $tmp_file --wronly $tmp_file2 --command 0 1 1 ls -l
sleep .1 # simulate --wait
grep Makefile $tmp_file2 > /dev/null
should_succeed "ls command can write to write only file"


> $tmp_file
big_string=123456789
for i in `seq 5`; do
  big_string+="$big_string"
done
./simpsh --rdonly $tmp_file2 --wronly $tmp_file \
  --command 0 1 1 echo "$big_string"
grep $big_string $tmp_file > /dev/null;
should_succeed "path command can write long strings to write only file"


> $tmp_file
./simpsh --rdonly $tmp_file --command 0 0 0 echo foo 2>/dev/null
should_fail "shouldn't be able to write to read only file"
grep foo $tmp_file > /dev/null
should_fail "shouldn't be able to write to read only file"


echo "foo" > $tmp_file
# the cat of $tmp_file should be empty and not add another line to tmp_file
./simpsh --wronly $tmp_file --command 0 0 0 cat $tmp_file 2>/dev/null
should_fail "shouldn't be able to read write only file"
cat $tmp_file | wc -l | grep 1 > /dev/null
should_succeed "shouldn't be able to read write only file"

echo foo > $tmp_file
echo bar > $tmp_file2
./simpsh --rdonly $tmp_file --rdonly $tmp_file2 --command 0 1 1 cat 2>/dev/null
should_fail "shouldn't be able to write to read only file"

echo foo > $tmp_file
echo bar > $tmp_file2

# cat of $tmp_file should end up in the $tmp_file2
./simpsh --rdonly $tmp_file --wronly $tmp_file2 \
  --command 0 1 1 cat $tmp_file
cat $tmp_file2 | grep foo > /dev/null && cat $tmp_file2 |
  wc -l | grep 1 > /dev/null
should_succeed "wronly should overwrite file"


#./simpsh --rdonly $tmp_file --command 0 0 0 echo foo
#echo "$?" | grep "1" > /dev/null
#should_succeed "exit status of failed subcommand should be the exit status of simpsh"


# NOTE: the bin/exit.sh provides an interesting exist value that should trump
# the first command's exit value of 1
#echo "exit 5" > exit.sh
#./simpsh --rdonly $tmp_file --command 0 0 0 echo foo \
#  --command 0 0 0 bash exit.sh
#echo "$?" | grep "5" > /dev/null
#should_succeed "max exit status of failed subcommand should be the exit status of simpsh"
#rm exit.sh

# TODO: test that verbose outputs each of the options in the right order
# TODO: test with larger number file descriptors

echo "Success"

delete_tmp
