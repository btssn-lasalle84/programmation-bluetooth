#ifndef SERVEUR_BLUETOOTH_H
#define SERVEUR_BLUETOOTH_H

#include <QObject>
#include <QtBluetooth>
#include <QString>
#include <QByteArray>
#include <QList>
#include <QMap>

/*static const QString serviceUuid(
  QStringLiteral("0000110a-0000-1000-8000-00805f9b34fb"));*/
static const QString serviceUuid(
  QStringLiteral("00001101-0000-1000-8000-00805f9b34fb"));

class ServeurBluetooth : public QObject
{
    Q_OBJECT
  private:
    QBluetoothServer*                serveur;
    QMap<QString, QBluetoothSocket*> sockets;
    QBluetoothLocalDevice            peripheriqueLocal;
    QBluetoothServiceInfo            serviceInfo;
    QList<QBluetoothAddress>         peripheriquesDistants;
    QMap<QString, QByteArray>        trames;

    void    activerBluetooth();
    void    desactiverBluetooth();
    QString recupererAdresseMAC(QBluetoothSocket* socket) const;

  public:
    ServeurBluetooth(QObject* parent = nullptr);
    ~ServeurBluetooth();
    void demarrer();
    void arreter();

  private slots:
    void gererNouveauClient();
    void deconnecterSocket();
    void lireDonneesSocket();
    void lireChangementEtatSocket(QBluetoothSocket::SocketState etat);
    void lireErreurSocket(QBluetoothSocket::SocketError erreur);
    void lireErreurDevice(QBluetoothLocalDevice::Error erreur);

  signals:
};

#endif // SERVEUR_BLUETOOTH_H
