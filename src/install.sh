#! /bin/bash

cd ..
# Since we don't copy the executable or the data folder anywhere, the desktop file has to be updated to contain the actual paths.
sed -e 's|$ROOT|'"$PWD"'|' src/SDLPoP.desktop.template > src/SDLPoP.desktop
cp src/SDLPoP.desktop ~/.local/share/applications/

# Associate replay files (*.p1r) with SDLPoP.
xdg-mime install src/user-extension-p1r.xml
xdg-icon-resource install --context mimetypes --size 32 data/icon.png application-x-extension-p1r
xdg-mime default ~/.local/share/applications/SDLPoP.desktop application/x-extension-p1r
