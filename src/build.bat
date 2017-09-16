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

:: To override the directory for SDL2 library files, simply set the SDL environment variable in the command shell.
:: (You could do that from the command-line, or from a wrapper script that calls this one.)

if [%SDL2%]==[] (
  set SDL2=..\..\SDL2-2.0.6
)

if not exist %SDL2% (
  echo Problem^: Could not find SDL2 directory.
  echo Tried to look here^: %SDL2%
  echo,
  echo To specify the SDL2 directory, set the SDL2 environment variable.
  echo Example command:
  echo set "SDL2=C:\work\libraries\SDL2-2.0.6"
  exit /b
)

:: To choose the build configuration, specify either "debug" or "release" as command-line parameter for this build script.

if [%1]==[debug] goto build_type_debug
if [%1]==[release] goto build_type_release
echo Build type not specified, compiling in release mode...
echo To specify the build type, run as "build.bat debug" or "build.bat release".

:build_type_debug
set BuildTypeCompilerFlags= /MDd /Od
set PreprocessorDefinitions= -DDEBUG=1
goto compile

:build_type_release
set BuildTypeCompilerFlags= /MD /O2
set PreprocessorDefinitions=

:compile
set SourceFiles= main.c data.c seg000.c seg001.c seg002.c seg003.c seg004.c seg005.c seg006.c seg007.c seg008.c seg009.c seqtbl.c replay.c options.c lighting.c screenshot.c
set CommonCompilerFlags= /nologo /MP /fp:fast /GR- /wd4048 %PreprocessorDefinitions% /I"%SDL2%\include"
set CommonLinkerFlags= /subsystem:windows,5.01 /libpath:"%SDL2%\lib\%VSCMD_ARG_TGT_ARCH%" SDL2main.lib SDL2.lib SDL2_image.lib SDL2_mixer.lib icon.res /out:..\prince.exe

rc /nologo /fo icon.res icon.rc
cl %BuildTypeCompilerFlags% %CommonCompilerFlags% %SourceFiles% /link %CommonLinkerFlags%

if %ERRORLEVEL% == 0 (goto success)
echo There were errors.
goto cleanup

:success
echo Output: ..\prince.exe

:cleanup
del icon.res
del *.obj

