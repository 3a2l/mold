#!/bin/bash
set -e
cd $(dirname $0)
echo -n "Testing $(basename -s .sh $0) ... "
t=$1/tmp/$(basename -s .sh $0)
mkdir -p $t

cat <<EOF | cc -o $t/a.o -c -x assembler -Wa,--keep-locals -
  .text
  .globl _start
_start:
  nop
foo:
  nop
.Lbar:
  nop
EOF

$2 -o $t/exe $t/a.o
readelf --symbols $t/exe > $t/log
fgrep -q _start $t/log
fgrep -q foo $t/log
fgrep -q .Lbar $t/log

$2 -o $t/exe $t/a.o --discard-locals
readelf --symbols $t/exe > $t/log
fgrep -q _start $t/log
fgrep -q foo $t/log
! fgrep -q .Lbar $t/log || false

$2 -o $t/exe $t/a.o --discard-all
readelf --symbols $t/exe > $t/log
fgrep -q _start $t/log
! fgrep -q foo $t/log || false
! fgrep -q .Lbar $t/log || false

$2 -o $t/exe $t/a.o --strip-all
readelf --symbols $t/exe > $t/log
! fgrep -q _start $t/log || false
! fgrep -q foo $t/log || false
! fgrep -q .Lbar $t/log || false

echo OK
