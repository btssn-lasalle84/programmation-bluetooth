#include <QApplication>
#include "serveur-bluetooth.h"

int main(int argc, char* argv[])
{
    QApplication a(argc, argv);
    ServeurBluetooth s;

    return a.exec();
}
