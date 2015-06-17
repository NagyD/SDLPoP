cmake_minimum_required(VERSION 3.1)
project(SDLPoP)

# Use C++11 when using the C++ compiler
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11")


if (WIN32)
    # Use the -mwindows compiler flag when compiling with MinGW to hide the console window
    # Only do this when not in debug mode
    if (NOT (CMAKE_BUILD_TYPE STREQUAL "Debug"))
        set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -mwindows")
        set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -mwindows")
    endif (NOT (CMAKE_BUILD_TYPE STREQUAL "Debug"))
endif(WIN32)

# have CMake output binaries to the directory that contains the source files
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY "${SDLPoP_SOURCE_DIR}")

# if SDL_2, SDL2_image and SDL2_mixer are not in the toolchain/build environment, specify /include and /lib paths:

#set(DIR_SDL2 "C:/SDL2-2.0.3")
#include_directories(${DIR_SDL2}/include)
#link_directories(${DIR_SDL2}/lib)

set(SOURCE_FILES
    main.cpp # replace with main.c when using the C instead of the C++ compiler
    common.h
    config.h
    data.c
    data.h
    proto.h
    seg000.c
    seg001.c
    seg002.c
    seg003.c
    seg004.c
    seg005.c
    seg006.c
    seg007.c
    seg008.c
    seg009.c
    roomscript.c
    types.h
)
add_executable(SDLPoP ${SOURCE_FILES})

# @Todo: add a configuration for Mac OS X

target_link_libraries(SDLPoP mingw32 SDL2main SDL2 SDL2.dll SDL2_image SDL2_mixer)



