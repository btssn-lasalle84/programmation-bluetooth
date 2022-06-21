#include "client-bluetooth.h"
#include <QMapIterator>
#include <QDebug>

ClientBluetooth::ClientBluetooth(QObject* parent) :
    QObject(parent), discoveryAgentDevice(nullptr),
    discoveryAgentService(nullptr), peripheriqueLocal()
{
    qDebug() << Q_FUNC_INFO;
    activerBluetooth();
    creerAgentsRecherche();
}

ClientBluetooth::~ClientBluetooth()
{
    arreter();
    desactiverBluetooth();
    qDebug() << Q_FUNC_INFO;
}

void ClientBluetooth::activerBluetooth()
{
    if(!this->peripheriqueLocal.isValid())
    {
        qCritical() << Q_FUNC_INFO;
        return;
    }
    else
    {
        qDebug() << Q_FUNC_INFO;
        this->peripheriqueLocal.powerOn();
        this->peripheriqueLocal.setHostMode(
          QBluetoothLocalDevice::HostDiscoverable);
        connect(&peripheriqueLocal,
                SIGNAL(error(QBluetoothLocalDevice::Error)),
                this,
                SLOT(lireErreurDevice(QBluetoothLocalDevice::Error)));
    }
}

void ClientBluetooth::desactiverBluetooth()
{
    qDebug() << Q_FUNC_INFO;
    // this->peripheriqueLocal.setHostMode(QBluetoothLocalDevice::HostPoweredOff);
}

void ClientBluetooth::creerAgentsRecherche()
{
    discoveryAgentDevice = new QBluetoothDeviceDiscoveryAgent(this);
    connect(discoveryAgentDevice,
            SIGNAL(deviceDiscovered(QBluetoothDeviceInfo)),
            this,
            SLOT(decouvrirAppareil(QBluetoothDeviceInfo)));
    discoveryAgentService = new QBluetoothServiceDiscoveryAgent(this);
    connect(discoveryAgentService,
            SIGNAL(serviceDiscovered(QBluetoothServiceInfo)),
            this,
            SLOT(decouvrirService(QBluetoothServiceInfo)));
}

void ClientBluetooth::enregistrerAppareil(const QBluetoothDeviceInfo device)
{
    // nouvel appareil ?
    enregistrerAppareil(device.address().toString());
    peripheriquesDistants.push_back(device);
}

QString ClientBluetooth::recupererAdresseMAC(QBluetoothSocket* socket) const
{
    QMap<QString, QBluetoothSocket*>::const_iterator i = sockets.constBegin();
    while(i != sockets.constEnd())
    {
        if(i.value() == socket)
        {
            return i.key();
        }
        ++i;
    }
    return QString();
}

QString ClientBluetooth::recupererAdresseMAC(QString nom) const
{
    for(int i = 0; i < peripheriquesDistants.size(); ++i)
    {
        if(peripheriquesDistants.at(i).name() == nom)
            return peripheriquesDistants.at(i).address().toString();
    }
    return QString();
}

int ClientBluetooth::recupererIndiceAppareil(QString adresseMAC) const
{
    for(int i = 0; i < peripheriquesDistants.size(); ++i)
    {
        if(peripheriquesDistants.at(i).address().toString() == adresseMAC)
            return i;
    }
    return -1;
}

QList<QBluetoothDeviceInfo> ClientBluetooth::getPeripheriquesDistants() const
{
    return peripheriquesDistants;
}

QString ClientBluetooth::getPrefixeNomAppareil() const
{
    return prefixeNomAppareil;
}

void ClientBluetooth::setPrefixeNomAppareil(QString prefixeNomAppareil)
{
    this->prefixeNomAppareil = prefixeNomAppareil;
}

QString ClientBluetooth::getServiceNom() const
{
    return serviceNom;
}

void ClientBluetooth::setServiceNom(QString serviceNom)
{
    this->serviceNom = serviceNom;
}

void ClientBluetooth::enregistrerAppareil(const QString adresseMAC)
{
    // nouvel appareil ?
    if(!sockets.contains(adresseMAC))
    {
        QBluetoothSocket* socket =
          new QBluetoothSocket(QBluetoothServiceInfo::RfcommProtocol);
        connect(socket, SIGNAL(connected()), this, SLOT(gererConnexion()));
        connect(socket, SIGNAL(disconnected()), this, SLOT(gererConnexion()));
        connect(socket,
                SIGNAL(error(QBluetoothSocket::SocketError)),
                this,
                SLOT(lireErreurSocket(QBluetoothSocket::SocketError)));
        connect(socket,
                SIGNAL(stateChanged(QBluetoothSocket::SocketState)),
                this,
                SLOT(lireChangementEtatSocket(QBluetoothSocket::SocketState)));
        connect(socket, SIGNAL(readyRead()), this, SLOT(lireDonneesSocket()));
        qDebug() << Q_FUNC_INFO << socket << adresseMAC;
        sockets[adresseMAC] = socket;
    }
}

void ClientBluetooth::connecterAppareil(QString nom)
{
    QString adresseMAC = recupererAdresseMAC(nom);
    if(!adresseMAC.isEmpty())
        connecter(adresseMAC);
}

void ClientBluetooth::deconnecterAppareil(QString nom)
{
    QString adresseMAC = recupererAdresseMAC(nom);
    if(!adresseMAC.isEmpty())
        deconnecter(adresseMAC);
}

void ClientBluetooth::connecter(QBluetoothDeviceInfo device)
{
    connecter(device.address().toString());
}

void ClientBluetooth::connecter(QBluetoothAddress adresse)
{
    connecter(adresse.toString());
}

void ClientBluetooth::connecter(QString adresseMAC)
{
    enregistrerAppareil(adresseMAC);
    if(sockets.contains(adresseMAC))
    {
        QBluetoothSocket* socket = sockets.value(adresseMAC);
        qDebug() << Q_FUNC_INFO << socket << adresseMAC << socket->state();
        if(socket->state() == QBluetoothSocket::ConnectingState)
        {
            socket->disconnectFromService();
        }
        if(socket->state() != QBluetoothSocket::ConnectedState)
        {
            if(serviceServeur.isValid())
            {
                QBluetoothUuid uuid =
                  QBluetoothUuid(QBluetoothUuid::SerialPort);
                socket->connectToService(serviceServeur.device().address(),
                                         uuid);
                // socket->connectToService(serviceServeur);
            }
            else
            {
                QBluetoothAddress adresse = QBluetoothAddress(adresseMAC);
                QBluetoothUuid    uuid =
                  QBluetoothUuid(QBluetoothUuid::SerialPort);
                socket->connectToService(adresse, uuid);
            }
        }
    }
}

void ClientBluetooth::deconnecter(QBluetoothDeviceInfo device)
{
    deconnecter(device.address().toString());
}

void ClientBluetooth::deconnecter(QBluetoothAddress adresse)
{
    deconnecter(adresse.toString());
}

void ClientBluetooth::deconnecter(QString adresseMAC)
{
    if(sockets.contains(adresseMAC))
    {
        QBluetoothSocket* socket = sockets.value(adresseMAC);
        qDebug() << Q_FUNC_INFO << socket << adresseMAC << socket->state();
        if(socket->state() == QBluetoothSocket::ConnectingState ||
           socket->state() == QBluetoothSocket::ConnectedState)
        {
            socket->disconnectFromService();
        }
    }
}

void ClientBluetooth::arreter()
{
    if(!peripheriqueLocal.isValid())
    {
        qCritical() << Q_FUNC_INFO;
        return;
    }

    arreterRecherche();
    arreterRechercheService();

    QMapIterator<QString, QBluetoothSocket*> i(sockets);
    while(i.hasNext())
    {
        i.next();
        if(i.value()->state() == QBluetoothSocket::ConnectingState ||
           i.value()->state() == QBluetoothSocket::ConnectedState)
        {
            i.value()->disconnectFromService();
        }
        delete i.value();
        sockets.remove(i.key());
    }
}

void ClientBluetooth::demarrerRecherche()
{
    if(!discoveryAgentDevice->isActive())
    {
        qDebug() << Q_FUNC_INFO;
        discoveryAgentDevice->start();
    }
}

void ClientBluetooth::arreterRecherche()
{
    qDebug() << Q_FUNC_INFO;
    if(discoveryAgentDevice->isActive())
        discoveryAgentDevice->stop();
}

void ClientBluetooth::demarrerRechercheService()
{
    if(!discoveryAgentService->isActive())
    {
        qDebug() << Q_FUNC_INFO;
        QBluetoothUuid uuid = QBluetoothUuid(QBluetoothUuid::SerialPort);
        discoveryAgentService->setUuidFilter(uuid);
        discoveryAgentService->start(
          QBluetoothServiceDiscoveryAgent::FullDiscovery);
    }
}

void ClientBluetooth::arreterRechercheService()
{
    qDebug() << Q_FUNC_INFO;
    if(discoveryAgentService->isActive())
        discoveryAgentService->stop();
}

void ClientBluetooth::decouvrirAppareil(QBluetoothDeviceInfo device)
{
    qDebug() << Q_FUNC_INFO << device.name() << device.address()
             << device.rssi();

    if(device.name().startsWith(prefixeNomAppareil))
    {
        qDebug() << Q_FUNC_INFO << device.name() << device.address()
                 << device.rssi();
        enregistrerAppareil(device);
        connecter(device);
    }
}

void ClientBluetooth::decouvrirService(QBluetoothServiceInfo service)
{
    /*qDebug() << Q_FUNC_INFO << service.serviceName() << service.serviceUuid()
             << service.device().name() << service.device().address()
             << service.device().rssi();*/
    if(service.device().name().startsWith(prefixeNomAppareil) &&
       service.serviceName() == serviceNom)
    {
        qDebug() << Q_FUNC_INFO << service.serviceName()
                 << service.serviceUuid() << service.device().name()
                 << service.device().address() << service.device().rssi();
        serviceServeur = service;
        arreterRechercheService();
        enregistrerAppareil(service.device());
        connecter(service.device());
    }
}

void ClientBluetooth::gererConnexion()
{
    QBluetoothSocket* socket     = qobject_cast<QBluetoothSocket*>(sender());
    QString           adresseMAC = recupererAdresseMAC(socket);
    qDebug() << Q_FUNC_INFO << socket << adresseMAC << socket->state();
    if(socket->state() == QBluetoothSocket::ConnectedState)
    {
        // Exemple
        socket->write(QByteArray("Hello world !\n"));
    }
}

void ClientBluetooth::deconnecterSocket()
{
    QBluetoothSocket* socket     = qobject_cast<QBluetoothSocket*>(sender());
    QString           adresseMAC = recupererAdresseMAC(socket);

    if(!adresseMAC.isEmpty())
    {
        qDebug() << Q_FUNC_INFO << socket << adresseMAC << socket->state();
        delete sockets[adresseMAC];
        int n = sockets.remove(adresseMAC);
        qDebug() << Q_FUNC_INFO << adresseMAC << "remove" << n;
        int indice = recupererIndiceAppareil(adresseMAC);
        if(indice != -1)
            peripheriquesDistants.removeAt(indice);
    }
}

void ClientBluetooth::lireDonneesSocket()
{
    QBluetoothSocket* socket = qobject_cast<QBluetoothSocket*>(sender());
    if(sockets.contains(socket->peerAddress().toString()))
    {
        qDebug() << Q_FUNC_INFO << socket->peerName()
                 << socket->peerAddress().toString();
        trames[socket->peerAddress().toString()] += socket->readAll();
        // Exemple
        if(trames[socket->peerAddress().toString()].endsWith("\r\n") ||
           trames[socket->peerAddress().toString()].endsWith("\r") ||
           trames[socket->peerAddress().toString()].endsWith("\n"))
        {
            qDebug() << Q_FUNC_INFO << trames[socket->peerAddress().toString()];
            trames[socket->peerAddress().toString()].clear();
        }
    }
}

void ClientBluetooth::lireChangementEtatSocket(
  QBluetoothSocket::SocketState etat)
{
    QBluetoothSocket* socket = qobject_cast<QBluetoothSocket*>(sender());
    qDebug() << Q_FUNC_INFO << socket << "etat" << etat;
    // qDebug() << Q_FUNC_INFO << socket << "etat" << socket->state() << etat;
}

void ClientBluetooth::lireErreurSocket(QBluetoothSocket::SocketError erreur)
{
    QBluetoothSocket* socket = qobject_cast<QBluetoothSocket*>(sender());
    qDebug() << Q_FUNC_INFO << socket << socket->state() << "erreur" << erreur;
}

void ClientBluetooth::lireErreurDevice(QBluetoothLocalDevice::Error erreur)
{
    qDebug() << Q_FUNC_INFO << erreur;
}
