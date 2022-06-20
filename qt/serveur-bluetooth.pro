QT       += core gui widgets bluetooth

CONFIG += c++11

SOURCES += serveur-bluetooth.cpp
HEADERS += 

CONFIG(release, debug|release):DEFINES+=QT_NO_DEBUG_OUTPUT
