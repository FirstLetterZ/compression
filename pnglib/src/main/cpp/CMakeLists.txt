# For more information about using CMake with Android Studio, read the
# documentation: https://d.android.com/studio/projects/add-native-code.html

# Sets the minimum version of CMake required to build the native library.

cmake_minimum_required(VERSION 3.10.2)

# Creates and names a library, sets it as either STATIC
# or SHARED, and provides the relative paths to its source code.
# You can define multiple libraries, and CMake builds it for you.
# Gradle automatically packages shared libraries with your APK.

project("pnglib")

find_package(zlib REQUIRED)

add_library(
        png
        SHARED
        png-lib-bridge.cpp
        png/arm/arm_init.c
        png/arm/filter_neon_intrinsics.c
        png/arm/palette_neon_intrinsics.c
        png/png.c
        png/pngerror.c
        png/pngget.c
        png/pngmem.c
        png/pngpread.c
        png/pngread.c
        png/pngrio.c
        png/pngrtran.c
        png/pngrutil.c
        png/pngset.c
        png/pngtrans.c
        png/pngwio.c
        png/pngwrite.c
        png/pngwtran.c
        png/pngwutil.c)

# Searches for a specified prebuilt library and stores the path as a
# variable. Because system libraries are included in the search path by
# default, you only need to specify the name of the public NDK library
# you want to add. CMake verifies that the library exists before
# completing its build.

find_library( # Sets the name of the path variable.
        log-lib

        # Specifies the name of the NDK library that
        # you want CMake to locate.
        log)

# Specifies libraries CMake should link to your target library. You
# can link multiple libraries, such as libraries you define in the
# build script, prebuilt third-party libraries, or system libraries.

target_link_libraries( # Specifies the target library.
        png
        PUBLIC ZLIB::ZLIB
        # Links the target library to the log library
        # included in the NDK.
        ${log-lib})