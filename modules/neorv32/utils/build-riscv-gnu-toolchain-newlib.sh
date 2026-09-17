#!/bin/bash

echo "Cloning toolchain ..."
git clone https://github.com/riscv/riscv-gnu-toolchain riscv-gnu-toolchain-newlib
cd riscv-gnu-toolchain-newlib
git submodule update --init --recursive

echo "Building toolchain ..."
./configure --prefix=/opt/riscv-newlib --with-arch=rv32i --with-abi=ilp32 --with-newlib
make -j $(nproc)
echo "Done!"

echo "Info: Don't forget to export PATH=/opt/riscv-newlib/bin:\$PATH"
