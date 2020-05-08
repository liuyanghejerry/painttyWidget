#!/bin/bash

#Package in /tmp in WSL envioronment.

VER=$(cat deb_package/DEBIAN/control | grep "Version" | awk '{print $2}')

ARCH=$(cat deb_package/DEBIAN/control | grep "Architecture" | awk '{print $2}')

rm -rf /tmp/deb_package

cp -r deb_package /tmp

cp ./iconset/painttyWidget.ico /tmp/deb_package/usr/share/pixmap/painttyWidget.ico

chmod -R 0755 /tmp/deb_package

dpkg-deb --build /tmp/deb_package ../../build/painttyDesktop/mrpaint_${VER}_${ARCH}.deb

rm -rf /tmp/deb_package

exit 0;