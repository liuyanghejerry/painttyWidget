#-------------------------------------------------
#
# Project created by QtCreator 2012-05-26T18:10:10
#
#-------------------------------------------------

CONFIG += ordered

TEMPLATE = subdirs

SUBDIRS = src/painttyDesktop

!mac {
    SUBDIRS += src/painttyUpdater
}
