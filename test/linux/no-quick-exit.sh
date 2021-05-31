#!/bin/bash
set -e
cd $(dirname $0)
echo -n "Testing $(basename -s .sh $0) ... "
t=$1/tmp/$(basename -s .sh $0)
mkdir -p $t

cat <<EOF | cc -o $t/a.o -c -xc -
#include <stdio.h>

int main() {
  printf("Hello world\n");
  return 0;
}
EOF

clang -fuse-ld=$2 -o $t/exe $t/a.o -Wl,-no-quick-exit
$t/exe | grep -q 'Hello world'

echo OK
