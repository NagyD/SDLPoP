#! /bin/bash

rm ~/.local/share/applications/SDLPoP.desktop

# Deassociate replay files (*.p1r) from SDLPoP.
xdg-mime uninstall user-extension-p1r.xml
xdg-icon-resource uninstall --context mimetypes --size 32 application-x-extension-p1r
