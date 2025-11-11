@echo off
setlocal
cd /d %~dp0

set SDL2=..\SDL2-2.30.0
set SDL2INC=%SDL2%\include
set SDL2LIB=%SDL2%\lib\x64
set CPPFLAGS=-I"%SDL2INC%"

set PATH=C:\msys64\mingw64\bin;%PATH%

echo Building SDLPoP...
echo.

gcc -Wall -std=c99 -O3 -ffast-math -I"%SDL2INC%" -I"%SDL2INC%\SDL2" -I"%SDL2INC%\..\include" -c main.c -o main.o
if errorlevel 1 goto error

gcc -Wall -std=c99 -O3 -ffast-math -I"%SDL2INC%" -I"%SDL2INC%\SDL2" -c data.c -o data.o

if errorlevel 1 goto error

gcc -Wall -std=c99 -O3 -ffast-math -I"%SDL2INC%" -I"%SDL2INC%\SDL2" -c seg000.c -o seg000.o
if errorlevel 1 goto error

gcc -Wall -std=c99 -O3 -ffast-math -I"%SDL2INC%" -I"%SDL2INC%\SDL2" -c seg001.c -o seg001.o
if errorlevel 1 goto error

gcc -Wall -std=c99 -O3 -ffast-math -I"%SDL2INC%" -I"%SDL2INC%\SDL2" -c seg002.c -o seg002.o
if errorlevel 1 goto error

gcc -Wall -std=c99 -O3 -ffast-math -I"%SDL2INC%" -I"%SDL2INC%\SDL2" -c seg003.c -o seg003.o
if errorlevel 1 goto error

gcc -Wall -std=c99 -O3 -ffast-math -I"%SDL2INC%" -I"%SDL2INC%\SDL2" -c seg004.c -o seg004.o
if errorlevel 1 goto error

gcc -Wall -std=c99 -O3 -ffast-math -I"%SDL2INC%" -I"%SDL2INC%\SDL2" -c seg005.c -o seg005.o
if errorlevel 1 goto error

gcc -Wall -std=c99 -O3 -ffast-math -I"%SDL2INC%" -I"%SDL2INC%\SDL2" -c seg006.c -o seg006.o
if errorlevel 1 goto error

gcc -Wall -std=c99 -O3 -ffast-math -I"%SDL2INC%" -I"%SDL2INC%\SDL2" -c seg007.c -o seg007.o
if errorlevel 1 goto error

gcc -Wall -std=c99 -O3 -ffast-math -I"%SDL2INC%" -I"%SDL2INC%\SDL2" -c seg008.c -o seg008.o
if errorlevel 1 goto error

gcc -Wall -std=c99 -O3 -ffast-math -I"%SDL2INC%" -I"%SDL2INC%\SDL2" -c seg009.c -o seg009.o
if errorlevel 1 goto error

gcc -Wall -std=c99 -O3 -ffast-math -I"%SDL2INC%" -c seqtbl.c -o seqtbl.o
if errorlevel 1 goto error

gcc -Wall -std=c99 -O3 -ffast-math -I"%SDL2INC%" -c replay.c -o replay.o
if errorlevel 1 goto error

gcc -Wall -std=c99 -O3 -ffast-math -I"%SDL2INC%" -c options.c -o options.o
if errorlevel 1 goto error

gcc -Wall -std=c99 -O3 -ffast-math -I"%SDL2INC%" -c lighting.c -o lighting.o
if errorlevel 1 goto error

gcc -Wall -std=c99 -O3 -ffast-math -I"%SDL2INC%" -c screenshot.c -o screenshot.o
if errorlevel 1 goto error

gcc -Wall -std=c99 -O3 -ffast-math -I"%SDL2INC%" -I"%SDL2INC%\SDL2" -c menu.c -o menu.o
if errorlevel 1 goto error

gcc -Wall -std=c99 -O3 -ffast-math -I"%SDL2INC%" -c midi.c -o midi.o
if errorlevel 1 goto error

gcc -Wall -std=c99 -O3 -ffast-math -I"%SDL2INC%" -c opl3.c -o opl3.o
if errorlevel 1 goto error

gcc -Wall -std=c99 -O3 -ffast-math -I"%SDL2INC%" -c stb_vorbis.c -o stb_vorbis.o
if errorlevel 1 goto error

echo.
echo Linking...
gcc -mwindows main.o data.o seg000.o seg001.o seg002.o seg003.o seg004.o seg005.o seg006.o seg007.o seg008.o seg009.o seqtbl.o replay.o options.o lighting.o screenshot.o menu.o midi.o opl3.o stb_vorbis.o -L"%SDL2LIB%" -lSDL2main -lSDL2 -lSDL2_image -lm -o ..\prince.exe
if errorlevel 1 goto error

echo.
echo Build successful! prince.exe created in parent directory.
goto end

:error
echo.
echo Build failed!
exit /b 1

:end
endlocal

