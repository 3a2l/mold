#!/bin/bash
set -e
cd $(dirname $0)
echo -n "Testing $(basename -s .sh $0) ... "
t=$1/tmp/$(basename -s .sh $0)
mkdir -p $t

cat <<EOF | cc -o $t/a.o -c -x assembler -
  .text
  .globl main
main:
  mov $.L.str+3, %rdi
  xor %rax, %rax
  call printf
  mov $.rodata.str1.1+16, %rdi
  xor %rax, %rax
  call printf
  xor %rax, %rax
  ret

  .section .rodata.str1.1, "aMS", @progbits, 1
  .string "bar"
.L.str:
  .string "xyzHello"
  .string "foo world\n"
EOF

clang -fuse-ld=$2 -static -o $t/exe $t/a.o
$t/exe | grep -q 'Hello world'

echo OK
