#!/bin/bash
set -e
cd $(dirname $0)
echo -n "Testing $(basename -s .sh $0) ... "
t=$(pwd)/tmp/$(basename -s .sh $0)
mkdir -p $t

cat <<'EOF' | cc -xc -c -o $t/a.o -
#include <stdio.h>

int main() {
  printf("Hello\n");
  return 0;
}
EOF

gcc -o $t/exe $t/a.o
readelf -p .comment $t/exe > $t/log
! grep -q mold $t/log || false

clang -o $t/exe $t/a.o
readelf -p .comment $t/exe > $t/log
! grep -q mold $t/log || false

LD_PRELOAD=$2 MOLD_REAL_PATH=$1 \
  gcc -o $t/exe $t/a.o
readelf -p .comment $t/exe > $t/log
grep -q mold $t/log

LD_PRELOAD=$2 MOLD_REAL_PATH=$1 \
  clang -o $t/exe $t/a.o
readelf -p .comment $t/exe > $t/log
grep -q mold $t/log

$1 -run env | grep -q '^MOLD_REAL_PATH=.*/mold$'

$1 -run /usr/bin/ld --version | grep -q mold
$1 -run /usr/bin/ld.lld --version | grep -q mold
$1 -run /usr/bin/ld.gold --version | grep -q mold

echo OK
