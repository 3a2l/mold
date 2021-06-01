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

$1 -o $t/exe \
  -dynamic-linker=/lib64/ld-linux-x86-64.so.2 \
  /usr/lib/x86_64-linux-gnu/crt1.o \
  /usr/lib/x86_64-linux-gnu/crti.o \
  /usr/lib/gcc/x86_64-linux-gnu/9/crtbegin.o \
  $t/a.o \
  /usr/lib/gcc/x86_64-linux-gnu/9/libgcc.a \
  /usr/lib/x86_64-linux-gnu/libgcc_s.so.1 \
  /lib/x86_64-linux-gnu/libc.so.6 \
  /usr/lib/x86_64-linux-gnu/libc_nonshared.a \
  /lib/x86_64-linux-gnu/ld-linux-x86-64.so.2 \
  /usr/lib/gcc/x86_64-linux-gnu/9/crtend.o \
  /usr/lib/x86_64-linux-gnu/crtn.o

$t/exe | grep -q 'Hello world'

echo OK
