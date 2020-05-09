#!/bin/bash

VER=$(printf "%.2f" `echo "scale=2; $(cat ../common/common.h | grep CLIENT_VER | awk '{print $6}'| head -c-2 )/100" | bc`)

ARCH=$(dpkg -l apt | grep 'apt' | awk '{print $4}')

QTVER=$(qmake -v | grep "Qt version" | awk '{print $4}')

COMMIT=$(git log | head -1 | awk '{print $2}' | cut -c1-7)

DIST=$(lsb_release -sc)

#Package in /tmp in WSL envioronment.

rm -rf /tmp/deb_package_temp

cp -r deb_package /tmp/deb_package_temp

cp ./iconset/painttyWidget.ico /tmp/deb_package_temp/usr/share/pixmap/painttyWidget.ico

INSTALLSIZE=$(du -sk /tmp/deb_package_temp/usr | awk '{print $1}')

sed -i "s/ARCH/${ARCH}/g" /tmp/deb_package_temp/DEBIAN/control 

sed -i "s/QTVER/${QTVER}/g" /tmp/deb_package_temp/DEBIAN/control 

sed -i "1 a Version: ${VER}-${COMMIT}-${DIST}" /tmp/deb_package_temp/DEBIAN/control
  
sed -i "4 a Installed-Size: ${INSTALLSIZE}" /tmp/deb_package_temp/DEBIAN/control

cat /tmp/deb_package_temp/DEBIAN/control

chmod -R 0755 /tmp/deb_package_temp

dpkg-deb --build /tmp/deb_package_temp ../../build/painttyDesktop/mrpaint_${VER}_${COMMIT}_${DIST}_${ARCH}.deb

rm -rf /tmp/deb_package_temp

exit 0;