#!/bin/bash
set -e
cd $(dirname $0)
echo -n "Testing $(basename -s .sh $0) ..."
t=$1/tmp/$(basename -s .sh $0)
mkdir -p $t

cat <<EOF | g++ -c -o $t/a.o -g -gz=zlib-gnu -xc++ -
int main() {
  return 0;
}
EOF

cat <<EOF | g++ -c -o $t/b.o -g -gz=zlib -xc++ -
int foo() {
  return 0;
}
EOF

clang -fuse-ld=$2 -o $t/exe $t/a.o $t/b.o

echo ' OK'
