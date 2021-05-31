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
  lea msg(%rip), %rdi
  xor %rax, %rax
  call printf
  xor %rax, %rax
  ret

  .data
msg:
  .string "Hello world\n"
EOF

clang -fuse-ld=$2 -o $t/exe $t/a.o -static
clang -fuse-ld=gold -o $t/exe $t/a.o -static
$t/exe | grep -q 'Hello world'

echo OK
