#! /bin/bash
# Usage: source export-lm32-bin.sh
if [ -d "toolchain" ]; then
  lm32binpath=$(pwd)
  lm32binpath="$lm32binpath/toolchain/bin/"
  echo "Info: Adding $lm32binpath to PATH variable."
  export PATH=$PATH:$lm32binpath
else
  echo "Error: Toolchain directory is missing. Try to execute autogen.sh!"
fi
