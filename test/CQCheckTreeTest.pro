TEMPLATE = app

TARGET = CQCheckTreeTest

DEPENDPATH += .

QT += widgets

#CONFIG += debug

# Input
SOURCES += \
CQCheckTreeTest.cpp \

HEADERS += \
CQCheckTreeTest.h \

DESTDIR     = .
OBJECTS_DIR = .

INCLUDEPATH += \
../include \
.

unix:LIBS += \
-L../lib \
-lCQCheckTree
