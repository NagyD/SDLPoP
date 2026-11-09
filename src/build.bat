@echo off
setlocal
cd %~dp0

:: Check that we have access to the MSVC compiler.

where /q cl
if ERRORLEVEL 1 (
  echo Problem^: the MSVC compiler ^(cl^) cannot not found.
  echo The solution is to run vcvarsall.bat, which sets the necessary environment variables.
  echo,
  echo Example command for VS2017^:
  echo call "C:\Program Files (x86)\Microsoft Visual Studio\2017\Community\VC\Auxiliary\Build\vcvarsall.bat" x86
  echo,
  echo Example command for VS2015^:
  echo call "%%VS140COMNTOOLS%%..\..\VC\vcvarsall.bat" x86
  exit /b
)

:: SDL3 is the default backend. To build against SDL2 instead, set the
:: SDLPOP_SDL2 environment variable (e.g. "set SDLPOP_SDL2=1") before running.
:: Override the SDL directory by setting the SDL3 (or SDL2) environment variable.

if not [%SDLPOP_SDL2%]==[] goto sdl2_setup

:sdl3_setup
if [%SDL3%]==[] (
  set SDL3=..\..\SDL3
)
set SDL_DIR=%SDL3%
set SDL_LIBS=SDL3.lib SDL3_image.lib
set SDL_DEFINES=-DUSE_SDL3
goto sdl_done

:sdl2_setup
if [%SDL2%]==[] (
  set SDL2=..\..\SDL2-2.0.6
)
set SDL_DIR=%SDL2%
set SDL_LIBS=SDL2main.lib SDL2.lib SDL2_image.lib
set SDL_DEFINES=

:sdl_done
if not exist %SDL_DIR% (
  echo Problem^: Could not find the SDL directory.
  echo Tried to look here^: %SDL_DIR%
  echo,
  echo To specify it, set the SDL3 (or SDL2) environment variable.
  echo Example command:
  echo set "SDL3=C:\work\libraries\SDL3"
  exit /b
)

:: To choose the build configuration, specify either "debug" or "release" as command-line parameter for this build script.

if [%1]==[debug] goto build_type_debug
if [%1]==[release] goto build_type_release
echo Build type not specified, compiling in release mode...
echo To specify the build type, run as "build.bat debug" or "build.bat release".
goto build_type_release

:build_type_debug
set BuildTypeCompilerFlags= /MTd /Od /Z7
set PreprocessorDefinitions= -DDEBUG=1
goto compile

:build_type_release
set BuildTypeCompilerFlags= /MT /O2
set PreprocessorDefinitions=

:compile
set SourceFiles= main.c data.c seg000.c seg001.c seg002.c seg003.c seg004.c seg005.c seg006.c seg007.c seg008.c seg009.c seqtbl.c replay.c options.c lighting.c screenshot.c menu.c midi.c opl3.c stb_vorbis.c sdl2_to_sdl3.c
set CommonCompilerFlags= /nologo /MP /fp:fast /GR- /wd4048 %PreprocessorDefinitions% %SDL_DEFINES% /I"%SDL_DIR%\include"
set CommonLinkerFlags= /subsystem:windows,5.01 /libpath:"%SDL_DIR%\lib\%VSCMD_ARG_TGT_ARCH%" %SDL_LIBS% Shell32.lib icon.res /out:..\prince.exe

rc /nologo /fo icon.res icon.rc
cl %BuildTypeCompilerFlags% %CommonCompilerFlags% %SourceFiles% /link %CommonLinkerFlags%

if %ERRORLEVEL% == 0 (goto success)
echo There were errors.
goto cleanup

:success
echo Output: ..\prince.exe

:cleanup
if [%1]==[debug] exit /b
del icon.res 2> NUL
del *.obj 2> NUL
del ..\prince.exp 2> NUL
del ..\prince.lib 2> NUL
del ..\prince.pdb 2> NUL
