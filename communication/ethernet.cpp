#include "ethernet.h"
#include <QDebug>

Ethernet::Ethernet(QObject *parent)
    : QObject(parent)
    , m_socket(new QTcpSocket(this))
    // , m_ipAddress("192.168.1.11")
    // , m_port(8888)
{
    // Connect socket signals
    connect(m_socket, &QTcpSocket::connected, this, &Ethernet::onConnected);
    connect(m_socket, &QTcpSocket::disconnected, this, &Ethernet::onDisconnected);
    connect(m_socket, &QTcpSocket::readyRead, this, &Ethernet::onReadyRead);
    connect(m_socket, &QTcpSocket::errorOccurred, this, &Ethernet::onError);

    // Don't auto-connect on construction - let MainWindow do it after setup
}

Ethernet::~Ethernet()
{
    if (m_socket->state() == QAbstractSocket::ConnectedState) {
        m_socket->disconnectFromHost();
    }
}

bool Ethernet::connectToRobot(const QString &ipAddress, int port)
{
    if (m_socket->state() == QAbstractSocket::ConnectedState) {
        qDebug() << "Already connected to robot";
        return true;
    }

    m_ipAddress = ipAddress;
    m_port = port;

    qDebug() << "Connecting to robot at" << ipAddress << ":" << port;
    m_socket->connectToHost(ipAddress, port);

    // Don't block UI - connection will be handled asynchronously via signals
    return true;
}

void Ethernet::disconnect()
{
    if (m_socket->state() == QAbstractSocket::ConnectedState) {
        m_socket->disconnectFromHost();
    }
}

bool Ethernet::isConnected() const
{
    return m_socket->state() == QAbstractSocket::ConnectedState;
}

void Ethernet::requestWifiList()
{
    if (!isConnected()) {
        emit error("Not connected to robot");
        qWarning() << "Cannot request WiFi list: Not connected to robot";
        return;
    }

    qDebug() << "Requesting WiFi list from robot";
    
    QJsonObject params;
    sendCommand("scan_wifi", params);
}

void Ethernet::requestWifiStatus()
{
    if (!isConnected()) {
        emit error("Not connected to robot");
        qWarning() << "Cannot request WiFi status: Not connected to robot";
        return;
    }

    qDebug() << "Requesting WiFi status from robot";
    
    QJsonObject params;
    sendCommand("get_wifi_status", params);
}

void Ethernet::connectToWifi(const QString &ssid, const QString &password)
{
    if (!isConnected()) {
        emit error("Not connected to robot");
        qWarning() << "Cannot connect to WiFi: Not connected to robot";
        return;
    }

    qDebug() << "Requesting WiFi connection to" << ssid;
    
    emit wifiConnecting(ssid);
    
    QJsonObject params;
    params["ssid"] = ssid;
    params["password"] = password;
    sendCommand("connect_wifi", params);
}

void Ethernet::onConnected()
{
    qDebug() << "Connected to robot computer";
    emit connected();
    
    // Automatically request WiFi status when connected
    QTimer::singleShot(100, this, &Ethernet::requestWifiStatus);
}

void Ethernet::onDisconnected()
{
    qDebug() << "Disconnected from robot computer";
    emit disconnected();
}

void Ethernet::onReadyRead()
{
    // Read all available data
    QByteArray data = m_socket->readAll();
    m_buffer.append(data);

    // Process complete messages (assuming newline-delimited JSON)
    while (m_buffer.contains('\n')) {
        int newlineIndex = m_buffer.indexOf('\n');
        QByteArray message = m_buffer.left(newlineIndex);
        m_buffer.remove(0, newlineIndex + 1);

        if (!message.isEmpty()) {
            processReceivedData(message);
        }
    }
}

void Ethernet::onError(QAbstractSocket::SocketError socketError)
{
    Q_UNUSED(socketError);
    QString errorMsg = m_socket->errorString();
    qWarning() << "Socket error:" << errorMsg;
    emit error(errorMsg);
}

void Ethernet::processReceivedData(const QByteArray &data)
{
    qDebug() << "Received data:" << data;

    // Parse JSON response
    QJsonDocument doc = QJsonDocument::fromJson(data);
    if (doc.isNull()) {
        qWarning() << "Failed to parse JSON response";
        return;
    }

    QJsonObject response = doc.object();
    QString type = response["type"].toString();

    if (type == "wifi_list") {
        QJsonArray wifiList = response["data"].toArray();
        qDebug() << "Received WiFi list with" << wifiList.size() << "networks";
        emit wifiListReceived(wifiList);
    } else if (type == "wifi_status") {
        QJsonObject status = response["data"].toObject();
        qDebug() << "Received WiFi status:" << status;
        emit wifiStatusReceived(status);
    } else if (type == "wifi_connection_result") {
        bool success = response["success"].toBool();
        QString message = response["message"].toString();
        qDebug() << "WiFi connection result:" << success << message;
        emit wifiConnectionResult(success, message);
        
        // Request updated WiFi status after connection attempt
        if (success) {
            QTimer::singleShot(1000, this, &Ethernet::requestWifiStatus);
        }
    } else if (type == "error") {
        QString errorMsg = response["message"].toString();
        qWarning() << "Robot error:" << errorMsg;
        emit error(errorMsg);
    }
}

void Ethernet::sendCommand(const QString &command, const QJsonObject &params)
{
    if (!isConnected()) {
        qWarning() << "Cannot send command: Not connected";
        return;
    }

    QJsonObject message;
    message["command"] = command;
    message["params"] = params;

    QJsonDocument doc(message);
    QByteArray data = doc.toJson(QJsonDocument::Compact);
    data.append('\n'); // Add newline delimiter

    qDebug() << "Sending command:" << data;
    m_socket->write(data);
    m_socket->flush();
}
