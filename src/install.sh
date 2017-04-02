#! /bin/bash

cd ..
# Since we don't copy the executable or the data folder anywhere, the desktop file has to be updated to contain the actual paths.
sed -e 's|$ROOT|'"$PWD"'|' src/SDLPoP.desktop.template > src/SDLPoP.desktop
cp src/SDLPoP.desktop /usr/share/applications/

