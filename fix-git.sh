#! /bin/bash

git submodule init
git submodule sync --recursive
git submodule update --init --recursive
