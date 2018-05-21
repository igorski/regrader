@echo off
rm -rf build
mkdir build
cd build
cmake.exe -G"Visual Studio 15 2017 Win64" ..
cmake --build . --config Release
cd ..