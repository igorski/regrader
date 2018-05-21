#!/usr/bin/env bash
clear
echo "Flushing build caches and output folders"
rm -rf build
echo "Creating build folders"
mkdir build
cd build
echo "Building project"
echo "----------------"
cmake "-DCMAKE_OSX_ARCHITECTURES=x86_64;i386" ..
make
