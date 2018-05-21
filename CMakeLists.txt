#####################################
# CMake Project for REGRADER Plugin #
#####################################

project(REGRADER)
cmake_minimum_required(VERSION 3.0.0)
set(CMAKE_COLOR_MAKEFILE ON)
set(CMAKE_CXX_STANDARD 11)

# path to Steinberg VST3 SDK
set(VSTSDK_PATH "/Library/VST_SDK/VST3_SDK" CACHE PATH "Path to VST3 SDK")

# compiler flags
add_definitions(-DNDEBUG)

if(MSVC)
    add_definitions(/D _CRT_SECURE_NO_WARNINGS)
endif()

###############
# Unix builds #
###############

if(UNIX)
    if(XCODE)
       set(CMAKE_XCODE_ATTRIBUTE_CLANG_CXX_LANGUAGE_STANDARD "c++14")
       set(CMAKE_XCODE_ATTRIBUTE_CLANG_CXX_LIBRARY "libc++")
    elseif(APPLE)
       set(CMAKE_POSITION_INDEPENDENT_CODE TRUE)
       set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++14 -stdlib=libc++")
       link_libraries(c++)
       # support Yosemite and up
       set(CMAKE_OSX_SYSROOT macosx10.10)
       set(CMAKE_OSX_DEPLOYMENT_TARGET "10.10")
    else()
        add_definitions( -D__cdecl= )
        set(CMAKE_POSITION_INDEPENDENT_CODE TRUE)
        set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++14 -Wno-multichar")
        link_libraries(stdc++fs pthread dl)
    endif()
endif()


#-------------------------------------------------------------------------------
# Includes
#-------------------------------------------------------------------------------

list(APPEND CMAKE_MODULE_PATH "${VSTSDK_PATH}/cmake/modules")

include(Global)
include(AddVST3Library)
include(Bundle)
include(ExportedSymbols)
include(PrefixHeader)
include(PlatformIOS)
include(PlatformToolset)
include(CoreAudioSupport)
include(AAXSupport)
include(VstGuiSupport)
include(UniversalBinary)
include(AddVST3Options)

###############
# VST sources #
###############

# uncomment to build as VST2.4 instead of VST3.0 (provides wider DAW compatibility)
#set(SMTG_CREATE_VST2_VERSION "Use VST2" ON)

set(VSTSDK_INCLUDE_DIR ${VSTSDK_PATH})
set(VSTSDK_PLUGIN_SOURCE
  ${VSTSDK_PATH}/public.sdk/source/vst/vstaudioeffect.cpp
  ${VSTSDK_PATH}/public.sdk/source/vst/vstaudioprocessoralgo.h
  ${VSTSDK_PATH}/public.sdk/source/vst/vsteditcontroller.h
  ${VSTSDK_PATH}/pluginterfaces/base/ibstream.h
  ${VSTSDK_PATH}/pluginterfaces/base/ustring.h
  ${VSTSDK_PATH}/pluginterfaces/vst/ivstevents.h
  ${VSTSDK_PATH}/pluginterfaces/vst/ivstparameterchanges.h
  ${VSTSDK_PATH}/pluginterfaces/vst/vstpresetkeys.h
)

#if (SMTG_CREATE_VST2_VERSION)
    set(regrader_vst2_sources
        ${VSTSDK_PATH}/public.sdk/source/common/memorystream.cpp
        ${VSTSDK_PATH}/public.sdk/source/vst/hosting/eventlist.cpp
        ${VSTSDK_PATH}/public.sdk/source/vst/hosting/hostclasses.cpp
        ${VSTSDK_PATH}/public.sdk/source/vst/hosting/parameterchanges.cpp
        ${VSTSDK_PATH}/public.sdk/source/vst/hosting/processdata.cpp
        ${VSTSDK_PATH}/public.sdk/source/vst/vst2wrapper/vst2wrapper.cpp
        ${VSTSDK_PATH}/public.sdk/source/vst/vst2wrapper/vst2wrapper.h
        ${VSTSDK_PATH}/public.sdk/source/vst2.x/audioeffect.cpp
        ${VSTSDK_PATH}/public.sdk/source/vst2.x/audioeffectx.cpp
        src/vstentry_vst2.cpp
    )
#endif()

###################
# Project sources #
###################

set(regrader_sources
    src/global.h
    src/audiobuffer.h
    src/audiobuffer.cpp
    src/paramids.h
    src/lfo.h
    src/lfo.cpp
    src/regraderprocess.h
    src/regraderprocess.cpp
    src/vst.h
    src/vst.cpp
    src/vstentry.cpp
    src/version.h
    src/ui/controller.h
    src/ui/controller.cpp
    src/ui/uimessagecontroller.h
    ${VSTSDK_PLUGIN_SOURCE}
)

# VST2 is only defined for macOS and Windows
if(MAC OR WIN)
    set(regrader_sources ${regrader_sources} ${regrader_vst2_sources})
endif()

set(target regrader)
smtg_add_vst3plugin(${target} ${VSTSDK_PATH} ${regrader_sources})

###########################
# STEINBERG VST LIBRARIES #
###########################

include_directories(${VSTSDK_INCLUDE_DIR})
target_link_libraries(${target} ${VSTSDK_LIBRARIES})
target_include_directories(${target} PUBLIC ${VSTSDK_PATH}/vstgui4)
if (UNIX)
	target_link_libraries(${target} PRIVATE ${VSTSDK_PATH}/build/lib/libbase.a)
	target_link_libraries(${target} PRIVATE ${VSTSDK_PATH}/build/lib/libsdk.a)
	target_link_libraries(${target} PRIVATE ${VSTSDK_PATH}/build/lib/libvstgui_support.a)
	target_link_libraries(${target} PRIVATE ${VSTSDK_PATH}/build/lib/libvstgui_uidescription.a)
	target_link_libraries(${target} PRIVATE ${VSTSDK_PATH}/build/lib/libvstgui.a)
endif()
if (WIN)
    target_link_libraries(${target} PRIVATE ${VSTSDK_PATH}/build/lib/Release/base.lib)
    target_link_libraries(${target} PRIVATE ${VSTSDK_PATH}/build/lib/Release/sdk.lib)
    target_link_libraries(${target} PRIVATE ${VSTSDK_PATH}/build/lib/Release/vstgui_support.lib)
    target_link_libraries(${target} PRIVATE ${VSTSDK_PATH}/build/lib/Release/vstgui_uidescription.lib)
    target_link_libraries(${target} PRIVATE ${VSTSDK_PATH}/build/lib/Release/vstgui.lib)
endif()

###############################
# PLATFORM SPECIFIC LIBRARIES #
###############################

IF (APPLE)
    if(XCODE)
        target_link_libraries(${target} PRIVATE "-framework Cocoa" "-framework OpenGL" "-framework Accelerate" "-framework QuartzCore" "-framework Carbon")
    else()
        find_library(COREFOUNDATION_FRAMEWORK CoreFoundation)
        find_library(COCOA_FRAMEWORK Cocoa)
        find_library(OPENGL_FRAMEWORK OpenGL)
        find_library(ACCELERATE_FRAMEWORK Accelerate)
        find_library(QUARTZCORE_FRAMEWORK QuartzCore)
        find_library(CARBON_FRAMEWORK Carbon)
        find_library(EXPAT Expat)
        target_link_libraries(${target} PRIVATE ${COREFOUNDATION_FRAMEWORK} ${COCOA_FRAMEWORK} ${OPENGL_FRAMEWORK} ${ACCELERATE_FRAMEWORK} ${QUARTZCORE_FRAMEWORK} ${CARBON_FRAMEWORK} ${EXPAT})
    endif()
ENDIF (APPLE)

#############
# Resources #
#############

smtg_add_vst3_resource(${target} "resources/graphics/regrader.uidesc")
smtg_add_vst3_resource(${target} "resources/graphics/background.png")
smtg_add_vst3_resource(${target} "resources/graphics/slider_background.png")
smtg_add_vst3_resource(${target} "resources/graphics/slider_handle.png")
smtg_add_vst3_resource(${target} "resources/graphics/slider_handle_2.0x.png")

#########################
# Set bundle properties #
#########################

set_target_properties(regrader PROPERTIES
  BUNDLE true
  BUNDLE_EXTENSION "vst3"
  XCODE_ATTRIBUTE_WRAPPER_EXTENSION "vst3"
  MACOSX_BUNDLE_INFO_PLIST "${CMAKE_CURRENT_SOURCE_DIR}/resources/osx/Info.plist"
  MACOSX_BUNDLE_BUNDLE_NAME "REGRADER"
  MACOSX_BUNDLE_GUI_IDENTIFIER "nl.igorski.REGRADER"
  MACOSX_BUNDLE_ICON_FILE ""
  MACOSX_BUNDLE_SHORT_VERSION_STRING "1.0.0"
  MACOSX_BUNDLE_COPYRIGHT "igorski.nl © 2018"
)

if(MAC)
    smtg_set_bundle(${target} INFOPLIST "${CMAKE_CURRENT_SOURCE_DIR}/resources/osx/Info.plist" PREPROCESS)
elseif(WIN)
    target_sources(${target} PRIVATE resources/graphics/regrader.rc)
endif()

if (SMTG_CREATE_VST2_VERSION)
    message(STATUS "SMTG_CREATE_VST2_VERSION is set. A VST 2 version of the plug-in will be created (just rename the generated file from .vst3 to .vst).")
    if(XCODE)
        # fix missing VSTPluginMain symbol when also building VST 2 version
        set_target_properties(${target} PROPERTIES XCODE_ATTRIBUTE_EXPORTED_SYMBOLS_FILE "")
    endif()
    if (WIN)
        add_definitions(-D_CRT_SECURE_NO_WARNINGS)
    endif()
endif()

####################
# Set Install Path #
####################

if(APPLE)
  install(TARGETS regrader
    DESTINATION "$ENV{HOME}/Library/Audio/Plug-Ins/VST"
  )
elseif(WIN32)
  install(TARGETS regrader
    DESTINATION "C:/Program Files (x86)/Common Files/VST3/"
  )
elseif(WIN)
  install(TARGETS regrader
    DESTINATION "C:/Program Files/Common Files/VST3/"
  )
elseif(UNIX AND NOT APPLE) #Linux
  install(TARGETS regrader
    DESTINATION "/usr/lib/lxvst"
  )
endif()
