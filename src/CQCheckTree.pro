TEMPLATE = lib

TARGET = CQCheckTree

DEPENDPATH += .

CONFIG += staticlib

# Input
HEADERS += \
../include/CQCheckTree.h \

SOURCES += \
CQCheckTree.cpp \

OBJECTS_DIR = ../obj

DESTDIR = ../lib

INCLUDEPATH += \
. \
../include \
