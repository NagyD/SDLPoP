cmake_minimum_required(VERSION 2.8)
project(SDLPoP)

set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -Wall -std=gnu99")

# have CMake output binaries to the directory that contains the source files
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY "${SDLPoP_SOURCE_DIR}/..")

if (WIN32)
    if (MSVC)
        # Don't let Visual Studio run CMake
        message(SEND_ERROR "To build using MSVC on Windows, you can use NMake or run build.bat.")
        return()
    endif()

    # Use the -mwindows compiler flag when compiling with MinGW to hide the console window
    # Only do this when not in debug mode
    if (NOT (CMAKE_BUILD_TYPE STREQUAL "Debug"))
        set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -mwindows")
        set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -mwindows")
    endif (NOT (CMAKE_BUILD_TYPE STREQUAL "Debug"))

    # SDLPoP requires the SDL_2, SDL2_image and SDL2_mixer development libraries.
    # You can pass the SDL2 location to CMake like so: -DSDL2="C:/work/libraries/SDL2-2.0.5"
    # Or alternatively, specify the SDL2 location below:

    #set(SDL2 "C:/work/libraries/SDL2-2.0.5")

    # If the location of SDL2 is not specified, we will try to guess it.
    if (NOT(DEFINED SDL2))
        cmake_policy(SET CMP0015 NEW) # suppress warning about relative paths
        set(SDL2 "../../SDL2-2.0.5")
    endif()

    # Automatically detect whether we need the x86 or x64 version of the SDL2 library.
    if (CMAKE_SIZEOF_VOID_P EQUAL 8)
        set(SDL2_ARCH "x86_64-w64-mingw32")
    else()
        set(SDL2_ARCH "i686-w64-mingw32")
    endif()

    include_directories(${SDL2}/${SDL2_ARCH}/include)
    link_directories(${SDL2}/${SDL2_ARCH}/lib)
endif()

set(SOURCE_FILES
        main.c
        common.h
        config.h
        data.c
        data.h
        proto.h
        types.h
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
        seqtbl.c
        options.c
        replay.c
        lighting.c
        screenshot.c
        icon.rc
        )
add_executable(prince ${SOURCE_FILES})

if(WIN32)
    target_link_libraries(prince mingw32 SDL2main SDL2 SDL2.dll SDL2_image SDL2_mixer)
else()
    target_link_libraries(prince SDL2main SDL2 SDL2_image SDL2_mixer m)
endif()

