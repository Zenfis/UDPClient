#ifndef UDPCLIENT_H
#define UDPCLIENT_H

#include <QWidget>
#include <QUdpSocket>
#include <QLabel>
#include <QTimer>
#include <QDateTime>
#include <QTime>

class UDPClient : public QWidget
{
    Q_OBJECT

public:
    explicit UDPClient(QWidget *parent = nullptr);
    ~UDPClient();

signals:

private slots:
    void readingDatagrams();
    void signalServer();

private:
    QUdpSocket *udpSocket;
    QLabel *statusLabel;
    QLabel *heightLabel;
    QTimer *timer;
    QTime lastMessageTime;
    QTime twoSecondsAgo;
    QDateTime dataTime;
    QTime curTime;
};

#endif // UDPCLIENT_H
