#include <QApplication>
#include <iostream>
#include "client-bluetooth.h"

int main(int argc, char* argv[])
{
    QApplication    a(argc, argv);
    char*           adresseMacDistante = nullptr;
    ClientBluetooth clientBluetooth;

    if(argc == 2)
    {
        adresseMacDistante = ::new char[strlen(argv[1]) + 1];
        strcpy(adresseMacDistante, argv[1]);
    }
    else
    {
        std::cout << "Usage : " << argv[0] << " xx:xx:xx:xx:xx:xx" << std::endl;
        return 1;
    }

    // Exemple 1 : recherche par le nom de l'appareil
    clientBluetooth.setPrefixeNomAppareil("raspberrypi");
    // clientBluetooth.demarrerRecherche();

    // Exemple 2 : recherche par le service
    clientBluetooth.setPrefixeNomAppareil("raspberrypi");
    clientBluetooth.setServiceNom("SERVEUR_SERIAL_PORT");
    // clientBluetooth.demarrerRechercheService();

    // Exemple 3 : connexion directe avec l'adresse
    if(adresseMacDistante != nullptr)
        clientBluetooth.connecter(adresseMacDistante);

    return a.exec();
}
