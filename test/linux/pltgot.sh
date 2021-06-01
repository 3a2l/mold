#!/bin/bash
set -e
cd $(dirname $0)
echo -n "Testing $(basename -s .sh $0) ... "
t=$(pwd)/tmp/$(basename -s .sh $0)
mkdir -p $t

cat <<EOF | clang -fPIC -shared -o $t/a.so -x assembler -
.globl ext1, ext2
ext1:
  nop
ext2:
  nop
EOF

cat <<EOF | clang -c -o $t/b.o -x assembler -
.globl _start
_start:
  call ext1@PLT
  call ext2@PLT
  mov ext2@GOTPCREL(%rip), %rax
  ret
EOF

$1 -o $t/exe $t/b.o $t/a.so

objdump -d -j .plt.got $t/exe > $t/log

grep -Pq '201020:\s+ff 25 da 0f 00 00\s+jmpq   \*0xfda\(%rip\)\s+# 202000 <ext2>' $t/log

echo OK
