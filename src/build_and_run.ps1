# Fast incremental build and run script
# Only compiles changed files and launches game once

$env:Path = "C:\msys64\mingw64\bin;" + $env:Path
$SDL2 = "C:\msys64\mingw64"
$SDL2INC = "$SDL2\include\SDL2"
$SDL2LIB = "$SDL2\lib"

$ErrorActionPreference = "Stop"

# Change to script directory (src)
if ($PSScriptRoot) {
    Set-Location $PSScriptRoot
} else {
    # If run from src directory directly
    if (Test-Path "build_and_run.ps1") {
        Set-Location $PWD
    }
}

Write-Host "Building SDLPoP (incremental)..." -ForegroundColor Green

# Compile only changed files (GCC will skip if .o is newer than .c)
$files = @(
    "main.c", "data.c", "seg000.c", "seg001.c", "seg002.c", "seg003.c", 
    "seg004.c", "seg005.c", "seg006.c", "seg007.c", "seg008.c", "seg009.c",
    "seqtbl.c", "replay.c", "options.c", "lighting.c", "screenshot.c",
    "menu.c", "midi.c", "opl3.c", "stb_vorbis.c"
)

$compileFlags = @("-Wall", "-std=c99", "-O3", "-ffast-math", "-I$SDL2INC", "-I$SDL2INC\SDL2")
$objectFiles = @()

foreach ($file in $files) {
    $obj = $file -replace '\.c$', '.o'
    $objectFiles += $obj
    
    # Check if we need to compile (file changed or .o doesn't exist)
    $cFile = Get-Item $file -ErrorAction SilentlyContinue
    $oFile = Get-Item $obj -ErrorAction SilentlyContinue
    
    if (-not $oFile -or $cFile.LastWriteTime -gt $oFile.LastWriteTime) {
        Write-Host "  Compiling $file..." -ForegroundColor Yellow
        & gcc $compileFlags -c $file -o $obj
        if ($LASTEXITCODE -ne 0) {
            Write-Host "Build failed!" -ForegroundColor Red
            exit 1
        }
    }
}

Write-Host "Linking..." -ForegroundColor Green
$linkFlags = @("-mwindows", "-L$SDL2LIB", "-Wl,--whole-archive", "-lSDL2main", "-Wl,--no-whole-archive", "-lSDL2", "-lSDL2_image", "-lm")
& gcc $objectFiles $linkFlags -o ..\prince.exe

if ($LASTEXITCODE -ne 0) {
    Write-Host "Build failed!" -ForegroundColor Red
    exit 1
}

Write-Host "`n=== BUILD SUCCESSFUL ===" -ForegroundColor Green

# Close any existing game instance
Get-Process prince -ErrorAction SilentlyContinue | Stop-Process -Force
Start-Sleep -Milliseconds 200

# Launch game once
Set-Location ..
Write-Host "Running prince.exe...`n" -ForegroundColor Green
Start-Process -FilePath ".\prince.exe" -WorkingDirectory $PWD

