// Client Bluetooth

#include <iostream>
#include <cstdio>
#include <unistd.h>
#include <sys/socket.h>
#include <bluetooth/bluetooth.h>
#include <bluetooth/rfcomm.h>

// g++ -o client-bluetooth.out client-bluetooth.cpp -lbluetooth

int main(int argc, char** argv)
{
    /* l'adaptateur local bluetooth */
    // struct sockaddr_rc adaptateurLocalBluetooth = { 0 };
    /* le serveur bluetooth */
    struct sockaddr_rc adatateurDistantBluetooth = { 0 };
    int                socketServeur;
    int                retour;
    char               buffer[1024] = { 0 };
    /*
     * Exemple d'adresse :
     *  - dongle   "00:1A:7D:DA:71:15"
     *  - ESP32    "84:0D:8E:37:84:1E"
     *  - RPI      "B8:27:EB:B1:1A:12"
     */
    // const char* adresseMacDistante = "84:0D:8E:37:84:1E";
    char*   adresseMacDistante;
    uint8_t port = 1;

    if(argc == 2)
    {
        adresseMacDistante = new char[strlen(argv[1]) + 1];
        strcpy(adresseMacDistante, argv[1]);
    }
    else
    {
        std::cout << "Usage : " << argv[0] << " xx:xx:xx:xx:xx:xx" << std::endl;
        return 1;
    }

    // Crée une socket pour se connecter à un serveur
    socketServeur = socket(AF_BLUETOOTH, SOCK_STREAM, BTPROTO_RFCOMM);
    if(socketServeur == -1)
    {
        perror("socket");
        return 2;
    }

    // Prépare les paramètres de connexion
    adatateurDistantBluetooth.rc_family  = AF_BLUETOOTH;
    adatateurDistantBluetooth.rc_channel = (uint8_t)htobs(port);
    str2ba(adresseMacDistante, &adatateurDistantBluetooth.rc_bdaddr);

    // Demande de connexion vers un serveur
    std::cout << "Connexion au serveur : " << adresseMacDistante << " ..."
              << std::endl;
    retour = connect(socketServeur,
                     (struct sockaddr*)&adatateurDistantBluetooth,
                     sizeof(adatateurDistantBluetooth));
    if(retour)
    {
        std::cerr << "Impossible de se connecter au serveur : "
                  << adresseMacDistante << std::endl;
        perror("connect");
        return 3;
    }

    // Connecté
    std::cout << "Connecté au serveur : " << adresseMacDistante << std::endl;

    // Dialogue
    sprintf(buffer, "Hello world !\n");
    // int nbEcrits = write(socketServeur, buffer, strlen(buffer));
    //   ou :
    int nbEcrits = send(socketServeur, buffer, strlen(buffer), 0);
    switch(nbEcrits)
    {
        case -1: /* une erreur ! */
            perror("write/send");
            close(socketServeur);
            return 4;
        case 0: /* la socket est fermée */
            std::cerr << "La socket a été fermée par le serveur !\n";
            close(socketServeur);
            return 5;
        default: /* envoi de n octets */
            std::cout << "Message " << buffer << " envoyé avec succès"
                      << std::endl;
    }

    // int nbLus = read(socketServeur, buffer, sizeof(buffer));
    //  ou :
    int nbLus = recv(socketServeur, buffer, sizeof(buffer), 0);
    switch(nbLus)
    {
        case -1: /* une erreur ! */
            perror("read/recv");
            close(socketServeur);
            return 6;
        case 0: /* la socket est fermée */
            std::cerr << "La socket a été fermée par le serveur !\n";
            close(socketServeur);
            return 7;
        default: /* réception de n octets */
            buffer[nbLus] = '\0';
            std::cout << "Message " << buffer << " reçu avec succès"
                      << std::endl;
    }

    // Ferme la socket
    close(socketServeur);

    return 0;
}
