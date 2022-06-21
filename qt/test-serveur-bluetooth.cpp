#include <QApplication>
#include "serveur-bluetooth.h"

int main(int argc, char* argv[])
{
    QApplication     a(argc, argv);
    ServeurBluetooth serveurBluetooth;

    serveurBluetooth.demarrer();

    return a.exec();
}
