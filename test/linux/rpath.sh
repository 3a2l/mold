#!/bin/bash
set -e
cd $(dirname $0)
echo -n "Testing $(basename -s .sh $0) ... "
t=$(pwd)/tmp/$(basename -s .sh $0)
mkdir -p $t

cat <<EOF | cc -o $t/a.o -c -x assembler -
  .text
  .globl main
main:
  nop
EOF

clang -fuse-ld=`pwd`/../mold -o $t/exe $t/a.o \
  -Wl,-rpath,/foo -Wl,-rpath,/bar -Wl,-R/no/such/directory -Wl,-R/

readelf --dynamic $t/exe | \
  fgrep -q 'Library runpath: [/foo:/bar:/no/such/directory:/]'

echo OK
