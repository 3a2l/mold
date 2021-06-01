#!/bin/bash
set -e
cd $(dirname $0)
echo -n "Testing $(basename -s .sh $0) ... "
t=$1/tmp/$(basename -s .sh $0)
mkdir -p $t

echo '.globl _start; _start: jmp loop' | cc -o $t/a.o -c -x assembler -
echo '.globl loop; loop: jmp loop' | cc -o $t/b.o -c -x assembler -
$2 -static -o $t/exe $t/a.o $t/b.o
objdump -d $t/exe > /dev/null
file $t/exe | grep -q ELF

echo OK
