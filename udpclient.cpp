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
    setWindowTitle("Клиент");

    statusLabel = new QLabel("Связь с сервером: нет", this);
    heightLabel = new QLabel("Текущая высота: 0 м", this);

    QVBoxLayout *layout = new QVBoxLayout(this);

    layout->addWidget(statusLabel);
    layout->addWidget(heightLabel);
    setLayout(layout);

    udpSocket = new QUdpSocket(this);

    connect(udpSocket, SIGNAL(readyRead()), this, SLOT(readingDatagrams()));

    timer = new QTimer(this);
    connect(timer, &QTimer::timeout, this, &UDPClient::signalServer);
    timer->start(2000);

    if (!udpSocket->bind(QHostAddress::LocalHost, 1111)) {
        qDebug() << "Не удалось забиндить на адрес и порт";
    } else {
        qDebug() << "Забинжен на данный адрес и порт";
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

        //TODO: fix this code resetting the height num to zero
        if (msg1.header == 43981)
        {
            dataTime = QDateTime::currentDateTime();
            statusLabel->setText("Связь с сервером: да");
            heightLabel->setText(QString("Текущая высота: %1 м").arg(msg1.height));
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
