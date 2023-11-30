#ifndef UDPCLIENT_H
#define UDPCLIENT_H

#include <QWidget>
#include <QUdpSocket>
#include <QLabel>
#include <QTimer>
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
    void updateHeightLabel();
    void signalServer();

private:
    QUdpSocket *udpSocket;
    QLabel *statusLabel;
    QLabel *heightLabel;
    QLabel *indicatorLabel;
    QTimer *timer;
    QTimer *updTimer;
    QTime lastMessageTime;
    QTime twoSecondsAgo;
    QTime curTime;
    QTime tfsa;
};

#endif // UDPCLIENT_H
