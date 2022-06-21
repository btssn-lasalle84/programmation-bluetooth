# qmake CONFIG+=debug
# qmake CONFIG+=release

QT          += core gui widgets bluetooth
CONFIG      += c++11

TEMPLATE     = app
TARGET       = client-bluetooth.out
DEPENDPATH  += .
INCLUDEPATH += .

SOURCES     += client-bluetooth.cpp test-client-bluetooth.cpp
HEADERS     += client-bluetooth.h

CONFIG(release, debug|release):DEFINES+=QT_NO_DEBUG_OUTPUT
