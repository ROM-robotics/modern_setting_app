#ifndef ETHERNET_H
#define ETHERNET_H

#include <QObject>
#include <QTcpSocket>
#include <QString>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>
#include <QTimer>


const QString DEFAULT_ROBOT_IP = "10.0.0.5";
int const DEFAULT_ROBOT_PORT = 8888;

class Ethernet : public QObject
{
    Q_OBJECT

public:
    explicit Ethernet(QObject *parent = nullptr);
    ~Ethernet();

    // Connection methods
    bool connectToRobot(const QString &ipAddress = DEFAULT_ROBOT_IP, int port = DEFAULT_ROBOT_PORT);
    void disconnect();
    bool isConnected() const;

    // WiFi scanning
    void requestWifiList();
    void requestWifiStatus();
    void connectToWifi(const QString &ssid, const QString &password);

signals:
    void connected();
    void disconnected();
    void error(const QString &errorMessage);
    void wifiListReceived(const QJsonArray &wifiList);
    void wifiStatusReceived(const QJsonObject &status);
    void wifiConnecting(const QString &ssid);
    void wifiConnectionResult(bool success, const QString &message);

private slots:
    void onConnected();
    void onDisconnected();
    void onReadyRead();
    void onError(QAbstractSocket::SocketError socketError);

private:
    QTcpSocket *m_socket;
    QString m_ipAddress = DEFAULT_ROBOT_IP;
    int m_port = DEFAULT_ROBOT_PORT;
    QByteArray m_buffer;

    void processReceivedData(const QByteArray &data);
    void sendCommand(const QString &command, const QJsonObject &params = QJsonObject());
};

#endif // ETHERNET_H
