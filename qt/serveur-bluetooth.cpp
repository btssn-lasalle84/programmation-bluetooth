#include "serveur-bluetooth.h"
#include <QDebug>

ServeurBluetooth::ServeurBluetooth(QObject* parent) :
    QObject(parent), serveur(nullptr), sockets(NB_CLIENTS_MAX, nullptr), peripheriqueLocal(), serviceInfo()
{
    qDebug() << Q_FUNC_INFO;
    activerBluetooth();
    demarrer();
}

ServeurBluetooth::~ServeurBluetooth()
{
    arreter();
    desactiverBluetooth();
    qDebug() << Q_FUNC_INFO;
}

void ServeurBluetooth::activerBluetooth()
{
    if(!this->peripheriqueLocal.isValid())
    {
        return;
    }
    else
    {
        qDebug() << Q_FUNC_INFO;
        this->peripheriqueLocal.powerOn();
        this->peripheriqueLocal.setHostMode(
          QBluetoothLocalDevice::HostDiscoverable);
        this->peripheriquesDistants =
          this->peripheriqueLocal.connectedDevices();
        connect(&peripheriqueLocal,
                SIGNAL(error(QBluetoothLocalDevice::Error)),
                this,
                SLOT(renvoyerErreurDevice(QBluetoothLocalDevice::Error)));
    }
}

void ServeurBluetooth::desactiverBluetooth()
{
    qDebug() << Q_FUNC_INFO;
    this->peripheriqueLocal.setHostMode(QBluetoothLocalDevice::HostPoweredOff);
}

void ServeurBluetooth::demarrer()
{
    if(!peripheriqueLocal.isValid())
    {
        return;
    }
    if(serveur == nullptr)
    {
        serveur =
          new QBluetoothServer(QBluetoothServiceInfo::RfcommProtocol, this);
        connect(serveur, SIGNAL(newConnection()), this, SLOT(gererClient()));
        QBluetoothUuid uuid = QBluetoothUuid(serviceUuid);
        QString        serviceNom("SERVEUR_SERIAL_PORT");
        this->serviceInfo = this->serveur->listen(uuid, serviceNom);
        qDebug() << Q_FUNC_INFO;
    }
}

void ServeurBluetooth::arreter()
{
    if(!peripheriqueLocal.isValid())
    {
        return;
    }
    if(!this->serveur)
    {
        return;
    }

    this->serviceInfo.unregisterService();

    int nbSockets = sockets.size();
    for(int i = 0; i < nbSockets; ++i)
    {
        if(sockets.at(i))
        {
            if(sockets.at(i)->isOpen())
            {
                sockets.at(i)->close();
            }            
            delete sockets.at(i);            
            sockets[i]    = nullptr;
        }
    }
    delete this->serveur;
    this->serveur = nullptr;
}

void ServeurBluetooth::gererClient()
{
    QBluetoothSocket* socket = this->serveur->nextPendingConnection();
    if(!socket)
    {
        return;
    }
    // nouveau client
    qDebug() << Q_FUNC_INFO << socket->peerName()
             << socket->peerAddress().toString();
    for(int i = 0; i < sockets.size(); ++i)
    {
        if(sockets.at(i) == nullptr)
        {
            qDebug() << Q_FUNC_INFO << socket << "i" << i;
            sockets[i] = socket;
            connect(socket,
                    SIGNAL(disconnected()),
                    this,
                    SLOT(deconnecterSocket()));
            connect(socket, SIGNAL(readyRead()), this, SLOT(lireSocket()));
            connect(socket,
                    SIGNAL(error(QBluetoothSocket::SocketError)),
                    this,
                    SLOT(renvoyerErreurSocket(QBluetoothSocket::SocketError)));
            connect(socket,
                    SIGNAL(stateChanged(QBluetoothSocket::SocketState)),
                    this,
                    SLOT(changerEtatSocket(QBluetoothSocket::SocketState)));
            return;
        }
    }
}

int ServeurBluetooth::recupererNumeroSocket(QBluetoothSocket* socket) const
{
    if(socket == nullptr)
        return -1;
    for(int i = 0; i < sockets.size(); ++i)
    {
        if(sockets.at(i) == socket)
        {
            return i;
        }
    }
    return -1;
}

void ServeurBluetooth::deconnecterSocket()
{
    QBluetoothSocket* socket      = qobject_cast<QBluetoothSocket*>(sender());
    int               numeroSocket = recupererNumeroSocket(socket);
    if(numeroSocket != -1)
    {
        qDebug() << Q_FUNC_INFO << numeroSocket << socket << socket->peerName()
                 << socket->peerAddress().toString() << socket->state();
        delete sockets[numeroSocket];
        sockets[numeroSocket] = nullptr;
    }
}

void ServeurBluetooth::lireSocket()
{
    QBluetoothSocket* socket = qobject_cast<QBluetoothSocket*>(sender());
    int               numeroSocket = recupererNumeroSocket(socket);
    if(numeroSocket != -1)
    {
        qDebug() << Q_FUNC_INFO << socket->peerName()
                 << socket->peerAddress().toString();
        trames[numeroSocket] += socket->readAll();
        if(trames[numeroSocket].endsWith("\r\n") || trames[numeroSocket].endsWith("\r") || trames[numeroSocket].endsWith("\n"))
        {
            qDebug() << Q_FUNC_INFO << trames[numeroSocket];
            trames[numeroSocket].clear();
        }
    }
}

void ServeurBluetooth::changerEtatSocket(QBluetoothSocket::SocketState etat)
{
    qDebug() << Q_FUNC_INFO << etat;
}

void ServeurBluetooth::renvoyerErreurSocket(QBluetoothSocket::SocketError erreur)
{
    qDebug() << Q_FUNC_INFO << erreur;
}

void ServeurBluetooth::renvoyerErreurDevice(QBluetoothLocalDevice::Error erreur)
{
    qDebug() << Q_FUNC_INFO << erreur;
}
