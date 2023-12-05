#ifndef UDPCLIENT_H
#define UDPCLIENT_H

#include <QWidget>
#include <QUdpSocket>
#include <QLabel>
#include <QTimer>
#include <QTime>
#include <QSettings>

#include "heightindicatorwidget.h"

class UDPClient : public QWidget
{
    Q_OBJECT

public:
    explicit UDPClient(QWidget *parent = nullptr);
    ~UDPClient();

signals:

public slots:
    void updateHeight();

private slots:
    void readingDatagrams();
    void signalServer();

private:
    HeightIndicatorWidget *widget;
    QUdpSocket *udpSocket;
    QLabel *statusLabel;
    QLabel *heightLabel;
    QLabel *indicatorLabel;
    QTimer *timer;
    QTimer *updTimer;
    QTime lastMessageTime;
    QTime twoSecondsAgo;
    QTime curTime;
};

#endif // UDPCLIENT_H
