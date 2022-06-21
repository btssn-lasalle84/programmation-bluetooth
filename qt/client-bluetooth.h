#ifndef CLIENT_BLUETOOTH_H
#define CLIENT_BLUETOOTH_H

#include <QObject>
#include <QtBluetooth>
#include <QString>
#include <QByteArray>
#include <QList>
#include <QMap>

class ClientBluetooth : public QObject
{
    Q_OBJECT
  private:
    QMap<QString, QBluetoothSocket*> sockets;
    QBluetoothDeviceDiscoveryAgent*  discoveryAgentDevice;
    QBluetoothServiceDiscoveryAgent* discoveryAgentService;
    QBluetoothLocalDevice            peripheriqueLocal;
    QList<QBluetoothDeviceInfo>      peripheriquesDistants;
    QMap<QString, QByteArray>        trames;
    QString                          prefixeNomAppareil;
    QString                          serviceNom;
    QBluetoothServiceInfo            serviceServeur;

    void    activerBluetooth();
    void    desactiverBluetooth();
    void    creerAgentsRecherche();
    void    enregistrerAppareil(const QBluetoothDeviceInfo device);
    QString recupererAdresseMAC(QBluetoothSocket* socket) const;
    QString recupererAdresseMAC(QString nom) const;
    int     recupererIndiceAppareil(QString adresseMAC) const;

  public:
    ClientBluetooth(QObject* parent = nullptr);
    ~ClientBluetooth();

    QList<QBluetoothDeviceInfo> getPeripheriquesDistants() const;
    QString                     getPrefixeNomAppareil() const;
    void    setPrefixeNomAppareil(QString prefixeNomAppareil);
    QString getServiceNom() const;
    void    setServiceNom(QString serviceNom);
    void    enregistrerAppareil(const QString adresseMAC);
    void    connecterAppareil(QString nom);
    void    deconnecterAppareil(QString nom);
    void    connecter(QBluetoothDeviceInfo device);
    void    connecter(QBluetoothAddress adresse);
    void    connecter(QString adresseMAC);
    void    deconnecter(QBluetoothDeviceInfo device);
    void    deconnecter(QBluetoothAddress adresse);
    void    deconnecter(QString adresseMAC);
    void    arreter();

  public slots:
    void demarrerRecherche();
    void arreterRecherche();
    void demarrerRechercheService();
    void arreterRechercheService();

  private slots:
    void decouvrirAppareil(QBluetoothDeviceInfo device);
    void decouvrirService(QBluetoothServiceInfo service);
    void gererConnexion();
    void deconnecterSocket();
    void lireDonneesSocket();
    void lireChangementEtatSocket(QBluetoothSocket::SocketState etat);
    void lireErreurSocket(QBluetoothSocket::SocketError erreur);
    void lireErreurDevice(QBluetoothLocalDevice::Error erreur);

  signals:
};

#endif // CLIENT_BLUETOOTH_H
