# REGRADER

Regrader is a VST/AU plug-in which provides a delay effect in which the repeats degrade in various ways to provide a nice twist on the ears.
The delays repeats can be synced to the host tempo and time signature and be fully automated.

If you require some inspiration, it is used pretty much all over the [Imago](https://songwhip.com/drosophelia/imago) album by Drosophelia or you can [read what others say](https://bedroomproducersblog.com/2018/06/08/free-regrader-lo-fi-delay/).

## On compatibility

### Build as VST 2.4

VST3.0 is great and all, but support across DAW's is poor (looking at a certain popular German product). You can however build this plugin as a VST2.4 plugin and enjoy it on a wider range of host platforms.

However: as of SDK 3.6.11, Steinberg no longer packages the _./pluginterfaces/vst2.x_-folder inside the VST3_SDK folder.
If you _really_ wish to build a VST2 plugin, copying the folder from an older SDK version _could_ work (verified 3.6.9. _vst2.x_ folders to work with SDK 3.7.0), though be aware
that you _need a license to use VST2_. You can view [Steinbergs rationale on this decision here](https://www.steinberg.net/en/newsandevents/news/newsdetail/article/vst-2-coming-to-an-end-4727.html).

Once your SDK is "setup" for VST2, simply uncomment the following line in _CMakeLists.txt_:

```
set(SMTG_CREATE_VST2_VERSION "Use VST2" ON)
```

And rename the generated plugin extension from _.vst3_ to _.vst_ (or _.dll_ on Windows).

## Build instructions

### Environment setup

The project uses [CMake](https://cmake.org) to generate the build system after which you can use _make_ to build the application.

Apart from requiring _CMake_ and a _g++_ compiler, the only other dependency is the [VST SDK from Steinberg](https://www.steinberg.net/en/company/developers.html).

## Generating the Makefiles

The project has been developed against the [VST3 SDK version 3.7.0](https://www.steinberg.net/vst3sdk) on macOS and Windows 10 and should work completely via CLI without requiring either Xcode or Visual Studio (for both command line/build tools suffice). Linux build system is provided, but is as yet untested.

Additionally, the Steinberg VST sources need to be built as well. Following Steinbergs guidelines, the target is a _/build_-subfolder of the _/VST3_SDK_-folder, execute the following commands from the Steinberg VST SDK root:

```
./copy_vst2_to_vst3_sdk.sh
cd VST3_SDK
mkdir build
cd build
cmake -DCMAKE_BUILD_TYPE=Release ..
cmake --build .
```

The result being that in _{VST3_SDK_ROOT}/VST3_SDK/build/lib_ all Steinberg VST libraries are prebuilt.
NOTE: Windows users need to append _--config Release_ to the last cmake (build) call as the build type must be defined during compilation.

### Building the Regrader plugin

Run CMake to generate Regrader's Makefile for your environment, after which you can compile the plugin using make. The build output will be stored in _./build/VST3/regrader.vst_ as well as copied to your systems VST-plugin folder.

You must provide the path to your custom SDK download location by providing _VST3_SDK_ROOT_ to CMake like so:

```
cmake -DVST3_SDK_ROOT=/path/to/VST_SDK/VST3_SDK/ ..
```

#### Compiling on Unix systems:

```
mkdir build
cd build
cmake -DVST3_SDK_ROOT=/path/to/VST_SDK/VST3_SDK/ ..
make .
```

#### Compiling on Windows:

Assuming the Visual Studio Build Tools have been installed:

```
mkdir build
cd build
cmake.exe -G"Visual Studio 16 2019" -DVST3_SDK_ROOT=/path/to/VST_SDK/VST3_SDK/ ..
cmake.exe --build .
```

### Running the plugin

You can copy the build output into your system VST(3) folder and run it directly in a VST host / DAW of your choice.

When debugging, you can also choose to run the plugin against Steinbergs validator and editor host utilities:

    {VST3_SDK_ROOT}/build/bin/validator  build/VST3/regrader.vst3
    {VST3_SDK_ROOT}/build/bin/editorhost build/VST3/regrader.vst3

### Build as Audio Unit (macOS only)

Is aided by the excellent [Jamba framework](https://github.com/pongasoft/jamba) by Pongasoft, which provides a toolchain around Steinbergs SDK. Execute the following instructions to build Regrader as an Audio Unit:

* Build the AUWrapper Project in the Steinberg SDK folder
* Create a Release build of the Xcode project generated in step 1, this creates _VST3_SDK/public.sdk/source/vst/auwrapper/build/lib/Release/libauwrapper.a_
* Run _sh build_au.sh_ from the repository root, providing the path to _VST3_SDK_ROOT_ as before:

```
VST3_SDK_ROOT=/path/to/VST_SDK/VST3_SDK sh build_au.sh
```

The subsequent Audio Unit component will be located in _./build/VST3/regrader.component_ as well as linked
in _~/Library/Audio/Plug-Ins/Components/_

You can validate the Audio Unit using Apple's _auval_ utility, by running _auval -v aufx dely IGOR_ on the command line. Note that there is the curious behaviour that you might need to reboot before the plugin shows up, though you can force a flush of the Audio Unit cache at runtime by running _killall -9 AudioComponentRegistrar_.
