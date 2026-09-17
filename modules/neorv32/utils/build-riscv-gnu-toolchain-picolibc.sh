#!/bin/bash

echo "Cloning toolchain ..."
git clone https://github.com/riscv/riscv-gnu-toolchain riscv-gnu-toolchain-picolibc
cd riscv-gnu-toolchain-picolibc
git submodule update --init --recursive

echo "Building toolchain ..."
./configure --prefix=/opt/riscv-picolibc --with-arch=rv32i --with-abi=ilp32 --with-picolibc
make -j $(nproc)
echo "Done!"

echo "Info: Don't forget to export PATH=/opt/riscv-picolibc/bin:\$PATH"
