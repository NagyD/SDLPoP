#!/bin/bash

set -euo pipefail


###############################################################################
# SDLPoP macOS Build Script
#
# Builds:
#
#   - Universal x86_64 + arm64 application bundle
#   - Bundles SDL2.framework
#   - Bundles SDL2_image
#   - Removes extended attributes
#   - Optionally signs with a Developer ID identity
#   - Creates a notarization-ready ZIP
#
# Signing is optional.
#
# This script does NOT submit anything to Apple's Notary Service.
#
# Usage:
#
#   ./macos-build.sh /path/to/SDL-libraries
#
# Or:
#
#   SDL2_ROOT=/path/to/SDL-libraries ./macos-build.sh
#
# Optional signing:
#
#   SIGNING_IDENTITY="Developer ID Application: Your Name (TEAMID)" \
#   ./macos-build.sh /path/to/SDL-libraries
###############################################################################


###############################################################################
# CONFIGURATION
###############################################################################

PROJECT_DIR="$(cd "$(dirname "$0")" && pwd)"

BUILD_DIR="$PROJECT_DIR/build"

APP_BUNDLE="$PROJECT_DIR/prince.app"

ZIP_FILE="$PROJECT_DIR/prince.zip"

MACOS_MIN_VERSION="11.0"

ARCHITECTURES="x86_64;arm64"

SDL2_IMAGE_NAME="libSDL2_image-2.0.0.900.0.dylib"


###############################################################################
# SDL2 ROOT CONFIGURATION
###############################################################################

# SDL2_ROOT can be supplied either:
#
# 1. As the first command-line argument:
#
#    ./macos-build.sh /path/to/SDL-libraries
#
# 2. Through an environment variable:
#
#    SDL2_ROOT=/path/to/SDL-libraries ./macos-build.sh

SDL2_ROOT="${SDL2_ROOT:-${1:-}}"


###############################################################################
# OPTIONAL SIGNING CONFIGURATION
###############################################################################

# SIGNING_IDENTITY can be supplied through an environment variable.
#
# Example:
#
# SIGNING_IDENTITY="Developer ID Application: Your Name (TEAMID)" \
# ./macos-build.sh /path/to/SDL-libraries

SIGNING_IDENTITY="${SIGNING_IDENTITY:-}"


###############################################################################
# HELPER FUNCTIONS
###############################################################################

header() {

    echo
    echo "============================================================"
    echo "$1"
    echo "============================================================"
    echo

}


fail() {

    echo
    echo "ERROR: $1"
    echo

    exit 1

}


###############################################################################
# START
###############################################################################

header "SDLPoP macOS BUILD STARTING"

echo "Project directory:"
echo "$PROJECT_DIR"
echo

echo "Build directory:"
echo "$BUILD_DIR"
echo

echo "Application bundle:"
echo "$APP_BUNDLE"
echo

echo "ZIP file:"
echo "$ZIP_FILE"
echo

echo "SDL libraries:"
echo "${SDL2_ROOT:-NOT CONFIGURED}"
echo

echo "Architectures:"
echo "$ARCHITECTURES"
echo

echo "Minimum macOS version:"
echo "$MACOS_MIN_VERSION"
echo

if [ -n "$SIGNING_IDENTITY" ]
then

    echo "Signing:"
    echo "Enabled"

else

    echo "Signing:"
    echo "Disabled"

fi

echo


###############################################################################
# PRE-FLIGHT CHECKS
###############################################################################

header "PRE-FLIGHT CHECKS"


echo "Checking required directories..."


[ -d "$PROJECT_DIR/src" ] \
    || fail "Source directory not found: $PROJECT_DIR/src"


[ -d "$PROJECT_DIR/data" ] \
    || fail "Data directory not found: $PROJECT_DIR/data"


[ -n "$SDL2_ROOT" ] \
    || fail "SDL2_ROOT is not configured.

Usage:

    ./macos-build.sh /path/to/SDL-libraries

or:

    SDL2_ROOT=/path/to/SDL-libraries ./macos-build.sh"


[ -d "$SDL2_ROOT" ] \
    || fail "SDL2 root not found: $SDL2_ROOT"


[ -d "$SDL2_ROOT/SDL2.framework" ] \
    || fail "SDL2.framework not found in: $SDL2_ROOT"


[ -f "$SDL2_ROOT/lib/$SDL2_IMAGE_NAME" ] \
    || fail "$SDL2_IMAGE_NAME not found in: $SDL2_ROOT/lib"


echo
echo "SUCCESS: Required SDL libraries found."


###############################################################################
# CHECK REQUIRED BUILD TOOLS
###############################################################################

header "CHECKING REQUIRED BUILD TOOLS"


for tool in \
    cmake \
    ditto \
    file \
    lipo \
    xattr \
    unzip \
    codesign
do

    command -v "$tool" >/dev/null 2>&1 \
        || fail "Required tool not found: $tool"

done


echo "SUCCESS: Required build tools found."


###############################################################################
# CHECK SIGNING IDENTITY
###############################################################################

if [ -n "$SIGNING_IDENTITY" ]
then

    header "CHECKING SIGNING IDENTITY"

    security find-identity -v -p codesigning | \
        grep -F "$SIGNING_IDENTITY" \
        >/dev/null \
        || fail "Developer ID signing identity not found:

$SIGNING_IDENTITY"

    echo "SUCCESS: Signing identity found."

fi


###############################################################################
# CLEAN PREVIOUS BUILD
###############################################################################

header "CLEANING PREVIOUS BUILD"

echo "Removing old build directory..."
rm -rf "$BUILD_DIR"

echo "Removing old application bundle..."
rm -rf "$APP_BUNDLE"

echo "Removing old ZIP..."
rm -f "$ZIP_FILE"

echo
echo "SUCCESS: Previous build artifacts removed."


###############################################################################
# CMAKE CONFIGURATION
###############################################################################

header "CONFIGURING CMAKE"


cmake \
    -S "$PROJECT_DIR/src" \
    -B "$BUILD_DIR" \
    -DCMAKE_OSX_ARCHITECTURES="$ARCHITECTURES" \
    -DCMAKE_OSX_DEPLOYMENT_TARGET="$MACOS_MIN_VERSION" \
    -DCREATE_BUNDLE=1 \
    -DSDL2_ROOT="$SDL2_ROOT"


###############################################################################
# BUILD APPLICATION
###############################################################################

header "BUILDING SDLPoP"


cmake \
    --build "$BUILD_DIR" \
    --parallel


###############################################################################
# BUILD RESULT CHECK
###############################################################################

header "VERIFYING BUILD OUTPUT"


[ -d "$APP_BUNDLE" ] \
    || fail "Application bundle was not created."


[ -f "$APP_BUNDLE/Contents/MacOS/prince" ] \
    || fail "Main executable not found."


[ -d "$APP_BUNDLE/Contents/Frameworks/SDL2.framework" ] \
    || fail "SDL2.framework not found in application."


[ -f "$APP_BUNDLE/Contents/Frameworks/$SDL2_IMAGE_NAME" ] \
    || fail "SDL2_image dylib not found in application."


echo "SUCCESS: Required application components found."


###############################################################################
# VERIFY UNIVERSAL EXECUTABLE
###############################################################################

header "VERIFYING MAIN EXECUTABLE"


file \
    "$APP_BUNDLE/Contents/MacOS/prince"

echo


lipo \
    -info \
    "$APP_BUNDLE/Contents/MacOS/prince"


###############################################################################
# VERIFY UNIVERSAL ARCHITECTURES
###############################################################################

header "VERIFYING REQUIRED ARCHITECTURES"


EXECUTABLE_ARCHS=$(lipo \
    -archs \
    "$APP_BUNDLE/Contents/MacOS/prince")


echo "Detected architectures:"
echo "$EXECUTABLE_ARCHS"
echo


echo "$EXECUTABLE_ARCHS" | grep -qw "x86_64" \
    || fail "x86_64 architecture missing."


echo "$EXECUTABLE_ARCHS" | grep -qw "arm64" \
    || fail "arm64 architecture missing."


echo "SUCCESS: Universal x86_64 + arm64 executable verified."


###############################################################################
# REMOVE EXTENDED ATTRIBUTES
###############################################################################

header "REMOVING EXTENDED ATTRIBUTES"


echo "Removing recursive extended attributes..."


xattr \
    -cr \
    "$APP_BUNDLE"


echo


echo "Removing extended attributes from files, directories and symlinks..."


while IFS= read -r -d '' item
do

    xattr \
        -s \
        -c \
        "$item" \
        2>/dev/null \
        || true

done < <(

    find \
        "$APP_BUNDLE" \
        \( \
            -type f \
            -o \
            -type d \
            -o \
            -type l \
        \) \
        -print0

)


###############################################################################
# FINAL XATTR CHECK
###############################################################################

header "FINAL XATTR CHECK"


found=0


while IFS= read -r -d '' item
do

    attrs=$(xattr -s "$item" 2>/dev/null || true)

    if [ -n "$attrs" ]
    then

        echo "--- $item ---"
        echo "$attrs"

        found=1

    fi

done < <(

    find \
        "$APP_BUNDLE" \
        \( \
            -type f \
            -o \
            -type d \
            -o \
            -type l \
        \) \
        -print0

)


echo


if [ "$found" -eq 0 ]
then

    echo "SUCCESS: No extended attributes found."

else

    fail "Extended attributes still exist."

fi


###############################################################################
# REMOVE UNWANTED METADATA FILES
###############################################################################

header "REMOVING UNWANTED METADATA FILES"

echo "Removing .DS_Store files..."

find \
    "$APP_BUNDLE" \
    -name ".DS_Store" \
    -type f \
    -delete

echo

echo "Removing AppleDouble files..."

find \
    "$APP_BUNDLE" \
    -name "._*" \
    -type f \
    -delete

echo

echo "Checking for remaining unwanted metadata files..."

UNWANTED_FILES=$(
    find \
        "$APP_BUNDLE" \
        -type f \
        \( \
            -name ".DS_Store" \
            -o \
            -name "._*" \
        \)
)

if [ -n "$UNWANTED_FILES" ]
then
    echo "$UNWANTED_FILES"
    fail "Unwanted metadata files still exist."
fi

echo "SUCCESS: No unwanted metadata files found."


###############################################################################
# OPTIONAL CODE SIGNING
###############################################################################

if [ -n "$SIGNING_IDENTITY" ]
then

    ###############################################################################
    # SIGN SDL2_IMAGE
    ###############################################################################

    header "SIGNING SDL2_IMAGE"


    codesign \
        --force \
        --options runtime \
        --timestamp \
        --sign "$SIGNING_IDENTITY" \
        "$APP_BUNDLE/Contents/Frameworks/$SDL2_IMAGE_NAME"


    ###############################################################################
    # SIGN SDL2 FRAMEWORK
    ###############################################################################

    header "SIGNING SDL2 FRAMEWORK"


    codesign \
        --force \
        --options runtime \
        --timestamp \
        --sign "$SIGNING_IDENTITY" \
        "$APP_BUNDLE/Contents/Frameworks/SDL2.framework"


    ###############################################################################
    # SIGN APPLICATION
    ###############################################################################

    header "SIGNING APPLICATION"


    codesign \
        --force \
        --options runtime \
        --timestamp \
        --sign "$SIGNING_IDENTITY" \
        "$APP_BUNDLE"


    ###############################################################################
    # SIGNATURE VERIFICATION
    ###############################################################################

    header "STRICT CODE SIGNATURE VERIFICATION"


    codesign \
        --verify \
        --deep \
        --strict \
        --verbose=4 \
        "$APP_BUNDLE"


    ###############################################################################
    # DISPLAY FINAL SIGNATURE
    ###############################################################################

    header "FINAL APPLICATION SIGNATURE"


    codesign \
        -dv \
        --verbose=4 \
        "$APP_BUNDLE" \
        2>&1

else

    header "CODE SIGNING SKIPPED"

    echo "No SIGNING_IDENTITY was supplied."
    echo
    echo "The application bundle will not be Developer ID signed."

fi


###############################################################################
# CREATE NOTARIZATION / DISTRIBUTION ZIP
###############################################################################

header "CREATING ZIP"


ditto \
    -c \
    -k \
    --keepParent \
    "$APP_BUNDLE" \
    "$ZIP_FILE"


[ -f "$ZIP_FILE" ] \
    || fail "ZIP file was not created."


echo
echo "SUCCESS: ZIP created:"
echo "$ZIP_FILE"


###############################################################################
# ZIP CONTENT CHECK
###############################################################################

header "ZIP CONTENT CHECK"


echo "===== SUSPICIOUS FILES IN ZIP ====="


SUSPICIOUS_FILES=$(

    unzip -Z1 "$ZIP_FILE" | \
    grep -E \
    '(^|/)\.DS_Store$|(^|/)\._[^/]*$|__MACOSX/|(^|/)\.git/|(^|/)\.svn/|~$|\.bak$|\.tmp$|\.swp$|\.orig$' \
    || true

)


if [ -n "$SUSPICIOUS_FILES" ]
then

    echo "$SUSPICIOUS_FILES"

    fail "Suspicious metadata or temporary files found in ZIP."

else

    echo "SUCCESS: No suspicious metadata or temporary files found."

fi


echo


echo "===== DEVELOPMENT / BUILD ARTIFACTS ====="


BUILD_ARTIFACTS=$(

    unzip -Z1 "$ZIP_FILE" | \
    grep -E \
    '(^|/)(CMakeFiles|CMakeCache\.txt|Makefile|build|Debug|Release)/|\.o$|\.a$|\.dSYM/' \
    || true

)


if [ -n "$BUILD_ARTIFACTS" ]
then

    echo "$BUILD_ARTIFACTS"

    fail "Development or build artifacts found in ZIP."

else

    echo "SUCCESS: No obvious development artifacts found."

fi


###############################################################################
# TOP-LEVEL ZIP CONTENT
###############################################################################

header "VERIFYING ZIP TOP-LEVEL CONTENT"


TOP_LEVEL_CONTENT=$(

    unzip -Z1 "$ZIP_FILE" | \
    awk -F/ '

        NF > 0 {

            print $1

        }

    ' | \
    sort -u

)


echo "$TOP_LEVEL_CONTENT"


if [ "$TOP_LEVEL_CONTENT" != "prince.app" ]
then

    fail "Unexpected top-level content found in ZIP."

fi


echo
echo "SUCCESS: Only prince.app exists at ZIP top level."


###############################################################################
# EXECUTABLES AND FRAMEWORKS
###############################################################################

header "EXECUTABLES AND FRAMEWORKS IN ZIP"


unzip -Z1 "$ZIP_FILE" | \
grep -E \
'(^|/)(MacOS/[^/]+|Frameworks/.*\.(dylib|framework)(/|$))' \
|| true


###############################################################################
# ZIP FILE COUNT
###############################################################################

header "TOTAL ZIP FILE COUNT"


unzip -Z1 "$ZIP_FILE" | \
wc -l


###############################################################################
# ZIP INTEGRITY TEST
###############################################################################

header "ZIP INTEGRITY TEST"


unzip \
    -t \
    "$ZIP_FILE" \
    > /dev/null


echo "SUCCESS: ZIP integrity test passed."


###############################################################################
# FINAL RESULT
###############################################################################

header "BUILD COMPLETE"


echo "SUCCESS: SDLPoP build completed successfully."

echo

echo "Application:"
echo "$APP_BUNDLE"

echo

echo "ZIP:"
echo "$ZIP_FILE"

echo


if [ -n "$SIGNING_IDENTITY" ]
then

    echo "Signing:"
    echo "SUCCESS: Application was signed."

    echo
    echo "The ZIP has NOT been submitted to Apple."


else

    echo "Signing:"
    echo "SKIPPED"

    echo
    echo "The ZIP is suitable for testing but must be signed"
    echo "before Developer ID distribution or notarization."

fi

###############################################################################
# NEXT STEPS (OPTIONAL)
###############################################################################

echo
echo "============================================================"
echo "NEXT STEPS (OPTIONAL)"
echo "============================================================"
echo

echo "The application has been built successfully."
echo

if [ -n "$SIGNING_IDENTITY" ]; then
    echo "The application has been signed with:"
    echo "$SIGNING_IDENTITY"
    echo
fi

echo "The generated ZIP is ready for submission to Apple Notary Service."
echo

echo "1. Submit the ZIP for notarization:"
echo
echo "   xcrun notarytool submit prince.zip \\"
echo "       --keychain-profile \"YOUR-NOTARIZATION-PROFILE\" \\"
echo "       --wait"
echo

echo "2. After notarization succeeds, staple the notarization ticket:"
echo
echo "   xcrun stapler staple prince.app"
echo

echo "3. Verify the stapled application:"
echo
echo "   codesign --verify --deep --strict --verbose=4 prince.app"
echo
echo "   spctl -a -vvv prince.app"
echo

echo "4. Create the final distribution ZIP after stapling:"
echo
echo "   rm -f prince.zip"
echo
echo "   ditto -c -k --keepParent prince.app prince.zip"
echo

echo
