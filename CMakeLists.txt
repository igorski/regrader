#####################################
# CMake Project for REGRADER Plugin #
#####################################

# uncomment to build as VST2.4 instead of VST3.0 (provides wider DAW compatibility)
# note VST2 is only defined for macOS and Windows
if(SMTG_MAC OR SMTG_WIN)
    #set(SMTG_CREATE_VST2_VERSION "Use VST2" ON)
endif()

#######################
# DO NOT CHANGE BELOW #
#######################

project(REGRADER)
cmake_minimum_required(VERSION 3.12)
set(CMAKE_COLOR_MAKEFILE ON)
set(CMAKE_CXX_STANDARD 14)

# compiler flags
add_definitions(-DNDEBUG)

if(MSVC)
    add_definitions(/D _CRT_SECURE_NO_WARNINGS)
endif()

set(VST3_SDK_ROOT "/Library/VST_SDK/VST3_SDK")

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
        set(LINUX true)
        add_definitions( -D__cdecl= )
        set(CMAKE_POSITION_INDEPENDENT_CODE TRUE)
        set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++14 -Wno-multichar")
        link_libraries(stdc++fs pthread dl)
    endif()
endif()


#-------------------------------------------------------------------------------
# Includes
#-------------------------------------------------------------------------------

list(APPEND CMAKE_MODULE_PATH "${VST3_SDK_ROOT}/cmake/modules")

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

set(VSTSDK_INCLUDE_DIR ${VST3_SDK_ROOT})
set(VSTSDK_PLUGIN_SOURCE
  ${VST3_SDK_ROOT}/public.sdk/source/vst/vstaudioeffect.cpp
  ${VST3_SDK_ROOT}/public.sdk/source/vst/vstaudioprocessoralgo.h
  ${VST3_SDK_ROOT}/public.sdk/source/vst/vsteditcontroller.h
  ${VST3_SDK_ROOT}/pluginterfaces/base/ibstream.h
  ${VST3_SDK_ROOT}/pluginterfaces/base/ustring.h
  ${VST3_SDK_ROOT}/pluginterfaces/vst/ivstevents.h
  ${VST3_SDK_ROOT}/pluginterfaces/vst/ivstparameterchanges.h
  ${VST3_SDK_ROOT}/pluginterfaces/vst/vstpresetkeys.h
)

#if (SMTG_CREATE_VST2_VERSION)
    set(regrader_vst2_sources
        ${VST3_SDK_ROOT}/public.sdk/source/common/memorystream.cpp
        ${VST3_SDK_ROOT}/public.sdk/source/vst/hosting/eventlist.cpp
        ${VST3_SDK_ROOT}/public.sdk/source/vst/hosting/hostclasses.cpp
        ${VST3_SDK_ROOT}/public.sdk/source/vst/hosting/parameterchanges.cpp
        ${VST3_SDK_ROOT}/public.sdk/source/vst/hosting/processdata.cpp
        ${VST3_SDK_ROOT}/public.sdk/source/vst/vst2wrapper/vst2wrapper.cpp
        ${VST3_SDK_ROOT}/public.sdk/source/vst/vst2wrapper/vst2wrapper.h
        ${VST3_SDK_ROOT}/public.sdk/source/vst2.x/audioeffect.cpp
        ${VST3_SDK_ROOT}/public.sdk/source/vst2.x/audioeffectx.cpp
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
    src/bitcrusher.h
    src/bitcrusher.cpp
    src/decimator.h
    src/decimator.cpp
    src/filter.h
    src/filter.cpp
    src/flanger.h
    src/flanger.cpp
    src/lfo.h
    src/lfo.cpp
    src/lowpassfilter.h
    src/lowpassfilter.cpp
    src/limiter.h
    src/limiter.cpp
    src/paramids.h
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

if(MAC OR WIN)
    set(regrader_sources ${regrader_sources} ${regrader_vst2_sources})
endif()

set(target regrader)

##############
# Audio Unit #
##############

if (JAMBA_ENABLE_AUDIO_UNIT)
    set(JAMBA_TEST_CASES_DIR "${JAMBA_ROOT}/test/cpp")
    set(JAMBA_TEST_CASES_SOURCES
        ${JAMBA_TEST_CASES_DIR}/pongasoft/Utils/Collection/test-CircularBuffer.cpp
        ${JAMBA_TEST_CASES_DIR}/pongasoft/Utils/Concurrent/test-concurrent.cpp
        ${JAMBA_TEST_CASES_DIR}/pongasoft/Utils/Concurrent/test-concurrent_lockfree.cpp
        ${JAMBA_TEST_CASES_DIR}/pongasoft/Utils/test-Lerp.cpp
        ${JAMBA_TEST_CASES_DIR}/pongasoft/Utils/test-StringUtils.cpp
        ${JAMBA_TEST_CASES_DIR}/pongasoft/VST/GUI/Params/test-GUIParameters.cpp
        ${JAMBA_TEST_CASES_DIR}/pongasoft/VST/GUI/Params/test-ParamAware.cpp
        ${JAMBA_TEST_CASES_DIR}/pongasoft/VST/GUI/Views/test-CustomViewCreator.cpp
        ${JAMBA_TEST_CASES_DIR}/pongasoft/VST/GUI/Views/test-SelfContainedViewListener.cpp
        ${JAMBA_TEST_CASES_DIR}/pongasoft/VST/test-AudioBuffers.cpp
        ${JAMBA_TEST_CASES_DIR}/pongasoft/VST/test-AudioUtils.cpp
        ${JAMBA_TEST_CASES_DIR}/pongasoft/VST/test-ParamConverters.cpp
        ${JAMBA_TEST_CASES_DIR}/pongasoft/VST/test-SampleRateBasedClock.cpp
        ${JAMBA_TEST_CASES_DIR}/pongasoft/VST/Utils/test-Utils.cpp
        ${JAMBA_TEST_CASES_DIR}/pongasoft/VST/Utils/test-FastWriteMemoryStream.cpp
        ${JAMBA_TEST_CASES_DIR}/pongasoft/VST/Utils/test-ReadOnlyMemoryStream.cpp
    )

    include(jamba.cmake)

    set(CPP_SOURCES src)

    configure_file(${CPP_SOURCES}/version.h ${CMAKE_BINARY_DIR}/generated/version.h)
    include_directories(${CMAKE_BINARY_DIR}/generated/)

    set(vst_resources
        "resource/background.png"
        "resource/slider_background.png"
        "resource/slider_handle.png"
        "resource/slider_handle_2.0x.png"
    )

    jamba_add_vst_plugin(
        TARGET              "${target}"
        RELEASE_FILENAME    "${target}.component"
        TARGETS_PREFIX      "jmb_"
        VST_SOURCES         "${regrader_sources}"
        UIDESC              "resource/regrader.uidesc"
        RESOURCES           "${vst_resources}"
        TEST_CASE_SOURCES   "${JAMBA_TEST_CASES_SOURCES}"
        TEST_LINK_LIBRARIES "jamba"
    )

else()

#######
# VST #
#######

smtg_add_vst3plugin(${target} ${regrader_sources})

include_directories(${VSTSDK_INCLUDE_DIR})

if (UNIX)
    target_link_libraries(${target} PRIVATE ${VST3_SDK_ROOT}/build/lib/Release/libbase.a)
    target_link_libraries(${target} PRIVATE ${VST3_SDK_ROOT}/build/lib/Release/libpluginterfaces.a)
    target_link_libraries(${target} PRIVATE ${VST3_SDK_ROOT}/build/lib/Release/libsdk.a)
    target_link_libraries(${target} PRIVATE ${VST3_SDK_ROOT}/build/lib/Release/libvstgui_support.a)
    target_link_libraries(${target} PRIVATE ${VST3_SDK_ROOT}/build/lib/Release/libvstgui_uidescription.a)
    target_link_libraries(${target} PRIVATE ${VST3_SDK_ROOT}/build/lib/Release/libvstgui.a)
endif()
if (WIN)
    target_link_libraries(${target} PRIVATE ${VST3_SDK_ROOT}/build/lib/Release/base.lib)
    target_link_libraries(${target} PRIVATE ${VST3_SDK_ROOT}/build/lib/Release/pluginterfaces.lib)
    target_link_libraries(${target} PRIVATE ${VST3_SDK_ROOT}/build/lib/Release/sdk.lib)
    target_link_libraries(${target} PRIVATE ${VST3_SDK_ROOT}/build/lib/Release/vstgui_support.lib)
    target_link_libraries(${target} PRIVATE ${VST3_SDK_ROOT}/build/lib/Release/vstgui_uidescription.lib)
    target_link_libraries(${target} PRIVATE ${VST3_SDK_ROOT}/build/lib/Release/vstgui.lib)
endif()


# macOS specific libraries

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
    ## these are iOS-specific and require the following frameworks:
    ## UIKit CoreGraphics QuartzCore CoreText Accelerate ImageIO MobileCoreServices
    #target_sources(${target} PRIVATE
    #    ${VST3_SDK_ROOT}/vstgui4/vstgui/vstgui_uidescription.cpp
    #    ${VST3_SDK_ROOT}/vstgui4/vstgui/vstgui_ios.mm
    #    ${VST3_SDK_ROOT}/vstgui4/vstgui/plugin-bindings/vst3editor.cpp
    #    ${VST3_SDK_ROOT}/public.sdk/source/vst/vstguieditor.cpp
    #)
    set_target_properties(regrader PROPERTIES
        BUNDLE true
        BUNDLE_EXTENSION "vst3"
        XCODE_ATTRIBUTE_WRAPPER_EXTENSION "vst3"
        MACOSX_BUNDLE_INFO_PLIST "${CMAKE_CURRENT_SOURCE_DIR}/mac/Info.plist"
        MACOSX_BUNDLE_BUNDLE_NAME "Regrader"
        MACOSX_BUNDLE_GUI_IDENTIFIER "nl.igorski.regrader"
        MACOSX_BUNDLE_ICON_FILE ""
        MACOSX_BUNDLE_SHORT_VERSION_STRING "1.0.4"
        MACOSX_BUNDLE_COPYRIGHT "igorski.nl © 2018"
    )
endif()

# Steinberg SDK

if (LINUX)
    target_sources (${target} PRIVATE
	${VST3_SDK_ROOT}/public.sdk/source/main/linuxmain.cpp
	${VST3_SDK_ROOT}/vstgui4/vstgui/plugin-bindings/vst3editor.cpp
    )
    set(VSTGUI_LTO_COMPILER_FLAGS "-O3 -flto")
    set(VSTGUI_LTO_LINKER_FLAGS "")
    find_package(X11 REQUIRED)
    find_package(Freetype REQUIRED)
    find_package(PkgConfig REQUIRED)
    pkg_check_modules(LIBXCB REQUIRED xcb)
    pkg_check_modules(LIBXCB_UTIL REQUIRED xcb-util)
    pkg_check_modules(LIBXCB_CURSOR REQUIRED xcb-cursor)
    pkg_check_modules(LIBXCB_KEYSYMS REQUIRED xcb-keysyms)
    pkg_check_modules(LIBXCB_XKB REQUIRED xcb-xkb)
    pkg_check_modules(LIBXKB_COMMON REQUIRED xkbcommon)
    pkg_check_modules(LIBXKB_COMMON_X11 REQUIRED xkbcommon-x11)
    set(LINUX_LIBRARIES
        ${X11_LIBRARIES}
        ${FREETYPE_LIBRARIES}
        ${LIBXCB_LIBRARIES}
        ${LIBXCB_UTIL_LIBRARIES}
        ${LIBXCB_CURSOR_LIBRARIES}
        ${LIBXCB_KEYSYMS_LIBRARIES}
        ${LIBXCB_XKB_LIBRARIES}
        ${LIBXKB_COMMON_LIBRARIES}
        ${LIBXKB_COMMON_X11_LIBRARIES}
        cairo
        fontconfig
        dl
    )
    target_link_libraries(${target} PRIVATE ${LINUX_LIBRARIES}) 
endif()
target_include_directories(${target} PUBLIC ${VST3_SDK_ROOT}/vstgui4)

# Resources

smtg_add_vst3_resource(${target} "resource/regrader.uidesc")
smtg_add_vst3_resource(${target} "resource/background.png")
smtg_add_vst3_resource(${target} "resource/slider_background.png")
smtg_add_vst3_resource(${target} "resource/slider_handle.png")
smtg_add_vst3_resource(${target} "resource/slider_handle_2.0x.png")

if(MAC)
    smtg_set_bundle(${target} INFOPLIST "${CMAKE_CURRENT_SOURCE_DIR}/mac/Info.plist" PREPROCESS)
elseif(WIN)
    target_sources(${target} PRIVATE resource/regrader.rc)
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

endif()

######################
# Installation paths #
######################

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
