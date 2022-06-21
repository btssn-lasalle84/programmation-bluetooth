# Programmation Bluetooth

Ce document fournit quelques connaissances de base pour programmer une communication Bluetooth sous GNU/Linux.

- [Programmation Bluetooth](#programmation-bluetooth)
  - [Bluez](#bluez)
  - [Pré-requis](#pré-requis)
  - [Mise en oeuvre](#mise-en-oeuvre)
  - [Appairer un appareil](#appairer-un-appareil)
  - [Notions de Bluetooth](#notions-de-bluetooth)
  - [Notions de socket](#notions-de-socket)
  - [Notions de client/serveur](#notions-de-clientserveur)
  - [Créer un(e) socket](#créer-une-socket)
  - [L'adressage socket](#ladressage-socket)
    - [Côté serveur](#côté-serveur)
    - [Côté client](#côté-client)
  - [Exemples client/serveur](#exemples-clientserveur)
    - [C/C++](#cc)
    - [Qt](#qt)
  - [Voir aussi](#voir-aussi)

## Bluez

**BlueZ** est un logiciel qui met en œuvre la technologie sans fil Bluetooth sur le système d'exploitation GNU/Linux. BlueZ est maintenant la mise en œuvre Bluetooth de référence pour GNU/Linux et a été intégré au noyau Linux.

## Pré-requis

Les paquetages sous Ubuntu 20.04 :

```sh
$ dpkg -l | grep -iE "bluez|bluetooth"
ii  bluez                                         5.53-0ubuntu3.6                                     amd64        Bluetooth tools and daemons
ii  bluez-cups                                    5.53-0ubuntu3.6                                     amd64        Bluetooth printer driver for CUPS
ii  bluez-hcidump                                 5.53-0ubuntu3.6                                     amd64        Analyses Bluetooth HCI packets
ii  bluez-obexd                                   5.53-0ubuntu3.6                                     amd64        bluez obex daemon
ii  gir1.2-gnomebluetooth-1.0:amd64               3.34.3-0ubuntu1                                     amd64        Introspection data for GnomeBluetooth
ii  gnome-bluetooth                               3.34.3-0ubuntu1                                     amd64        GNOME Bluetooth tools
ii  libbluetooth-dev                              5.53-0ubuntu3.6                                     amd64        Development files for using the BlueZ Linux Bluetooth library
ii  libbluetooth3:amd64                           5.53-0ubuntu3.6                                     amd64        Library to use the BlueZ Linux Bluetooth stack
ii  libgnome-bluetooth13:amd64                    3.34.3-0ubuntu1                                     amd64        GNOME Bluetooth tools - support library
ii  libkf5bluezqt-data                            5.68.0-0ubuntu1                                     all          data files for bluez-qt
ii  libkf5bluezqt6:amd64                          5.68.0-0ubuntu1                                     amd64        Qt wrapper for bluez
ii  libqt5bluetooth5:amd64                        5.12.8-0ubuntu1                                     amd64        Qt Connectivity Bluetooth module
ii  libqt5bluetooth5-bin                          5.12.8-0ubuntu1                                     amd64        Qt Connectivity Bluetooth module helper binaries
ii  pulseaudio-module-bluetooth                   1:13.99.1-1ubuntu3.13                               amd64        Bluetooth module for PulseAudio sound server
ii  qml-module-org-kde-bluezqt:amd64              5.68.0-0ubuntu1                                     amd64        QML wrapper for bluez
```

Pour développer, il faut :

```sh
$ sudo apt install libbluetooth-dev

$ dpkg -L libbluetooth-dev
```

Liste des outils du paquetage **BlueZ** :

```sh
$ dpkg -L bluez | grep "/usr/bin"
/usr/bin/bccmd
/usr/bin/bluemoon
/usr/bin/bluetoothctl
/usr/bin/btattach
/usr/bin/btmgmt
/usr/bin/btmon
/usr/bin/ciptool
/usr/bin/gatttool
/usr/bin/hciattach
/usr/bin/hcitool
/usr/bin/hex2hcd
/usr/bin/l2ping
/usr/bin/l2test
/usr/bin/obexctl
/usr/bin/rctest
/usr/bin/rfcomm
/usr/bin/sdptool
```

Les plus utiles :

- `bluetoothctl` est un outil de contrôle du Bluetooth
- `hciconfig` est utilisé pour configurer les périphériques Bluetooth
- `hcitool` est utilisé pour configurer les connexions Bluetooth et envoyer une commande spéciale aux périphériques Bluetooth
- `l2ping` envoie une requête d’écho L2CAP à l’adresse MAC Bluetooth BD_ADDR donnée en notation hexadécimale
- `rfcomm` est utilisé pour configurer, maintenir et inspecter la configuration RFCOMM du sous-système Bluetooth dans le noyau Linux
- `sdptool` permet d’effectuer des requêtes SDP sur les périphériques Bluetooth
- `hcidump` lit et affiche les données HCI brutes d’une communication Bluetooth

Pour Qt :

```sh
$ sudo apt-get install libbluetooth-dev qtconnectivity5-dev
```

## Mise en oeuvre

Le service Bluetooth se contrôle de la manière suivante : `systemctl status|start|stop|restart bluetooth.service`

```sh
$ sudo systemctl status bluetooth.service
bluetooth.service - Bluetooth service
     Loaded: loaded (/lib/systemd/system/bluetooth.service; enabled; vendor preset: enabled)
     Active: active (running) since Thu 2022-06-16 06:28:40 CEST; 2 days ago
       Docs: man:bluetoothd(8)
   Main PID: 1914030 (bluetoothd)
     Status: "Running"
      Tasks: 1 (limit: 38262)
     Memory: 8.0M
     CGroup: /system.slice/bluetooth.service
             └─1914030 /usr/lib/bluetooth/bluetoothd

juin 16 06:28:40 sedatech systemd[1]: Starting Bluetooth service...
...
```

Et l'interface Bluetooth (ici un _dongle_ USB) :

```sh
$ rfkill list bluetooth
2: hci0: Bluetooth
	Soft blocked: no
	Hard blocked: no

$ hcitool dev
Devices:
	hci0	00:1A:7D:DA:71:13

$ hciconfig
hci0:	Type: Primary  Bus: USB
	BD Address: 00:1A:7D:DA:71:13  ACL MTU: 310:10  SCO MTU: 64:8
	UP RUNNING PSCAN ISCAN INQUIRY
	RX bytes:204341586 acl:299 sco:0 events:7183413 errors:0
	TX bytes:1061893 acl:200 sco:0 commands:141425 errors:0
```

Si le périphérique est `DOWN`, il faut l'activer : `$ hciconfig hci0 up`

Il est possible d’effectuer une recherche des appareils détectables :

```sh
$ hcitool inq

$ hcitool scan
```

`sdptool` permet également d'obtenir des informations sur les services des périphériques.

> sdptool vs Bluez 5 : https://bbs.archlinux.org/viewtopic.php?id=201672

```sh
$ sudo vim /etc/systemd/system/dbus-org.bluez.service

ExecStart=/usr/lib/bluetooth/bluetoothd --compat

$ sudo systemctl daemon-reload
$ sudo systemctl restart bluetooth

$ sudo chmod 777 /var/run/sdp
```

Il est possible d’effectuer une recherche des services détectables :

```sh
$ sdptool browse local
...

$ sdptool search --bdaddr local SP
Searching for SP on FF:FF:FF:00:00:00 ...
Service Name: SERIAL_PORT
Service Description: SERIAL_PORT
Service Provider: SERIAL_PORT
Service RecHandle: 0x10009
Service Class ID List:
  "Serial Port" (0x1101)
Protocol Descriptor List:
  "L2CAP" (0x0100)
  "RFCOMM" (0x0003)
    Channel: 1
Profile Descriptor List:
  "Serial Port" (0x1101)
    Version: 0x0100

$ sdptool browse [xx:xx:xx:xx:xx:xx]
Browsing 84:0D:8E:37:84:1E ...
Service Name: ESP32SPP
Service RecHandle: 0x10000
Service Class ID List:
  "Serial Port" (0x1101)
Protocol Descriptor List:
  "L2CAP" (0x0100)
  "RFCOMM" (0x0003)
    Channel: 1
Profile Descriptor List:
  "Serial Port" (0x1101)
    Version: 0x0102
```

Les canaux utilisés par `rfcomm` (ici, le `3` et le `12`) :

```sh
$ sudo cat /sys/kernel/debug/bluetooth/rfcomm
00:1a:7d:da:71:13 00:00:00:00:00:00 4 3
00:1a:7d:da:71:13 00:00:00:00:00:00 4 12

$ cat /proc/net/rfcomm
sk               RefCnt Rmem   Wmem   User   Inode  Parent
0000000000000000 2      0      0      0      44004585 0
0000000000000000 2      0      0      0      44004584 0

$ sudo ls -l /proc/*/fd | grep 44004585
lrwx------ 1 root root 64 juin  18 15:51 47 -> socket:[44004585]
```

Voir aussi : [Mise en oeuvre du Bluetooth](http://tvaira.free.fr/projets/activites/activite-bluetooth.html)

## Appairer un appareil

On utilise `bluetoothctl` :

```sh
$ sudo bluetoothctl
[NEW] Controller 00:1A:7D:DA:71:13 Pairable: yes
[bluetooth]# help
Menu main:
Available commands:
-------------------
...
scan                                              Scan Options Submenu
...
list                                              List available controllers
show [ctrl]                                       Controller information
...
devices                                           List available devices
paired-devices                                    List paired devices
...
power <on/off>                                    Set controller power
pairable <on/off>                                 Set controller pairable mode
discoverable <on/off>                             Set controller discoverable mode
...
agent <on/off/capability>                         Enable/disable agent with given capability
...
scan <on/off>                                     Scan for devices
info [dev]                                        Device information
pair [dev]                                        Pair with device
trust [dev]                                       Trust device
...
remove <dev>                                      Remove device
...
quit                                              Quit program
exit                                              Quit program
...
```

Pour cela, on va :

- déclarer un agent
- mettre le contrôleur sous tension
- chercher les appareils à proximité
- si besoin faire confiance à l'appareil
- appairer et connecter un appareil

```sh
[bluetooth]# agent on
Agent registered
[bluetooth]# power on
Changing power on succeeded
[bluetooth]# scan on
Discovery started
[NEW] Device xx:xx:xx:xx:xx:xx nnnnnnnn
...
[bluetooth]# trust xx:xx:xx:xx:xx:xx
...
[bluetooth]# pair xx:xx:xx:xx:xx:xx
...
```

Pour modifier les modes de l'appareil :

- le rendre "connectable" :

```sh
$ sudo hciconfig hci0 pscan
```

- le rendre "découvrable" :

```sh
$ sudo hciconfig hci0 iscan
```

- les deux ("connectable" et "découvrable"):

```sh
$ sudo hciconfig hci0 piscan

$ hciconfig
hci0:	Type: Primary  Bus: UART
	BD Address: B8:27:EB:B1:1A:12  ACL MTU: 1021:8  SCO MTU: 64:1
	UP RUNNING PSCAN ISCAN
	RX bytes:6209 acl:68 sco:0 events:201 errors:0
	TX bytes:9612 acl:68 sco:0 commands:134 errors:0
```

Voir aussi :

```sh
$ dbus-send --system --dest=org.bluez --type=method_call --print-reply  / org.freedesktop.DBus.ObjectManager.GetManagedObjects

$ dbus-send --system --type=method_call --print-reply --dest=org.bluez /org/bluez/hci0 org.freedesktop.DBus.Properties.Get string:org.bluez.Adapter1 string:Discoverable
variant       boolean true
```

Voir aussi : [Mise en oeuvre du Bluetooth](http://tvaira.free.fr/projets/activites/activite-bluetooth.html)

## Notions de Bluetooth

Bluetooth est une norme de communications permettant l’échange bidirectionnel de données à très courte distance en utilisant des ondes radio UHF sur une bande de fréquence de 2,4 GHz.

Une communication Bluetooth s'appuie sur un modèle à couches.

Les couches basses prennent en charge les tâches matérielles comme la couche radio (la couche la plus basse) qui s’occupe de l’émission et de la réception des ondes radio. Elle définit les caractéristiques telles que la bande de fréquence et l’arrangement des canaux, les caractéristiques du transmetteur, de la modulation, du récepteur, etc.

Les adresses physiques des périphériques (équivalentes à l’adresse MAC d’une carte réseau) sont nommées `BD_ADDR` (_Bluetooth Device Address_) et sont codées sur 48 bits. Ces adresses sont gérées par la IEEE Registration Authority.

Les couches logicielles gèrent les liens entre les périphériques « maîtres » et « esclaves » ainsi que les types de liaisons (synchrones ou asynchrones). Elles s'appuient sur la couche `L2CAP` (_Logical Link Control & Adaptation Protocol_) qui fournit les services de multiplexage des protocoles de niveau supérieur et la segmentation et le réassemblage des paquets.

Il existe plusieurs services : `RFCOMM`, `SDP` (_Service Discovery Protocol_) et `OBEX` (_OBject EXchange_).

Le service `RFCOMM` (_Radio frequency communication_) est basé sur les spécifications RS-232, qui émule des liaisons séries. Il peut notamment servir à faire passer une communication IP par Bluetooth. `RFCOMM` est utilisé lorsque le débit des données n’atteint pas plus de 360 kbit/s.

Un profil correspond à une spécification fonctionnelle d’un usage particulier. Les profils peuvent également correspondre à différents types de périphériques. Les profils ont pour but d’assurer une interopérabilité entre tous les appareils Bluetooth. Ils définissent notamment la manière d’implémenter un usage défini et les protocoles spécifiques à utiliser.

Afin d’échanger des données, les appareils doivent être appairés. L’appairage se fait en lançant la découverte à partir d’un appareil et en échangeant un code.

Voir aussi : [Mise en oeuvre du Bluetooth](http://tvaira.free.fr/projets/activites/activite-bluetooth.html)

## Notions de socket

Un(e) socket représente une interface de communication logicielle avec le système d’exploitation qui permet d’exploiter les services d’un protocole réseau et par laquelle une application peut envoyer et recevoir des données.

C’est donc un mécanisme de communication bidirectionelle entre processus (locaux et/ou distants).

Un(e) socket désigne aussi un ensemble normalisé de fonctions de communication (une API).

> Les pages man principales sous Unix/Linux concernant la programmation réseau sont regroupées dans le chapitre 7 : `man 7 socket`

## Notions de client/serveur

Les communications réseaux sont basées sur l'architecture client/serveur.

Les élements à retenir sont :

- le serveur est le processus qui offre un service
- le client est le processeur qui demande un service
- la communication s’initie TOUJOURS à la demande du client
- le serveur se place en attente d'une demande d'un client
- le client doit obligatoirement connaître les paramètres de connexion pour joindre un serveur

> Les communications serveur-serveur ou client-client ne sont donc pas possibles. Il en ressort aussi que l'algorithme d'un processus serveur est différent de celui d'un processus client.

## Créer un(e) socket

Pour dialoguer, chaque processus devra préalablement créer un(e) socket de communication en indiquant :

- le domaine de communication : ceci sélectionne la famille de protocole à employer. Il faut savoir que chaque famille possède son adressage. Par exemple pour les protocoles Internet IPv4, on utilisera le domaine `AF_INET` et `AF_INET6` pour le protocole IPv6. En Bluetooth, on utilise `AF_BLUETOOTH`.
- le type de socket à utiliser pour le dialogue. On a généralement le choix entre : `SOCK_STREAM` (qui correspond à un mode connecté), `SOCK_DGRAM` (qui correspond à un mode non connecté) ou `SOCK_RAW` (qui permet un accès direct aux protocoles). Ici, on sélectionne `SOCK_STREAM`.
- le protocole à utiliser sur la socket. Le numéro de protocole dépend du domaine de communication et du type de la socket. Par exemple pour les protocoles Internet, on a `TCP` pour `SOCK_STREAM` et `UDP` pour `SOCK_DGRAM`). En Bluetooth, on a le choix entre plusieurs protocoles comme `BTPROTO_L2CAP` ou `BTPROTO_RFCOMM`.

En résumé, l'appel qui permet de créer un(e) socket de communication Bluetooth :

```cpp
int s = socket(AF_BLUETOOTH, SOCK_STREAM, BTPROTO_RFCOMM);
```

## L'adressage socket

L’interface socket propose une structure d’adresse générique :

```cpp
struct sockaddr
{
    unsigned short int sa_family; // au choix
    unsigned char sa_data[14]; // en fonction de la famille
};
```

Dans le domaine `AF_BLUETOOTH`, on utilise une structure compatible :

```cpp
/* RFCOMM socket address */
struct sockaddr_rc {
	sa_family_t	rc_family;
	bdaddr_t	rc_bdaddr;
	uint8_t		rc_channel;
};
```

Les adresses physiques Bluetooth sont de type `bdaddr_t` (_Bluetooth Device Address_) et codées sur 48 bits (6 octets) :

```cpp
/* BD Address */
typedef struct {
	uint8_t b[6];
} bdaddr_t;
```

Le numéro de canal (_channel_) fait office de numéro de port. Les protocoles de transport Bluetooth ont été conçus avec beaucoup moins de numéros de port disponibles, ce qui signifie qu'il y a un risque qu'un numéro de port arbitraire (choisi au moment de la conception) soit déjà utilisé (_bind: Address already in use_). `L2CAP` compte environ 15 000 numéros de port non réservés mais `RFCOMM` n'en a que 30.

RFCOMM 	channel 	none 	1-30
L2CAP 	PSM 	odd numbered 1-4095 	odd numbered 4097 - 32765

> Une solution à ce problème est le SDP (_Service Discovery Protocol_). Au lieu de convenir d'un port à utiliser au moment de la conception de l'application, l'approche Bluetooth consiste à attribuer des ports au moment de l'exécution et à suivre un modèle de publication-abonnement.

Exemple pour un client :

```cpp
#include <bluetooth/bluetooth.h> /* pour bdaddr_t */
#include <bluetooth/rfcomm.h> /* pour sockaddr_rc */

const char* adresseMacDistante = "00:1A:7D:DA:71:13"; /* l'adresse du serveur bluetooth */
struct sockaddr_rc adatateurDistantBluetooth = { 0 }; /* le serveur bluetooth */

// Prépare les paramètres de connexion
adatateurDistantBluetooth.rc_family  = AF_BLUETOOTH;
adatateurDistantBluetooth.rc_channel = (uint8_t)1; // numéro de port
str2ba(adresseMacDistante, &adatateurDistantBluetooth.rc_bdaddr);
```

### Côté serveur

Le processus serveur crée une socket appelée généralement socket d'écoute (appel `socket()`). Elle est nécessaire pour "écouter" les demandes de connexions.

Le serveur doit tout d'abord lier (_bind_) sa socket d'écoute à une adresse d'interface de communication locale (appel `bind()`).

Ensuite, le serveur placera sa socket d'écoute en attente de demande de connexion d'un client (appels `listen()` et `accept()`).

Si le serveur accepte la demande de connexion d'un client (appel `accept()`), cela crée une nouvelle socket appelée généralement socket de dialogue. Dans ce cas, le serveur est dit "mono-client" car il peut gérer la connexion et le dialogue avec un seul client.

Si le serveur met en place un traitement spécifique, il pourra continuer à "écouter" des nouvelles demandes de connexion sur sa socket d'écoute et dialoguer avec les sockets des clients. Dans ce cas, le serveur est dit "multi-clients" car il peut gérer la connexion et le dialogue avec plusieurs clients.

Pour implémenter un serveur multi-clients, il y deux approches de base :

- il peut demander l'aide du système d'exploitation pour surveiller l'activité sur les sockets. L'apple système qui permet cela se nomme `select()`.
- il peut mettre en place un traitement multi-tâches avec des processus lourds (appel `fork()`) ou légers (des _threads_ avec l'appel `pthread_create()`).

Pour dialoguer, les processus envoient des donnes (appel `write()`) et/ou recoivent des données (appel `read()`).

Les sockets étant des ressources systèmes, il faut obligatoirement les libérer avec l'appel `close()`.

Exemple de processus serveur mono-client :

```cpp

```

### Côté client

Principe :

- Le processus client crée une socket appelée généralement socket de communication (appel `socket()`).

- Le client doit renseigner une structure d'adresse qui permettra de joindre une socket d'un processus serveur.

- Ensuite, le client fera sa demande de connexion (appel `connect()`).

- Si le serveur accepte sa demande de connexion, les deux processus pourront s'échanger des données avec les appels `read()` et `write()`.

- Les sockets étant des ressources systèmes, il faut obligatoirement les libérer avec l'appel `close()`.

Exemple de processus client :

```cpp
#include <iostream>
#include <cstdio>
#include <unistd.h>
#include <sys/socket.h>
#include <bluetooth/bluetooth.h>
#include <bluetooth/rfcomm.h>

int main(int argc, char** argv)
{
    struct sockaddr_rc adaptateurLocalBluetooth = { 0 }; /* adaptateur local bluetooth */
    struct sockaddr_rc adatateurDistantBluetooth = { 0 }; /* le serveur bluetooth */
    int         socketServeur;
    int         retour;
    char        buffer[1024] = { 0 };
    const char* adresseMacDistante = "84:0D:8E:37:84:1E"; /* par exemple un ESP32 */
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
    sprintf(buffer, "Hello world !\n");

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
```

Test :

```
$ sdptool browse
Inquiring ...
Browsing 84:0D:8E:37:84:1E ...
Service Name: ESP32SPP
Service RecHandle: 0x10000
Service Class ID List:
  "Serial Port" (0x1101)
Protocol Descriptor List:
  "L2CAP" (0x0100)
  "RFCOMM" (0x0003)
    Channel: 1
Profile Descriptor List:
  "Serial Port" (0x1101)
    Version: 0x0102

$ g++ client-bluetooth.cpp -lbluetooth -o client-bluetooth

$ ./client-bluetooth
Connecté au serveur : 84:0D:8E:37:84:1E
Message Hello world !
 envoyé avec succès
...
```

## Exemples client/serveur

### C/C++

Fabriquer les programmes d'exemple en version :

```sh
$ cd client-serveur/
$ make
g++ -std=c++11 -Wall  -c  client-bluetooth.cpp -o client-bluetooth.o
g++ -o client-bluetooth.out client-bluetooth.o -lbluetooth
g++ -std=c++11 -Wall  -c  serveur-bluetooth.cpp -o serveur-bluetooth.o
g++ -o serveur-bluetooth.out serveur-bluetooth.o -lbluetooth
$ ls -l *.out
-rwxrwxr-x 1 tv tv 17896 juin  21 16:20 client-bluetooth.out
-rwxrwxr-x 1 tv tv 27096 juin  21 16:20 serveur-bluetooth.out
```

Tests :

- démarrer le serveur dans un terminal

```sh
$ ./serveur-bluetooth.out
Enregistrement UUID 01110000-0010-0000-8000-0080fb349b5f : ok !
Attente de demande de connexion ...
```

- démarrer maintenant le client dans un autre terminal

```sh
$ ./client-bluetooth.out B8:27:EB:B1:1A:12
Connexion au serveur : B8:27:EB:B1:1A:12 ...
Connecté au serveur : B8:27:EB:B1:1A:12
Message Hello world !
 envoyé avec succès
Message ok
 reçu avec succès
```

- dans le terminal du serveur

```sh
Enregistrement UUID 01110000-0010-0000-8000-0080fb349b5f : ok !
Attente de demande de connexion ...
Adresse MAC client : 00:1A:7D:DA:71:13
Message Hello world !
 reçu avec succès
Message ok
 envoyé avec succès
```

Nettoyage :

```sh
$ make clean
```

### Qt

Pour utiliser l’API Qt Bluetooth dans une application, il faut commencer par ajouter l’option de configuration suivante à le fichier de projet `.pro` :

```
QT += bluetooth
```

Voir : [Mise en oeuvre du Bluetooth avec Qt](http://tvaira.free.fr/projets/activites/activite-bluetooth.html#qt-et-bluetooth)

Fabriquer les programmes d'exemple en version _debug_ :

```sh
$ cd qt
$ qmake serveur-bluetooth.pro CONFIG+=debug
$ make
$ qmake client-bluetooth.pro CONFIG+=debug
$ make
$ ls -l *.out
-rwxrwxr-x 1 tv tv 1285744 juin  21 09:41 client-bluetooth.out
-rwxrwxr-x 1 tv tv 1228032 juin  21 07:06 serveur-bluetooth.out
```

Tests :

- démarrer le serveur dans un terminal

```sh
$ ./serveur-bluetooth.out
ServeurBluetooth::ServeurBluetooth(QObject*)
void ServeurBluetooth::activerBluetooth()
void ServeurBluetooth::demarrer()
```

- démarrer maintenant le client dans un autre terminal

```sh
$ ./client-bluetooth.out B8:27:EB:B1:1A:12
ClientBluetooth::ClientBluetooth(QObject*)
void ClientBluetooth::activerBluetooth()
qt.bluetooth.bluez: Missing CAP_NET_ADMIN permission. Cannot determine whether a found address is of random or public type.
void ClientBluetooth::enregistrerAppareil(QString) QBluetoothSocket(0x56482c9d2080) "B8:27:EB:B1:1A:12"
void ClientBluetooth::connecter(QString) QBluetoothSocket(0x56482c9d2080) "B8:27:EB:B1:1A:12" QBluetoothSocket::UnconnectedState
void ClientBluetooth::lireChangementEtatSocket(QBluetoothSocket::SocketState) QBluetoothSocket(0x56482c9d2080) etat QBluetoothSocket::ConnectingState
void ClientBluetooth::lireChangementEtatSocket(QBluetoothSocket::SocketState) QBluetoothSocket(0x56482c9d2080) etat QBluetoothSocket::ConnectedState
void ClientBluetooth::gererConnexion() QBluetoothSocket(0x56482c9d2080) "B8:27:EB:B1:1A:12" QBluetoothSocket::ConnectedState
void ClientBluetooth::lireDonneesSocket() "raspberrypi" "B8:27:EB:B1:1A:12"
void ClientBluetooth::lireDonneesSocket() "ok\r\n"
^C
```

- dans le terminal du serveur

```sh
ServeurBluetooth::ServeurBluetooth(QObject*)
void ServeurBluetooth::activerBluetooth()
void ServeurBluetooth::demarrer()
void ServeurBluetooth::gererNouveauClient() nouveau client ? QBluetoothSocket(0xf0148) "sedatech" "00:1A:7D:DA:71:13"
void ServeurBluetooth::gererNouveauClient() nouveau client ! QBluetoothSocket(0xf0148) "sedatech" "00:1A:7D:DA:71:13"
void ServeurBluetooth::lireDonneesSocket() "sedatech" "00:1A:7D:DA:71:13"
void ServeurBluetooth::lireDonneesSocket() "Hello world !\n"
qt.bluetooth.bluez: void QBluetoothSocketPrivate::_q_readNotify() 10 error: -1 "Connexion ré-initialisée par le correspondant"
void ServeurBluetooth::lireChangementEtatSocket(QBluetoothSocket::SocketState) QBluetoothSocket(0xf0148) etat QBluetoothSocket::ClosingState
void ServeurBluetooth::lireChangementEtatSocket(QBluetoothSocket::SocketState) QBluetoothSocket(0xf0148) etat QBluetoothSocket::UnconnectedState
void ServeurBluetooth::deconnecterSocket() QBluetoothSocket(0xf0148) "00:1A:7D:DA:71:13" QBluetoothSocket::UnconnectedState
void ServeurBluetooth::deconnecterSocket() "00:1A:7D:DA:71:13" remove 1
```

Nettoyage :

```sh
$ qmake serveur-bluetooth.pro CONFIG+=debug
$ make distclean
rm -f moc_serveur-bluetooth.cpp
rm -f serveur-bluetooth.o test-serveur-bluetooth.o moc_serveur-bluetooth.o
rm -f *~ core *.core
rm -f serveur-bluetooth.out
rm -f .qmake.stash
rm -f Makefile

$ qmake client-bluetooth.pro CONFIG+=debug
$ make distclean
rm -f moc_client-bluetooth.cpp
rm -f client-bluetooth.o test-client-bluetooth.o moc_client-bluetooth.o
rm -f *~ core *.core
rm -f client-bluetooth.out
rm -f .qmake.stash
rm -f Makefile
```

## Voir aussi

- [Bluetooth](http://tvaira.free.fr/projets/activites/activite-bluetooth.html)
- [Bluetooth LE](http://tvaira.free.fr/bts-sn/activites/activite-ble/bluetooth-ble.html)
  - [Bluetooth LE avec Qt](http://tvaira.free.fr/bts-sn/activites/activite-ble/activite-ble-qt.html)
  - [Bluetooth LE avec un ESP32](http://tvaira.free.fr/bts-sn/activites/activite-ble/activite-ble-esp32.html)
- [Bluetooth avec Android](http://tvaira.free.fr/dev/android/android-4.html)

©️ Thierry VAIRA <<thierry.vaira@gmail.com>> - LaSalle Avignon - 2022
