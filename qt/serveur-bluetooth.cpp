#include <QApplication>
#include <QObject>
#include <QtBluetooth>
#include <QString>
#include <QByteArray>
#include <QList>
#include <QVector>
#include <QDebug>

static const QString serviceUuid(
  QStringLiteral("0000110a-0000-1000-8000-00805f9b34fb"));

class ServeurBluetooth : public QObject
{
    Q_OBJECT
  private:
    QBluetoothServer*          serveur;
    QVector<QBluetoothSocket*> sockets;
    QBluetoothLocalDevice      peripheriqueLocal;
    QBluetoothServiceInfo      serviceInfo;
    QList<QBluetoothAddress>   peripheriquesDistants;
    QByteArray                 trame;

  public:
    ServeurBluetooth(QObject* parent = nullptr);
    ~ServeurBluetooth();
    void activerBluetooth();
    void desactiverBluetooth();
    void demarrer();
    void arreter();

  public slots:
    void gererClient();
    void deconnecterSocket();
    void lireSocket();
    void changerEtatSocket(QBluetoothSocket::SocketState etat);
    void renvoyerErreurSocket(QBluetoothSocket::SocketError erreur);
    void renvoyerErreurDevice(QBluetoothLocalDevice::Error erreur);

  signals:
    void clientConnecte(QString nom, QString adresse);
    void clientDeconnecte(QString nomModule);
};

ServeurBluetooth::ServeurBluetooth(QObject* parent) :
    QObject(parent), serveur(nullptr), peripheriqueLocal(), serviceInfo()
{
    qDebug() << Q_FUNC_INFO;
    activerBluetooth();
    demarrer();
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
            delete this->serveur;
            delete sockets.at(i);
            this->serveur = nullptr;
            sockets[i]    = nullptr;
        }
    }
}

int main(int argc, char* argv[])
{
    QApplication a(argc, argv);

    return a.exec();
}
