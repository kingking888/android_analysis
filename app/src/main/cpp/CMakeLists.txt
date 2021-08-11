# For more information about using CMake with Android Studio, read the
# documentation: https://d.android.com/studio/projects/add-native-code.html

# Sets the minimum version of CMake required to build the native library.

cmake_minimum_required(VERSION 3.10.2)

# Declares and names the project.

project("local_caller")
enable_language(ASM)
# Creates and names a library, sets it as either STATIC
# or SHARED, and provides the relative paths to its source code.
# You can define multiple libraries, and CMake builds them for you.
# Gradle automatically packages shared libraries with your APK.

#include_directories(
#        ${DobbyHome}/include
#        ${DobbyHome}/source
#        ${DobbyHome}/builtin-plugin
#        ${DobbyHome}/builtin-plugin/AndroidRestriction
#        ${DobbyHome}/builtin-plugin/SymbolResolver
#        ${DobbyHome}/external/logging
#)
if (${CMAKE_ANDROID_ARCH_ABI} STREQUAL "armeabi-v7a"  OR ${CMAKE_ANDROID_ARCH_ABI} STREQUAL "arm64-v8a")
    # arch prefix
    if (${CMAKE_ANDROID_ARCH_ABI} STREQUAL "arm64-v8a")
        file(GLOB native_hook_asm_src
                "route_func/asm64/*.asm")
    else ()
        file(GLOB native_hook_asm_src
                "route_func/asm/*.asm")
    endif ()


    file(GLOB native_hook_src
            "hook_libc/*.h"
            "hook_libc/*.cpp"
            "hook_libc/*.c"

            "utils/*.h"
            "utils/*.cpp"
            "utils/*.c"

            "xposed/*.h"
            "xposed/*.cpp"
            "xposed/*.c"

            "libxhook/*.h"
            "libxhook/*.cpp"
            "libxhook/*.c"

            "dlfc/*.h"
            "dlfc/*.cpp"
            "dlfc/*.c"

            "dobby/include/dobby.h"

            "hook_jni/*.h"
            "hook_jni/*.cpp"
            "hook_jni/*.c"

            "route_func/*.h"
            "route_func/*.cpp"

            "json/*.h"
            "json/*.cpp"
            "json/*.inl"

            "*.h"
            "*.cpp"
            "*.c"
            "export.def"
            )


    add_library( # Sets the name of the library.
            native_hook

            # Sets the library as a shared library.
            SHARED
            # Provides a relative path to your source file(s).
            ${native_hook_asm_src}
            ${native_hook_src})

    set(DobbyHome "./dobby")
    #set(GENERATE_SHARED OFF)
    #set(DOBBY_DEBUG ON)
    add_subdirectory(${DobbyHome} dobby)
    target_link_libraries(native_hook dobby)

    find_library( # Sets the name of the path variable.
            log-lib
            # Specifies the name of the NDK library that
            # you want CMake to locate.
            log)

    # Specifies libraries CMake should link to your target library. You
    # can link multiple libraries, such as libraries you define in this
    # build script, prebuilt third-party libraries, or system libraries.

    target_link_libraries( # Specifies the target library.
            native_hook

            # Links the target library to the log library
            # included in the NDK.
            ${log-lib})
endif ()