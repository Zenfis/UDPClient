#include "udpclient.h"
#include <QVBoxLayout>

#pragma pack(push, 1)
struct Message1
{
    quint16 header = 0xABCD;
    quint16 height;
};
#pragma pack(pop)

#pragma pack(push, 1)
struct Message2
{
    quint16 header = 0x1234;
};
#pragma pack(pop)

Message1 msg1;
Message2 msg2;

UDPClient::UDPClient(QWidget *parent) :
    QWidget(parent)
{
    QVBoxLayout *layout = new QVBoxLayout(this);
    udpSocket = new QUdpSocket(this);
    timer = new QTimer(this);
    updTimer = new QTimer(this);

    setWindowTitle("Клиент");
    setFixedSize(250, 100);
    statusLabel = new QLabel("Связь с сервером: нет", this);
    heightLabel = new QLabel("Текущая высота: 0 м", this);
    layout->addWidget(statusLabel);
    layout->addWidget(heightLabel);
    setLayout(layout);

    if (udpSocket->bind(QHostAddress::LocalHost, 1111))
    {
        connect(udpSocket, SIGNAL(readyRead()), this, SLOT(readingDatagrams()));
        connect(timer, &QTimer::timeout, this, &UDPClient::signalServer);
        timer->start(2000);
        connect(updTimer, SIGNAL(timeout()), this, SLOT(updateHeightLabel()));
    }
    else
    {
        qDebug() << "Error";
    }
}

UDPClient::~UDPClient(){}

void UDPClient::signalServer()
{
    QByteArray datagram;
    QDataStream out(&datagram, QIODevice::WriteOnly);
    out << msg2.header;

    udpSocket->writeDatagram(datagram.constData(), datagram.size(), QHostAddress::LocalHost, 9999);
    curTime = QTime::currentTime();
    qDebug() << curTime.toString() << "- Ping";
}

void UDPClient::updateHeightLabel()
{
    heightLabel->setText(QString("Текущая высота: %1 м").arg(QString::number(static_cast<double>(msg1.height)))); //fix
    updTimer->stop();
}

void UDPClient::readingDatagrams()
{
    QHostAddress sender;
    quint16 senderPort;

    while (udpSocket->hasPendingDatagrams())
    {
        QByteArray datagram;
        datagram.resize(udpSocket->pendingDatagramSize());
        udpSocket->readDatagram(datagram.data(), datagram.size(), &sender, &senderPort);

        lastMessageTime = QTime::currentTime();
        twoSecondsAgo = QTime::currentTime().addSecs(-2);

        QDataStream in(&datagram, QIODevice::ReadOnly);

        in >> msg1.header >> msg1.height;

        if (msg1.header == 43981)
        {
            if (!updTimer->isActive())
            {
                updTimer->start(23000);
            }
            statusLabel->setText("Связь с сервером: да");
            curTime = QTime::currentTime();
            qDebug() << curTime.toString() << "- Pong";
        }
        else if (lastMessageTime > twoSecondsAgo)
        {
            statusLabel->setText("Связь с сервером: нет");
            heightLabel->setText(QString("Текущая высота: 0 м"));
        }
    }
}
