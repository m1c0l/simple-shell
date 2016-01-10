passed=true

test_simpsh() {
  one="$($1)"
  two="$2"

  if [ "$one" != "$two" ]; then
    echo "'$1' has incorrect output
Output:
$one
Expected:
$two
" 2>&1

    passed=false
  fi
}

# Test syntax
# test_simpsh "<simpsh command>" "<expected output>"
# test_simpsh "<simpsh command>" "$(<shell equivalent>)"

#test_simpsh "./simpsh" "$(ls)"

if [ "$passed" = "true" ]; then
  echo "All tests passed"
  exit 0
else
  exit 1
fi
