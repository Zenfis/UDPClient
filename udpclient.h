#ifndef UDPCLIENT_H
#define UDPCLIENT_H

#include <QWidget>
#include <QUdpSocket>
#include <QLabel>

class UDPClient : public QWidget
{
    Q_OBJECT

public:
    explicit UDPClient(QWidget *parent = nullptr);
    ~UDPClient();

signals:

private slots:
    void readingDatagrams();

private:
    QUdpSocket *udpSocket;
    QLabel *statusLabel;
    QLabel *heightLabel;
};

#endif // UDPCLIENT_H
