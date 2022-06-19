// Serveur Bluetooth

#include <iostream>
#include <cstdio>
#include <unistd.h>
#include <sys/socket.h>
#include <bluetooth/bluetooth.h>
#include <bluetooth/rfcomm.h>
#include <bluetooth/sdp.h>
#include <bluetooth/sco.h>
#include <bluetooth/sdp_lib.h>

// g++ serveur-bluetooth.cpp -lbluetooth -o serveur-bluetooth

sdp_session_t* register_service(uint8_t rfcomm_channel);

int main(int argc, char** argv)
{
    struct sockaddr_rc adaptateurLocalBluetooth = {
        0
    }; /* adaptateur local bluetooth */
    struct sockaddr_rc adatateurDistantBluetooth = {
        0
    }; /* le client bluetooth */
    int         socketEcoute, socketClient;
    socklen_t   longueurAdresseSocket = sizeof(adatateurDistantBluetooth);
    char        buffer[1024]          = { 0 };
    const char* adresseMacLocale = "00:1A:7D:DA:71:13"; //"00:1a:7d:da:71:13";
    bdaddr_t    my_bdaddr_any    = { 0, 0, 0, 0, 0, 0 };
    char        adresseMacClient[24] = { 0 };

    int            port    = 1;
    sdp_session_t* session = register_service(port);

    std::cout << "Adresse MAC serveur : " << adresseMacLocale << std::endl;

    // Crée une socket pour accepter les demandes de connexion
    socketEcoute = socket(AF_BLUETOOTH, SOCK_STREAM, BTPROTO_RFCOMM);

    if(socketEcoute < 0)
    {
        perror("socket");
        return 1;
    }

    // Lie la socket à l'adaptateur local
    adaptateurLocalBluetooth.rc_family = AF_BLUETOOTH;
    // bacpy(&adaptateurLocalBluetooth.rc_bdaddr, &my_bdaddr_any); /* adaptateur
    // local bluetooth */
    //  ou :
    str2ba(adresseMacLocale, &adaptateurLocalBluetooth.rc_bdaddr);
    adaptateurLocalBluetooth.rc_channel =
      htobs((uint8_t)port); /* i.e. numéro de port */
    if(bind(socketEcoute,
            (struct sockaddr*)&adaptateurLocalBluetooth,
            sizeof(adaptateurLocalBluetooth)) == -1)
    {
        perror("bind");
        return 2;
    }

    // Prépare la socket à écouter les demandes de connexion (appel non
    // bloquant)
    if(listen(socketEcoute, 1) == -1)
    {
        perror("listen");
        return 3;
    }

    // Attente de connexion d'un client (appel bloquant)
    std::cout << "Attente de demande connexion ..." << std::endl;
    socketClient = accept(socketEcoute,
                          (struct sockaddr*)&adatateurDistantBluetooth,
                          &longueurAdresseSocket);

    // Connecté
    ba2str(&adatateurDistantBluetooth.rc_bdaddr, adresseMacClient);
    std::cout << "Adresse MAC client : " << adresseMacClient << std::endl;

    // Dialogue
    // int nbLus = read(socketClient, buffer, sizeof(buffer));
    // ou :
    int nbLus = recv(socketClient, buffer, sizeof(buffer), 0);
    switch(nbLus)
    {
        case -1: /* une erreur ! */
            perror("read");
            close(socketClient);
            close(socketEcoute);
            return 4;
        case 0: /* la socket est fermée */
            std::cerr << "La socket a été fermée par le client !\n";
            close(socketClient);
            close(socketEcoute);
            return 0;
        default: /* réception de n octets */
            std::cout << "Message " << buffer << " reçu avec succès"
                      << std::endl;
    }

    sprintf(buffer, "ok\n");
    // int nbEcrits = write(socketClient, buffer, strlen(buffer));
    //  ou :
    int nbEcrits = send(socketClient, buffer, strlen(buffer), 0);
    switch(nbEcrits)
    {
        case -1: /* une erreur ! */
            perror("write");
            close(socketClient);
            close(socketEcoute);
            return 5;
        case 0: /* la socket est fermée */
            std::cerr << "La socket a été fermée par le client !\n";
            close(socketClient);
            close(socketEcoute);
            return 0;
        default: /* envoi de n octets */
            std::cout << "Message " << buffer << " envoyé avec succès"
                      << std::endl;
    }

    // Ferme les sockets
    close(socketClient);
    close(socketEcoute);
    sdp_close(session);

    return 0;
}

sdp_session_t* register_service(uint8_t rfcomm_channel)
{
    // Adapted from http://www.btessentials.com/examples/bluez/sdp-register.c
    uint32_t    svc_uuid_int[] = { 0x00001101,
                                   0x00001000,
                                   0x80000080,
                                   0x5F9B34FB };
    const char* service_name   = "SERIAL_PORT";
    const char* svc_dsc        = "SERIAL_PORT";
    const char* service_prov   = "SERIAL_PORT";

    uuid_t      root_uuid, l2cap_uuid, rfcomm_uuid, svc_uuid, svc_class_uuid;
    sdp_list_t *l2cap_list = 0, *rfcomm_list = 0, *root_list = 0,
               *proto_list = 0, *access_proto_list = 0, *svc_class_list = 0,
               *profile_list   = 0;
    sdp_data_t*        channel = 0;
    sdp_profile_desc_t profile;
    sdp_record_t       record  = { 0 };
    sdp_session_t*     session = NULL;

    // set the general service ID
    sdp_uuid128_create(&svc_uuid, &svc_uuid_int);
    sdp_set_service_id(&record, svc_uuid);

    char str[256] = "";
    sdp_uuid2strn(&svc_uuid, str, 256);
    printf("Enregistrement UUID %s : ", str);

    // set the service class
    sdp_uuid16_create(&svc_class_uuid, SERIAL_PORT_SVCLASS_ID);
    svc_class_list = sdp_list_append(0, &svc_class_uuid);
    sdp_set_service_classes(&record, svc_class_list);

    // set the Bluetooth profile information
    sdp_uuid16_create(&profile.uuid, SERIAL_PORT_PROFILE_ID);
    profile.version = 0x0100;
    profile_list    = sdp_list_append(0, &profile);
    sdp_set_profile_descs(&record, profile_list);

    // make the service record publicly browsable
    sdp_uuid16_create(&root_uuid, PUBLIC_BROWSE_GROUP);
    root_list = sdp_list_append(0, &root_uuid);
    sdp_set_browse_groups(&record, root_list);

    // set l2cap information
    sdp_uuid16_create(&l2cap_uuid, L2CAP_UUID);
    l2cap_list = sdp_list_append(0, &l2cap_uuid);
    proto_list = sdp_list_append(0, l2cap_list);

    // register the RFCOMM channel for RFCOMM sockets
    sdp_uuid16_create(&rfcomm_uuid, RFCOMM_UUID);
    channel     = sdp_data_alloc(SDP_UINT8, &rfcomm_channel);
    rfcomm_list = sdp_list_append(0, &rfcomm_uuid);
    sdp_list_append(rfcomm_list, channel);
    sdp_list_append(proto_list, rfcomm_list);

    access_proto_list = sdp_list_append(0, proto_list);
    sdp_set_access_protos(&record, access_proto_list);

    // set the name, provider, and description
    sdp_set_info_attr(&record, service_name, service_prov, svc_dsc);

    // connect to the local SDP server, register the service record,
    // and disconnect
    // session = sdp_connect(BDADDR_ANY, BDADDR_LOCAL, SDP_RETRY_IF_BUSY);
    bdaddr_t my_bdaddr_any   = { 0, 0, 0, 0, 0, 0 };
    bdaddr_t my_bdaddr_local = { 0, 0, 0, 0xff, 0xff, 0xff };
    session = sdp_connect(&my_bdaddr_any, &my_bdaddr_local, SDP_RETRY_IF_BUSY);
    if(session != NULL)
    {
        sdp_record_register(session, &record, 0);
        printf("ok\n");
    }
    else
    {
        printf("erreur !\n");
    }

    // cleanup
    sdp_data_free(channel);
    sdp_list_free(l2cap_list, 0);
    sdp_list_free(rfcomm_list, 0);
    sdp_list_free(root_list, 0);
    sdp_list_free(access_proto_list, 0);
    sdp_list_free(svc_class_list, 0);
    sdp_list_free(profile_list, 0);

    return session;
}
