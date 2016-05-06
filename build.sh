#!/bin/bash


cd $(dirname $0)
_current_dir=$(pwd)

if [[ ! -d build ]]; then
	mkdir build
else
	rm -rf build/ && mkdir build
fi
cd build

cmake ..
make -j4
cd $_current_dir
