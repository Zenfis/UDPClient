#include "udpclient.h"
#include <QVBoxLayout>

UDPClient::UDPClient(QWidget *parent) :
    QWidget(parent)
{
    setWindowTitle("Клиент");

    statusLabel = new QLabel("Связь с сервером: НЕТ", this);
    heightLabel = new QLabel("Текущая высота: 0 м", this);

    QVBoxLayout *layout = new QVBoxLayout(this);

    layout->addWidget(statusLabel);
    layout->addWidget(heightLabel);
    setLayout(layout);

    udpSocket = new QUdpSocket(this);

    connect(udpSocket, SIGNAL(readyRead()), this, SLOT(readingDatagrams()));

    if (!udpSocket->bind(QHostAddress::LocalHost, 1111)) {
        qDebug() << "Не удалось забиндить на адрес и порт";
    } else {
        qDebug() << "Забинжен на данный адрес и порт";
    }
}

UDPClient::~UDPClient(){}

void UDPClient::readingDatagrams()
{
    QHostAddress sender;
    quint16 senderPort;

    while (udpSocket->hasPendingDatagrams())
    {
        QByteArray datagram;
        datagram.resize(udpSocket->pendingDatagramSize());
        udpSocket->readDatagram(datagram.data(), datagram.size(), &sender, &senderPort);
        heightLabel->setText(QString(datagram));
        qDebug() << "Получено сообщение: " << datagram.data()
            << "\nОтправитель:\nIP: " << sender.toString()
            << "\nPort: " << QString("%1").arg(senderPort)
            << "\n==========";
    }

    /*
   while (udpSocket->hasPendingDatagrams())
   {
       QByteArray datagram;
       datagram.resize(udpSocket->pendingDatagramSize());
       udpSocket->readDatagram(datagram.data(), datagram.size(), &sender, &senderPort);

       // Deserialize the QByteArray into a Message1 instance
       QDataStream in(&datagram, QIODevice::ReadOnly);
       Message1 message;
       in >> message.header >> message.height;

       // Set the heightLabel text to the height value
       heightLabel->setText(QString("Текущая высота: %1 м").arg(message.height));

       qDebug() << "Получено сообщение: " << message.header << ", " << message.height
           << "\nОтправитель:\nIP: " << sender.toString()
           << "\nPort: " << QString("%1").arg(senderPort)
           << "\n==========";
   }*/
}
