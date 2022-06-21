#include "serveur-bluetooth.h"
#include <QMapIterator>
#include <QDebug>

ServeurBluetooth::ServeurBluetooth(QObject* parent) :
    QObject(parent), serveur(nullptr), peripheriqueLocal(), serviceInfo()
{
    qDebug() << Q_FUNC_INFO;
    activerBluetooth();
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
        qCritical() << Q_FUNC_INFO;
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
                SLOT(lireErreurDevice(QBluetoothLocalDevice::Error)));
    }
}

void ServeurBluetooth::desactiverBluetooth()
{
    qDebug() << Q_FUNC_INFO;
    // this->peripheriqueLocal.setHostMode(QBluetoothLocalDevice::HostPoweredOff);
}

void ServeurBluetooth::demarrer()
{
    if(!peripheriqueLocal.isValid())
    {
        qCritical() << Q_FUNC_INFO;
        return;
    }
    if(serveur == nullptr)
    {
        serveur =
          new QBluetoothServer(QBluetoothServiceInfo::RfcommProtocol, this);
        connect(serveur,
                SIGNAL(newConnection()),
                this,
                SLOT(gererNouveauClient()));
        // QBluetoothUuid uuid = QBluetoothUuid(serviceUuid);
        // QString        serviceNom("SERVEUR_SERIAL_PORT");
        // QBluetoothUuid uuid = QBluetoothUuid(QBluetoothUuid::SerialPort);
        // QString        serviceNom("Serial Port");
        // this->serviceInfo = this->serveur->listen(uuid, serviceNom);
        this->serveur->listen(
          QBluetoothAddress(this->peripheriqueLocal.address()));
        qDebug() << Q_FUNC_INFO;

        QBluetoothServiceInfo::Sequence classId;

        classId << QVariant::fromValue(
          QBluetoothUuid(QBluetoothUuid::SerialPort));
        serviceInfo.setAttribute(
          QBluetoothServiceInfo::BluetoothProfileDescriptorList,
          classId);

        classId.prepend(QVariant::fromValue(QBluetoothUuid(serviceUuid)));
        serviceInfo.setAttribute(QBluetoothServiceInfo::ServiceClassIds,
                                 classId);

        serviceInfo.setAttribute(QBluetoothServiceInfo::ServiceName,
                                 tr("SERVEUR_SERIAL_PORT"));
        serviceInfo.setAttribute(QBluetoothServiceInfo::ServiceDescription,
                                 tr("Exemple bluetooth serveur"));
        serviceInfo.setAttribute(QBluetoothServiceInfo::ServiceProvider,
                                 tr("lasalle84.net"));

        serviceInfo.setServiceUuid(QBluetoothUuid(serviceUuid));

        serviceInfo.setAttribute(
          QBluetoothServiceInfo::BrowseGroupList,
          QBluetoothUuid(QBluetoothUuid::PublicBrowseGroup));

        QBluetoothServiceInfo::Sequence protocolDescriptorList;
        QBluetoothServiceInfo::Sequence protocol;
        protocol << QVariant::fromValue(QBluetoothUuid(QBluetoothUuid::L2cap));
        protocolDescriptorList.append(QVariant::fromValue(protocol));
        protocol.clear();
        protocol << QVariant::fromValue(QBluetoothUuid(QBluetoothUuid::Rfcomm))
                 << QVariant::fromValue(quint8(serveur->serverPort()));
        protocolDescriptorList.append(QVariant::fromValue(protocol));
        serviceInfo.setAttribute(QBluetoothServiceInfo::ProtocolDescriptorList,
                                 protocolDescriptorList);

        serviceInfo.registerService(
          QBluetoothAddress(this->peripheriqueLocal.address()));
    }
}

void ServeurBluetooth::arreter()
{
    if(!peripheriqueLocal.isValid())
    {
        qCritical() << Q_FUNC_INFO;
        return;
    }
    if(!this->serveur)
    {
        return;
    }

    this->serviceInfo.unregisterService();

    QMapIterator<QString, QBluetoothSocket*> i(sockets);
    while(i.hasNext())
    {
        i.next();
        delete i.value();
        sockets.remove(i.key());
    }

    delete this->serveur;
    this->serveur = nullptr;
}

void ServeurBluetooth::gererNouveauClient()
{
    QBluetoothSocket* socket = this->serveur->nextPendingConnection();
    if(!socket)
    {
        return;
    }
    // nouveau client ?
    qDebug() << Q_FUNC_INFO << "nouveau client ?" << socket
             << socket->peerName() << socket->peerAddress().toString();
    if(!sockets.contains(socket->peerAddress().toString()))
    {
        qDebug() << Q_FUNC_INFO << "nouveau client !" << socket
                 << socket->peerName() << socket->peerAddress().toString();
        sockets[socket->peerAddress().toString()] = socket;
        connect(socket,
                SIGNAL(disconnected()),
                this,
                SLOT(deconnecterSocket()));
        connect(socket, SIGNAL(readyRead()), this, SLOT(lireDonneesSocket()));
        connect(socket,
                SIGNAL(error(QBluetoothSocket::SocketError)),
                this,
                SLOT(lireErreurSocket(QBluetoothSocket::SocketError)));
        connect(socket,
                SIGNAL(stateChanged(QBluetoothSocket::SocketState)),
                this,
                SLOT(lireChangementEtatSocket(QBluetoothSocket::SocketState)));
    }
}

QString ServeurBluetooth::recupererAdresseMAC(QBluetoothSocket* socket) const
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

void ServeurBluetooth::deconnecterSocket()
{
    QBluetoothSocket* socket     = qobject_cast<QBluetoothSocket*>(sender());
    QString           adresseMAC = recupererAdresseMAC(socket);

    if(!adresseMAC.isEmpty())
    {
        qDebug() << Q_FUNC_INFO << socket << adresseMAC << socket->state();
        delete sockets[adresseMAC];
        int n = sockets.remove(adresseMAC);
        qDebug() << Q_FUNC_INFO << adresseMAC << "remove" << n;
    }
}

void ServeurBluetooth::lireDonneesSocket()
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
            // Une réponse
            sockets[socket->peerAddress().toString()]->write(
              QByteArray("ok\r\n"));
        }
    }
}

void ServeurBluetooth::lireChangementEtatSocket(
  QBluetoothSocket::SocketState etat)
{
    QBluetoothSocket* socket = qobject_cast<QBluetoothSocket*>(sender());
    qDebug() << Q_FUNC_INFO << socket << "etat" << etat;
    // qDebug() << Q_FUNC_INFO << socket << "etat" << socket->state() << etat;
}

void ServeurBluetooth::lireErreurSocket(QBluetoothSocket::SocketError erreur)
{
    QBluetoothSocket* socket = qobject_cast<QBluetoothSocket*>(sender());
    qDebug() << Q_FUNC_INFO << socket << socket->state() << "erreur" << erreur;
}

void ServeurBluetooth::lireErreurDevice(QBluetoothLocalDevice::Error erreur)
{
    qDebug() << Q_FUNC_INFO << erreur;
}
