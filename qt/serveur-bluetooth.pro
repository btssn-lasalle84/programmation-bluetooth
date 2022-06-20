QT          += core gui widgets bluetooth
CONFIG      += c++11

TEMPLATE     = app
TARGET       = serveur-bluetooth.out
DEPENDPATH  += .
INCLUDEPATH += .

SOURCES     += serveur-bluetooth.cpp test-serveur-bluetooth.cpp
HEADERS     += serveur-bluetooth.h

CONFIG(release, debug|release):DEFINES+=QT_NO_DEBUG_OUTPUT
