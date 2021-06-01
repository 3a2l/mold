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

clang -Wl,-build-id=sha1 -fuse-ld=$2 $t/a.o -o - > $t/exe
chmod 755 $t/exe
$t/exe | grep -q 'Hello world'

echo OK
