#!/bin/bash
set -e
cd $(dirname $0)
echo -n "Testing $(basename -s .sh $0) ... "
t=$1/tmp/$(basename -s .sh $0)
mkdir -p $t

cat <<EOF | cc -o $t/a.o -c -x assembler -
  .weak   weak_fn
  .global _start
_start:
  nop
EOF


$2 -o $t/exe $t/a.o
readelf -a $t/exe > /dev/null

echo OK
