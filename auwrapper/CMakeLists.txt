cmake_minimum_required(VERSION 3.0.0)
set(CMAKE_COLOR_MAKEFILE ON)
set(CMAKE_CXX_STANDARD 11)

# path to Steinberg VST3 SDK
set(VSTSDK_PATH "/Library/VST_SDK/VST3_SDK" CACHE PATH "Path to VST3 SDK")

# path to CoreAudio SDK
# NOTE: this is not the same as the CoreAudio framework as provided with XCode but rather 'Core Audio Utility Classes'
# available at https://developer.apple.com/library/archive/samplecode/CoreAudioUtilityClasses/Introduction/Intro.html
# under the "Download Sample Code" button
set(SMTG_COREAUDIO_SDK_PATH "/Library/VST_SDK/CoreAudio" CACHE PATH "Path to CoreAudio SDK")

#-------------------------------------------------------------------------------
# Includes
#-------------------------------------------------------------------------------

# Include Regrader VST3 build
add_subdirectory("${CMAKE_CURRENT_SOURCE_DIR}/.." build)

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

#if(MAC AND XCODE AND SMTG_COREAUDIO_SDK_PATH)
if(SMTG_COREAUDIO_SDK_PATH)
    set(target regrader_au)
    set(${target}_sources
        doc.cpp
        audiounitconfig.h
        Info.plist
    )
    add_library(${target} MODULE ${${target}_sources})
    set_target_properties(${target} PROPERTIES BUNDLE TRUE)
    set_target_properties(${target} PROPERTIES XCODE_ATTRIBUTE_GENERATE_MASTER_OBJECT_FILE "YES")
    set_target_properties(${target} PROPERTIES XCODE_ATTRIBUTE_OTHER_LDFLAGS "-all_load")
    set_target_properties(${target} PROPERTIES XCODE_ATTRIBUTE_GENERATE_PKGINFO_FILE "YES")
    set_target_properties(${target} PROPERTIES XCODE_ATTRIBUTE_WRAPPER_EXTENSION "component")
    set_target_properties(${target} PROPERTIES LIBRARY_OUTPUT_DIRECTORY ${VST3_OUTPUT_DIR})
    target_link_libraries(${target} PRIVATE "${VSTSDK_PATH}/public.sdk/source/vst/auwrapper/build/lib/Release/libauwrapper.a")
    smtg_set_bundle(${target} INFOPLIST "${CMAKE_CURRENT_LIST_DIR}/Info.plist" PREPROCESS)

    set(outputdir ${VST3_OUTPUT_DIR}/$<CONFIG>)

    add_dependencies(${target} regrader)
    add_custom_command(TARGET ${target} POST_BUILD COMMAND /bin/mkdir "-p" ${outputdir}/${target}.component/Contents/Resources)

    # either create symbolic link to VST3 or copy it into the output (the latter necessary for a release build)
#   add_custom_command(TARGET ${target} POST_BUILD COMMAND /bin/ln "-sf" "${outputdir}/regrader.vst3" "${outputdir}/${target}.component/Contents/Resources/plugin.vst3")
    add_custom_command(TARGET ${target} POST_BUILD COMMAND /bin/cp "-R" "${outputdir}/regrader.vst3" "${outputdir}/${target}.component/Contents/Resources/plugin.vst3")

# desperation sets in, what is this mysterious .bundle and where is the .plist ??
#    add_custom_command(TARGET ${target} POST_BUILD COMMAND /bin/cp "-R" "${outputdir}/${target}.bundle/Contents" "${outputdir}/${target}.component")
#    add_custom_command(TARGET ${target} POST_BUILD COMMAND /bin/cp "-R" "${CMAKE_CURRENT_LIST_DIR}/Info.plist" "${outputdir}/${target}.component/Contents")

    # create symbolic link in home plugins folder
    add_custom_command(TARGET ${target} POST_BUILD COMMAND /bin/ln "-sf" "${outputdir}/${target}.component" "~/Library/Audio/Plug-Ins/Components/")

    execute_process(COMMAND xcrun --find Rez OUTPUT_VARIABLE OSX_REZ_COMMAND OUTPUT_STRIP_TRAILING_WHITESPACE)
    add_custom_command(TARGET ${target} POST_BUILD COMMAND "${OSX_REZ_COMMAND}"
        "-d" "SystemSevenOrLater=1"
        "-script" "Roman"
        "-d" "i386_YES"
        "-d" "x86_64_YES"
        "-is" "${CMAKE_OSX_SYSROOT}"
        "-I" "${CMAKE_OSX_SYSROOT}/System/Library/Frameworks/CoreServices.framework/Frameworks/CarbonCore.framework/Versions/A/Headers"
        "-I" "/System/Library/Frameworks/CoreServices.framework/Frameworks/CarbonCore.framework/Versions/A/Headers"
        "-I" "${SMTG_COREAUDIO_SDK_PATH}/AudioUnits/AUPublic/AUBase"
        "-I" "${CMAKE_CURRENT_LIST_DIR}"
        "-o" "${outputdir}/${target}.component/Contents/Resources/regrader_au.rsrc"
        "-useDF"
        "${CMAKE_CURRENT_LIST_DIR}/auresource.r"
    )

else()
    message(FATAL_ERROR "Building as Audio Unit requires macOS, Xcode and the CoreAudio SDK")
endif()
