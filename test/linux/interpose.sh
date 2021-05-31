#!/bin/bash
set -e
cd $(dirname $0)
echo -n "Testing $(basename -s .sh $0) ... "
t=$1/tmp/$(basename -s .sh $0)
mkdir -p $t

cat <<EOF | clang -c -fPIC -o $t/a.o -xc -
#include <stdio.h>

void foo() {
  printf("Hello world\n");
}
EOF

clang -fuse-ld=$2 -shared -o $t/b.so $t/a.o -Wl,-z,interpose
readelf --dynamic $t/b.so | grep -q 'Flags: INTERPOSE'

echo OK
