#!/bin/bash
set -e
cd $(dirname $0)
echo -n "Testing $(basename -s .sh $0) ... "
t=$(pwd)/tmp/$(basename -s .sh $0)
mkdir -p $t

cat <<EOF | clang -c -o $t/a.o -x assembler -
.globl _start
_start:
  ret
EOF

$1 -o $t/exe $t/a.o

readelf --sections $t/exe > $t/log
! fgrep .interp $t/log || false

readelf --dynamic $t/exe > $t/log

$1 -o $t/exe $t/a.o --dynamic-linker=/foo/bar

readelf --sections $t/exe > $t/log
fgrep -q .interp $t/log

echo OK
