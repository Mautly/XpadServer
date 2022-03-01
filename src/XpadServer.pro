#-------------------------------------------------
#
# Project created by QtCreator 2014-05-01T19:27:40
#
#-------------------------------------------------

QT       += core network
QT       -= gui

TARGET = XpadServer
CONFIG   += console
CONFIG   -= app_bundle

TEMPLATE = app


SOURCES += main.cpp \
    image.cpp \
    imageprocessingthread.cpp \
    detector.cpp \
    mainthread.cpp \
    server.cpp

HEADERS += \
    main.h \
    image.h \
    imageprocessingthread.h \
    detector.h \
    mainthread.h \
    server.h \
    servertype.h

#unix: LIBS += -L$$PWD/../../../imxpad_lib/xpci_lib/ -lxpci_lib -lplda_lib
#unix:INCLUDEPATH += $$PWD/../../../imxpad_lib/xpci_lib
#unix:DEPENDPATH += $$PWD/../../../imxpad_lib/xpci_lib/

unix: LIBS += -L$$PWD/../../XpadUSBLibrary/Release -lXpadUSBLibrary
unix:INCLUDEPATH += $$PWD/../../XpadUSBLibrary/src
unix:DEPENDPATH += $$PWD/../../XpadUSBLibrary/src

macx: LIBS += -L$$PWD/../../XpadUSBLibrary/Release -lXpadUSBLibrary
macx:INCLUDEPATH += $$PWD/../../XpadUSBLibrary/src
macx:DEPENDPATH += $$PWD/../../XpadUSBLibrary/src

win32: LIBS += "$$PWD/../../../DRIVER/USB/xpad_usb/release/xpad_usb.lib"
win32: INCLUDEPATH += "$$PWD/../../../DRIVER/USB/xpad_usb/src"
win32: DEPENDPATH += "$$PWD/../../../DRIVER/USB/xpad_usb/src"

macx:  ICON = server_logo.icns
win32: RC_FILE = server_logo.rc

macx: QMAKE_MAC_SDK = macosx10.12
macx: QMAKE_MACOSX_DEPLOYMENT_TARGET = 10.7
