cmake_minimum_required(VERSION 3.13)
project(SDLPoP)

set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -Wall -std=c99")

set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -Werror=implicit-function-declaration")

# have CMake output binaries to the directory that contains the source files
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY "${SDLPoP_SOURCE_DIR}/..")

# SDLPoP requires the SDL2 and SDL2_image development libraries.
# You can pass the SDL2 location to CMake like so: -DSDL2="C:/work/libraries/SDL2-2.0.8"
# Or alternatively, specify the SDL2 location below:

#set(SDL2 "C:/work/libraries/SDL2-2.0.8")

# On macOS, if you used Homebrew to install SDL2, the location may be something like this:

#set(SDL2 "/usr/local/Cellar/sdl2/2.0.5")

if(APPLE)

    set(SDL2_ROOT "" CACHE PATH
        "Root directory containing SDL2.framework and libSDL2_image")

    if(NOT SDL2_ROOT AND DEFINED SDL2)
        set(SDL2_ROOT "${SDL2}")
    endif()

    if(NOT SDL2_ROOT)
        message(FATAL_ERROR
            "On macOS, specify the SDL libraries using:\n"
            "  -DSDL2_ROOT=/path/to/SDL-libraries")
    endif()

    set(SDL2_FRAMEWORK
        "${SDL2_ROOT}/SDL2.framework")

    set(SDL2_IMAGE_LIBRARY
        "${SDL2_ROOT}/lib/libSDL2_image-2.0.0.900.0.dylib")

    include_directories(
        "${SDL2_ROOT}/include"
    )

elseif (NOT(WIN32) AND (DEFINED SDL2))

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

# macOS bundle metadata

set(CMAKE_OSX_DEPLOYMENT_TARGET "11.0")

set(SDLPoP_VERSION "1.24 RC")
set(SDLPoP_BUILD_VERSION "1.24.0")

set(MACOSX_BUNDLE_INFO_STRING
    "SDLPoP - Prince of Persia")

set(MACOSX_BUNDLE_ICON_FILE
    "icon.icns")

set_source_files_properties(
    ${MACOSX_BUNDLE_ICON_FILE}
    PROPERTIES MACOSX_PACKAGE_LOCATION Resources
)

set(MACOSX_BUNDLE_GUI_IDENTIFIER
    "org.princed.SDLPoP")

set(MACOSX_BUNDLE_LONG_VERSION_STRING
    ${SDLPoP_VERSION})

set(MACOSX_BUNDLE_BUNDLE_NAME
    "SDLPoP")

set(MACOSX_BUNDLE_SHORT_VERSION_STRING
    ${SDLPoP_VERSION})

set(MACOSX_BUNDLE_BUNDLE_VERSION
    ${SDLPoP_BUILD_VERSION})

set(MACOSX_BUNDLE_COPYRIGHT
    "GNU General Public Licence, v3")


    add_executable(prince MACOSX_BUNDLE ${SOURCE_FILES} ${MACOSX_BUNDLE_ICON_FILE})

    set_target_properties(prince PROPERTIES
        MACOSX_BUNDLE_INFO_PLIST
        "${CMAKE_CURRENT_SOURCE_DIR}/Info.plist.in"
    )

    add_custom_command(TARGET prince POST_BUILD

        COMMAND ${CMAKE_COMMAND} -E copy_directory
        "${CMAKE_SOURCE_DIR}/../data"
        "$<TARGET_BUNDLE_DIR:prince>/Contents/Resources/data"

        COMMAND ${CMAKE_COMMAND} -E copy_directory
        "${CMAKE_SOURCE_DIR}/../doc"
        "$<TARGET_BUNDLE_DIR:prince>/Contents/Resources/doc"

        COMMAND ${CMAKE_COMMAND} -E copy
        "${CMAKE_SOURCE_DIR}/../SDLPoP.ini"
        "$<TARGET_BUNDLE_DIR:prince>/Contents/Resources/SDLPoP.ini"
    )

    add_custom_command(TARGET prince POST_BUILD

	# Bundle the complete SDL2 framework.
        COMMAND ditto
        "${SDL2_FRAMEWORK}"
        "$<TARGET_BUNDLE_DIR:prince>/Contents/Frameworks/SDL2.framework"

        # Bundle SDL2_image.
        COMMAND ${CMAKE_COMMAND} -E copy
        "${SDL2_IMAGE_LIBRARY}"
        "$<TARGET_BUNDLE_DIR:prince>/Contents/Frameworks/libSDL2_image-2.0.0.900.0.dylib"

        COMMAND ${CMAKE_COMMAND} -E create_symlink
        "libSDL2_image-2.0.0.900.0.dylib"
        "$<TARGET_BUNDLE_DIR:prince>/Contents/Frameworks/libSDL2_image-2.0.0.dylib"
    )

endif()

if(WIN32)
    target_link_libraries(prince mingw32 SDL2main SDL2 SDL2_image)
elseif(APPLE)
    set_target_properties(prince PROPERTIES
        SKIP_BUILD_RPATH TRUE
    )

    target_link_libraries(prince
        "-framework SDL2"
        "${SDL2_IMAGE_LIBRARY}"
        m
    )

    target_link_options(prince PRIVATE
        "-F${SDL2_ROOT}"
        "-Wl,-rpath,@executable_path/../Frameworks"
    )

elseif(PSP) # Linux, *BSD, etc.
	set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -O3")
	include(FindPkgConfig)
	pkg_search_module(SDL2 REQUIRED sdl2)
	pkg_search_module(SDL2IMAGE REQUIRED SDL2_image)
	target_include_directories(prince PRIVATE ${SDL2_INCLUDE_DIRS})
	target_link_libraries(prince PRIVATE
	${SDL2_LIBRARIES} ${SDL2IMAGE_LIBRARIES}
	)
	create_pbp_file(
		TARGET prince
		ICON_PATH ${CMAKE_SOURCE_DIR}/iconPSP.png
		BACKGROUND_PATH NULL
		PREVIEW_PATH NULL
		TITLE prince
	)
else()
    	target_link_libraries(prince SDL2 SDL2_image m)
endif()
