cmake_minimum_required(VERSION 2.8)
project(SDLPoP)

set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -Wall -std=gnu99")

# have CMake output binaries to the directory that contains the source files
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY "${SDLPoP_SOURCE_DIR}/..")

# SDLPoP requires the SDL2 and SDL2_image development libraries.
# You can pass the SDL2 location to CMake like so: -DSDL2="C:/work/libraries/SDL2-2.0.8"
# Or alternatively, specify the SDL2 location below:

#set(SDL2 "C:/work/libraries/SDL2-2.0.8")

# On macOS, if you used Homebrew to install SDL2, the location may be something like this:

#set(SDL2 "/usr/local/Cellar/sdl2/2.0.5")

if (NOT(WIN32) AND (DEFINED SDL2))
    include_directories(${SDL2}/include/SDL2)
    link_directories(${SDL2}/lib)
endif()

if (WIN32)
    if (MSVC)
        # Don't let Visual Studio run CMake
        message(SEND_ERROR "To build using MSVC on Windows, you can use NMake or run build.bat.")
        return()
    endif()

    # Use the -mwindows compiler flag when compiling with MinGW to hide the console window
    # Only do this when not in debug mode
    if (NOT (CMAKE_BUILD_TYPE STREQUAL "Debug"))
        set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -mwindows")
    endif (NOT (CMAKE_BUILD_TYPE STREQUAL "Debug"))

    # If the location of SDL2 is not specified, we will try to guess it.
    if (NOT(DEFINED SDL2))
        cmake_policy(SET CMP0015 NEW) # suppress warning about relative paths
        set(SDL2 "../../SDL2-2.0.8")
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
        midi.c opl3.c opl3.h
        replay.c
        lighting.c
        screenshot.c
        menu.c
        stb_vorbis.c
        icon.rc
        )

if (NOT(APPLE AND CREATE_BUNDLE))
    add_executable(prince ${SOURCE_FILES})
else()

    # macOS-specific:
    # By default, a normal binary executable will be created.
    # If you want an Application Bundle instead, pass CREATE_BUNDLE to CMake like so: -DCREATE_BUNDLE=1

    # NOTE: Currently, the SDL2 and SDL2_image frameworks are not being correctly included/linked in the bundle!
    #       As long as this isn't fixed, these bundles will only work if SDL2 and SDL2_image are installed.

    set (SDLPoP_VERSION "1.22")
    set (MACOSX_BUNDLE_INFO_STRING          ${PROJECT_NAME})
    set (MACOSX_BUNDLE_ICON_FILE            "icon.icns")
    set_source_files_properties(${MACOSX_BUNDLE_ICON_FILE} PROPERTIES MACOSX_PACKAGE_LOCATION Resources)
    set (MACOSX_BUNDLE_GUI_IDENTIFIER       "org.princed.SDLPoP")
    set (MACOSX_BUNDLE_LONG_VERSION_STRING  ${SDLPoP_VERSION})
    set (MACOSX_BUNDLE_BUNDLE_NAME          "SDLPoP")
    set (MACOSX_BUNDLE_SHORT_VERSION_STRING ${SDLPoP_VERSION})
    set (MACOSX_BUNDLE_BUNDLE_VERSION       ${SDLPoP_VERSION})
    set (MACOSX_BUNDLE_COPYRIGHT            "GNU General Public Licence, v3")
    add_executable(prince MACOSX_BUNDLE ${SOURCE_FILES} ${MACOSX_BUNDLE_ICON_FILE})
    add_custom_command(TARGET prince POST_BUILD
            COMMAND ${CMAKE_COMMAND} -E copy_directory
            ${CMAKE_SOURCE_DIR}/../data ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/prince.app/Contents/MacOS/data)
    add_custom_command(TARGET prince POST_BUILD
            COMMAND ${CMAKE_COMMAND} -E copy_directory
            ${CMAKE_SOURCE_DIR}/../doc ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/prince.app/Contents/MacOS/doc)
    add_custom_command(TARGET prince POST_BUILD
            COMMAND ${CMAKE_COMMAND} -E copy
            ${CMAKE_SOURCE_DIR}/../SDLPoP.ini ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/prince.app/Contents/MacOS/SDLPoP.ini)
endif()

if(WIN32)
    target_link_libraries(prince mingw32 SDL2main SDL2 SDL2_image)
elseif(APPLE)
    target_link_libraries(prince SDL2main SDL2 SDL2_image m)
else() # Linux, *BSD, etc.
    target_link_libraries(prince SDL2 SDL2_image m)
endif()
