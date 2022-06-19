// Client Bluetooth

#include <iostream>
#include <cstdio>
#include <unistd.h>
#include <sys/socket.h>
#include <bluetooth/bluetooth.h>
#include <bluetooth/rfcomm.h>

// g++ client-bluetooth.cpp -lbluetooth -o client-bluetooth

int main(int argc, char** argv)
{
    struct sockaddr_rc adaptateurLocalBluetooth = {
        0
    }; /* adaptateur local bluetooth */
    struct sockaddr_rc adatateurDistantBluetooth = {
        0
    }; /* le serveur bluetooth */
    int         socketServeur;
    int         retour;
    char        buffer[1024] = { 0 };
    const char* adresseMacDistante =
      "B8:27:EB:B1:1A:12"; //"84:0D:8E:37:84:1E"; /* par exemple un ESP32 */
    uint8_t port = 1;

    // Crée une socket pour se connecter à un serveur
    socketServeur = socket(AF_BLUETOOTH, SOCK_STREAM, BTPROTO_RFCOMM);

    // Prépare les paramètres de connexion
    adatateurDistantBluetooth.rc_family  = AF_BLUETOOTH;
    adatateurDistantBluetooth.rc_channel = (uint8_t)htobs(port);
    str2ba(adresseMacDistante, &adatateurDistantBluetooth.rc_bdaddr);

    // Demande de connexion vers un serveur
    retour = connect(socketServeur,
                     (struct sockaddr*)&adatateurDistantBluetooth,
                     sizeof(adatateurDistantBluetooth));

    if(retour)
    {
        perror("socket");
        return 1;
    }

    // Connecté
    std::cout << "Connecté au serveur : " << adresseMacDistante << std::endl;

    // Dialogue
    // sprintf(buffer, "Hello world !\n");
    sprintf(buffer, "$PIKAWA;P;1;2;\r\n");

    int nbEcrits = write(socketServeur, buffer, strlen(buffer));
    switch(nbEcrits)
    {
        case -1: /* une erreur ! */
            perror("write");
            close(socketServeur);
            return 2;
        case 0: /* la socket est fermée */
            std::cerr << "La socket a été fermée par le serveur !\n";
            close(socketServeur);
            return 0;
        default: /* envoi de n octets */
            std::cout << "Message " << buffer << " envoyé avec succès"
                      << std::endl;
    }

    int nbLus = read(socketServeur, buffer, sizeof(buffer));
    switch(nbLus)
    {
        case -1: /* une erreur ! */
            perror("read");
            close(socketServeur);
            return 3;
        case 0: /* la socket est fermée */
            std::cerr << "La socket a été fermée par le serveur !\n";
            close(socketServeur);
            return 0;
        default: /* réception de n octets */
            std::cout << "Message " << buffer << " reçu avec succès"
                      << std::endl;
    }

    // Ferme la socket
    close(socketServeur);

    return 0;
}
