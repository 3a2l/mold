#!/bin/bash
set -e
cd $(dirname $0)
echo -n "Testing $(basename -s .sh $0) ... "
t=$(pwd)/tmp/$(basename -s .sh $0)
mkdir -p $t

cat <<EOF | cc -o $t/a.o -c -x assembler -
  .text
  .globl  real_foobar
real_foobar:
  lea     msg(%rip), %rdi
  xor     %rax, %rax
  call    printf
  xor     %rax, %rax
  ret

  .globl  resolve_foobar
resolve_foobar:
  pushq   %rbp
  movq    %rsp, %rbp
  leaq    real_foobar(%rip), %rax
  popq    %rbp
  ret

  .globl  foobar
  .type   foobar, @gnu_indirect_function
  .set    foobar, resolve_foobar

  .globl  main
main:
  pushq   %rbp
  movq    %rsp, %rbp
  call    foobar@PLT
	xor     %rax, %rax
  popq    %rbp
  ret

  .data
msg:
  .string "Hello world\n"
EOF

clang -fuse-ld=$1 -o $t/exe $t/a.o
$t/exe | grep -q 'Hello world'

echo OK
