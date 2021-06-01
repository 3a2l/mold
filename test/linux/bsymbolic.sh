#!/bin/bash
set -e
cd $(dirname $0)
echo -n "Testing $(basename -s .sh $0) ... "
t=$1/tmp/$(basename -s .sh $0)
mkdir -p $t

cat <<EOF | cc -shared -fPIC -o $t/a.so -xc - -Wl,-Bsymbolic
int foo = 4;

int get_foo() {
  return foo;
}

void *bar() {
  return bar;
}
EOF

cat <<EOF | cc -c -o $t/b.o -xc - -fno-PIE
#include <stdio.h>

extern int foo;
int get_foo();
void *bar();

int main() {
  foo = 3;
  printf("%d %d %d\n", foo, get_foo(), bar == bar());
}
EOF

clang -fuse-ld=$2 -no-pie -o $t/exe $t/b.o $t/a.so
$t/exe | grep -q '3 4 0'

echo OK
