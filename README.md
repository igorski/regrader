# REGRADER

Regrader is a VST3.0 plug-in which provides a delay effect in which the repeats degrade in
various ways to provide a nice twist on the ears. The delays repeats can be synced to the
hosts tempo and time signature and be fully automated.

## On compatibility

### Build as VST 2.4

VST3.0 is great and all, but support across DAW's is poor (shout out to Bitwig Studio for being awesome). You can however build this plugin as a VST2.4 plugin and enjoy it on a wider range of host platforms. Simply uncomment the following line in _CMakeLists.txt_:

    set(SMTG_CREATE_VST2_VERSION "Use VST2" ON)

And rename the plugin extension from _.vst3_ to _.vst_.

Note: at the moment of writing there is an issue in SDK 3.6.9 where the VST2 plugin wrapper isn't working correctly on macOS. To correct this, add the following line to _"VST3_SDK/public.sdk/source/main/macexport.exp"_ :

    _VSTPluginMain

## Compiling for both 32-bit and 64-bit architectures:

Depending on your host software having 32-bit or 64-bit support, you can best compile for a
wider range of architectures, to do so replace all invocations of _cmake_ in this README with the following:

macOS:
```
cmake "-DCMAKE_OSX_ARCHITECTURES=x86_64;i386" ..
```

Windows:
```
cmake.exe -G"Visual Studio 15 2017 Win64" ..
cmake.exe -G"Visual Studio 15 2017 Win32" ..
```

## Build instructions

### Environment setup

The project uses [CMake](https://cmake.org) to generate the build system
after which you can use _make_ to build the application.

Apart from requiring _CMake_ and a _g++_ compiler, the only other dependency is
the [VST SDK from Steinberg](https://www.steinberg.net/en/company/developers.html).
Update _CMakeLists.txt_ to point to the root of the Steinberg SDK installation
location (update _"VSTSDK_PATH"_).

## Generating the Makefiles

The project has been developed against the VST 3.6.9 Audio Plug-Ins SDK on macOS and Windows 10 and should work completely via CLI without requiring either XCode or Visual Studio (for both command line/build tools suffice). Linux build system is provided, but is as yet untested.

Additionally, the Steinberg VST sources need to be built as well. Following
Steinbergs guidelines, the target is a _/build_-subfolder of the _/VST3_SDK_-folder, execute the following commands from the Steinberg VST SDK root:

    ./copy_vst2_to_vst3_sdk.sh
    cd VST3_SDK
    mkdir build
    cd build
    cmake ..
    cmake --build .

The result being that in _{VSTSDK_PATH}/VST3_SDK/build/lib_ all Steinberg VST libraries are prebuilt. Windows users need to append _--config Release_ to the last cmake (build) call.

### Building the Regrader plugin

Run CMake to generate Regrader's Makefile for your environment, after which you can compile the plugin using make. The build output will be stored in _./build/VST3/regrader.vst_
as well as copied to your systems VST-plugin folder.

#### Compiling on Unix systems:

    mkdir build
    cd build
    cmake ..
    make .

#### Compiling on Windows:

Assuming the Visual Studio Build Tools have been installed:

    mkdir build
    cd build
    cmake.exe -G"Visual Studio 15 2017 Win64" ..
    cmake.exe --build .

## Running the plugin

You can copy the build output into your system VST(3) folder and run it directly in a
VST host / DAW of your choice.

When debugging, you can also choose to run the plugin against Steinbergs validator
and editor host utilities:

    {VSTSDK_PATH}/build/bin/validator  build/VST3/regrader.vst3
    {VSTSDK_PATH}/build/bin/editorhost build/VST3/regrader.vst3
