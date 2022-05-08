#!/bin/bash

assert(){
  expected="$1"
  input="$2"

  ./main "$input" > tmp.s
  gcc -o tmp tmp.s
  ./tmp
  actual="$?"

  if [ "$actual" = "$expected" ]; then
    echo "$input => $actual"
  else
    echo "$input => $expected expected, but got $actual"
    exit 1
  fi
}

# step 1 
assert 3 3
assert 42 42
# step 2
assert 21 "5+20-4"

echo OK