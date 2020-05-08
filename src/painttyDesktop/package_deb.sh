#!/bin/bash

#Package in /tmp in WSL envioronment.

VER=$(cat deb_package/DEBIAN/control | grep "Version" | awk '{print $2}')

ARCH=$(dpkg -l apt | sed -n '6p' | awk '{print $4}')

QTVER=$(qmake -v | sed -n '2p' | awk '{print $4}')

COMMIT=$(git log | head -1 | awk '{print $2}' | cut -c1-7)

rm -rf /tmp/deb_package

cp -r deb_package /tmp

cp ./iconset/painttyWidget.ico /tmp/deb_package/usr/share/pixmap/painttyWidget.ico

INSTALLSIZE=$(du -sk /tmp/deb_package/usr | awk '{print $1}')

sed -i "s/ARCH/${ARCH}/g" /tmp/deb_package/DEBIAN/control 

sed -i "s/QTVER/${QTVER}/g" /tmp/deb_package/DEBIAN/control 

sed -i "/Version/ s/$/-${COMMIT}/" /tmp/deb_package/DEBIAN/control
  
sed -i "4 a Installed-Size: ${INSTALLSIZE}" /tmp/deb_package/DEBIAN/control

cat /tmp/deb_package/DEBIAN/control

chmod -R 0755 /tmp/deb_package

dpkg-deb --build /tmp/deb_package ../../build/painttyDesktop/mrpaint_${VER}_${ARCH}_${COMMIT}.deb

rm -rf /tmp/deb_package

exit 0;