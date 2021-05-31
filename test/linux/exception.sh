#!/bin/bash
set -e
cd $(dirname $0)
echo -n "Testing $(basename -s .sh $0) ... "
t=$1/tmp/$(basename -s .sh $0)
mkdir -p $t

cat <<EOF > $t/a.cc
int main() {
  try {
    throw 0;
  } catch (int x) {
    return x;
  }
  return 1;
}
EOF

clang++ -fuse-ld=$2 -o $t/exe $t/a.cc -static
$t/exe

clang++ -fuse-ld=$2 -o $t/exe $t/a.cc
$t/exe

clang++ -fuse-ld=$2 -o $t/exe $t/a.cc -Wl,--gc-sections
$t/exe

clang++ -fuse-ld=$2 -o $t/exe $t/a.cc -static -Wl,--gc-sections
$t/exe

clang++ -fuse-ld=$2 -o $t/exe $t/a.cc -static -mcmodel=large
$t/exe

echo OK
