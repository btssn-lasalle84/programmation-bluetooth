#ifndef SERVEUR_BLUETOOTH_H
#define SERVEUR_BLUETOOTH_H

#include <QObject>
#include <QtBluetooth>
#include <QString>
#include <QByteArray>
#include <QList>
#include <QVector>

static const QString serviceUuid(
  QStringLiteral("0000110a-0000-1000-8000-00805f9b34fb"));

class ServeurBluetooth : public QObject
{
    Q_OBJECT
  private:
    const int NB_CLIENTS_MAX   = 2; 
    QBluetoothServer*          serveur;
    QVector<QBluetoothSocket*> sockets;
    QBluetoothLocalDevice      peripheriqueLocal;
    QBluetoothServiceInfo      serviceInfo;
    QList<QBluetoothAddress>   peripheriquesDistants;
    QVector<QByteArray>        trames;

    int recupererNumeroSocket(QBluetoothSocket* socket) const;

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

#endif //SERVEUR_BLUETOOTH_H
