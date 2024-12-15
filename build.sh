#!/usr/bin/env bash
clear
echo "Flushing build caches and output folders"
rm -rf build
echo "Creating build folders"
mkdir build
cd build
echo "Building project"
echo "----------------"

# env variable for VST3_SDK_ROOT set ? pass it to CMake configuration

if [ -n "${VST3_SDK_ROOT}" ]; then
  DVST3_SDK_ROOT="-DVST3_SDK_ROOT=${VST3_SDK_ROOT}"
fi

# flag to build as VST2 supplied ? pass it to CMake configuration

if [ "$1" == "vst2" ]; then
    SMTG_CREATE_VST2_VERSION="-DSMTG_CREATE_VST2_VERSION=ON"
fi

cmake ${SMTG_CREATE_VST2_VERSION} ${DVST3_SDK_ROOT} ..
cmake --build . --config Release

buildStatus=$?

if [ $buildStatus -eq 0 ]; then
    if [ "$1" == "vst2" ]; then
        mv ./VST3/regrader.vst3 ./VST3/regrader.vst
    fi
    echo "Plugin built successfully"
else
    echo "An error occurred during build of plugin"
fi

exit $buildStatus
