# For more information about using CMake with Android Studio, read the
# documentation: https://d.android.com/studio/projects/add-native-code.html

# Sets the minimum version of CMake required to build the native library.

cmake_minimum_required(VERSION 3.10.2)

# Creates and names a library, sets it as either STATIC
# or SHARED, and provides the relative paths to its source code.
# You can define multiple libraries, and CMake builds it for you.
# Gradle automatically packages shared libraries with your APK.

project("jpglib")

#jpeg lib

add_library(
        jpeg
        SHARED
        native-lib.cpp
        jpeg/jaricom.c jpeg/jcapimin.c jpeg/jcapistd.c jpeg/jcarith.c jpeg/jccoefct.c jpeg/jccolor.c
        jpeg/jcdctmgr.c jpeg/jchuff.c jpeg/jcinit.c jpeg/jcmainct.c jpeg/jcmarker.c jpeg/jcmaster.c
        jpeg/jcomapi.c jpeg/jcparam.c jpeg/jcprepct.c jpeg/jcsample.c jpeg/jctrans.c jpeg/jdapimin.c
        jpeg/jdapistd.c jpeg/jdarith.c jpeg/jdatadst.c jpeg/jdatasrc.c jpeg/jdcoefct.c jpeg/jdcolor.c
        jpeg/jddctmgr.c jpeg/jdhuff.c jpeg/jdinput.c jpeg/jdmainct.c jpeg/jdmarker.c jpeg/jdmaster.c
        jpeg/jdmerge.c jpeg/jdpostct.c jpeg/jdsample.c jpeg/jdtrans.c jpeg/jerror.c jpeg/jfdctflt.c
        jpeg/jfdctfst.c jpeg/jfdctint.c jpeg/jidctflt.c jpeg/jidctfst.c jpeg/jidctint.c jpeg/jquant1.c
        jpeg/jquant2.c jpeg/jutils.c jpeg/jmemmgr.c jpeg/jmemnobs.c)

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
        jpeg

        # Links the target library to the log library
        # included in the NDK.
        ${log-lib})