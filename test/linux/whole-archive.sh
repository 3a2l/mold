#!/bin/bash
set -e
cd $(dirname $0)
echo -n "Testing $(basename -s .sh $0) ... "
t=$1/tmp/$(basename -s .sh $0)
mkdir -p $t

cat <<EOF | cc -o $t/a.o -c -x assembler -
  .text
  .globl _start
_start:
  ret
EOF

echo 'int fn1() { return 42; }' | cc -o $t/b.o -c -xc -
echo 'int fn2() { return 42; }' | cc -o $t/c.o -c -xc -

rm -f $t/d.a
ar cr $t/d.a $t/b.o $t/c.o

$2 -o $t/exe $t/a.o $t/d.a
readelf --symbols $t/exe > $t/readelf
! grep -q fn1 $t/readelf || false
! grep -q fn2 $t/readelf || false

$2 -o $t/exe $t/a.o --whole-archive $t/d.a
readelf --symbols $t/exe > $t/readelf
grep -q fn1 $t/readelf
grep -q fn2 $t/readelf

$2 -o $t/exe $t/a.o --whole-archive --no-whole-archive $t/d.a
readelf --symbols $t/exe > $t/readelf
! grep -q fn1 $t/readelf || false
! grep -q fn2 $t/readelf || false

echo OK
