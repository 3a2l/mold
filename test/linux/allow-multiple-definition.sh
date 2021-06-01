#!/bin/bash
set -e
cd $(dirname $0)
t=$1/tmp/$(basename -s .sh $0)
mkdir -p $t

echo 'int main() { return 0; }' > $t/a.c
echo 'int main() { return 0; }' > $t/b.c

! clang -fuse-ld=$2 -o $t/exe $t/a.c $t/b.c 2> /dev/null || false
clang -fuse-ld=$2 -o $t/exe $t/a.c $t/b.c -Wl,-allow-multiple-definition

echo OK
