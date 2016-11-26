cmake_minimum_required(VERSION 2.8)
project(SDLPoP)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11 -Wall")
set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -std=gnu99 -Wall")

if (WIN32)
    # Use the -mwindows compiler flag when compiling with MinGW to hide the console window
    # Only do this when not in debug mode
    if (NOT (CMAKE_BUILD_TYPE STREQUAL "Debug"))
        set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -mwindows")
        set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -mwindows")
    endif (NOT (CMAKE_BUILD_TYPE STREQUAL "Debug"))
endif(WIN32)

# Have CMake output binaries to the main directory
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY "${SDLPoP_SOURCE_DIR}/..")

# If SDL_2, SDL2_image and SDL2_mixer are not in the toolchain/build environment, specify /include and /lib paths:

#set(DIR_SDL2 "C:/SDL2-2.0.3")
#include_directories(${DIR_SDL2}/include)
#link_directories(${DIR_SDL2}/lib)

set(SOURCE_FILES
        main.c
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
        seqtbl.c
        options.c
        replay.c
        types.h
        )

if(WIN32)
    set(LIBS mingw32 SDL2main SDL2 SDL2.dll SDL2_image SDL2_mixer)
else()
    set(LIBS SDL2main SDL2 SDL2_image SDL2_mixer m dl)
endif()

# Target: prince executable
# =========================

add_executable(prince ${SOURCE_FILES})
target_link_libraries(prince ${LIBS})

# Target: mod executable or dll
# =============================

# Set to 0 if you want to build a launchable mod.exe instead of a shared library
set(SHOULD_BUILD_MOD_AS_DLL 1)

# List any additional custom source files here:
set(MOD_SOURCE_FILES )

if (SHOULD_BUILD_MOD_AS_DLL)
    add_library(mod SHARED ${SOURCE_FILES} ${MOD_SOURCE_FILES})
    set_target_properties(mod PROPERTIES PREFIX "") # prevent the dll being named "libmod.dll"
else()
    add_executable(mod ${SOURCE_FILES} ${MOD_SOURCE_FILES})
endif()

# List additional preprocessor definitions here:
target_compile_definitions(mod PUBLIC IS_SDLPOP_MOD_EXE )

target_link_libraries(mod ${LIBS})
