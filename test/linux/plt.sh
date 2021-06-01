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
  lea msg(%rip), %rdi
  xor %rax, %rax
  call printf@PLT
  xor %rax, %rax
  ret

  .data
msg:
  .string "Hello world\n"
EOF

clang -fuse-ld=$1 -o $t/exe $t/a.o

readelf --sections $t/exe | fgrep -q '.got'
readelf --sections $t/exe | fgrep -q '.got.plt'

$t/exe | grep -q 'Hello world'

echo OK
