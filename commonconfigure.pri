
DESTDIR = ./../../build
MOC_DIR = $$DESTDIR/$$TARGET
RCC_DIR = $$DESTDIR/$$TARGET
UI_DIR = $$DESTDIR/$$TARGET
OBJECTS_DIR = $$DESTDIR/$$TARGET


INCLUDEPATH += $$PWD/src/common \
               $$PWD/src/common/network \
               $$PWD/src/painttyDesktop/misc \
               $$PWD/src/painttyDesktop/paintingTools \
               $$PWD/src/painttyDesktop/paintingTools/brush \
               $$PWD/src/painttyDesktop/widgets \
               $$UI_DIR
