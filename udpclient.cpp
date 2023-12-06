#include "udpclient.h"

#include <QVBoxLayout>
#include <QPainterPath>
#include <QFileInfo>

#pragma pack(push, 1)
struct Message1
{
    quint16 header = 0xABCD;
    quint16 height;
};
struct Message2
{
    quint16 header = 0x1234;
};
#pragma pack(pop)

QString settingsFilePath = "C:\\Users\\user01\\Documents\\projects\\UDPClient\\settings.ini";
QSettings settings("C:\\Users\\user01\\Documents\\projects\\UDPClient\\settings.ini", QSettings::IniFormat);
QString client_ip = settings.value("clienthost/client_ip").toString();
quint16 client_port = settings.value("clienthost/client_port").toUInt();
QString server_ip = settings.value("sendtoserver/server_ip").toString();
quint16 server_port = settings.value("sendtoserver/server_port").toUInt();

Message1 msg1;
Message2 msg2;

UDPClient::UDPClient(QWidget *parent) :
    QWidget(parent)
{
    //Интерфейс
    {
        QVBoxLayout *layout = new QVBoxLayout(this);
        setMinimumSize(380, 400);
        setMaximumSize(740, 760);
        udpSocket = new QUdpSocket(this);
        timer = new QTimer(this);
        updTimer = new QTimer(this);
        setWindowTitle("Клиент");
        statusLabel = new QLabel("Связь с сервером: нет", this);
        heightLabel = new QLabel("Текущая высота: 0 м", this);
        widget = new HeightIndicatorWidget(this);
        layout->setSpacing(2);
        //layout->setContentsMargins(7, 0, 0, 0);
        statusLabel->setSizePolicy(QSizePolicy::MinimumExpanding, QSizePolicy::Fixed);
        statusLabel->setAlignment(Qt::AlignLeft);
        layout->addWidget(statusLabel);
        heightLabel->setSizePolicy(QSizePolicy::MinimumExpanding, QSizePolicy::Fixed);
        heightLabel->setAlignment(Qt::AlignLeft);
        layout->addWidget(heightLabel);
        layout->addWidget(widget);
        setLayout(layout);
    }
    //Проверка на ошибки
    {
        if (!QFileInfo::exists(settingsFilePath)) { qDebug() << "Файл настроек не найден"; return; }
        if (client_ip.isEmpty()) { qDebug() << "client_ip не указан в settings.ini"; return; }
        if (client_port == 0) { qDebug() << "client_port не указан в settings.ini"; return; }
        if (server_ip.isEmpty()) { qDebug() << "server_ip не указан в settings.ini"; return; }
        if (server_port == 0) { qDebug() << "server_port не указан в settings.ini"; return; }

        QHostAddress clientAddress;
        bool isValid = clientAddress.setAddress(client_ip);

        if (!isValid) { qDebug() << "Неверный IP адрес"; return; }
        if (udpSocket->state() == QAbstractSocket::BoundState) { qDebug() << "Сокет уже привязан"; return; }

        bool socketBinded = udpSocket->bind(clientAddress, client_port);

        if (!socketBinded) { qDebug() << "Ошибка привязки сокета: " << udpSocket->errorString(); return; }
    }
    //Соединения
    {
        connect(udpSocket, SIGNAL(readyRead()), this, SLOT(readingDatagrams()));
        connect(timer, SIGNAL(timeout()), this, SLOT(signalServer()));
        connect(updTimer, SIGNAL(timeout()), this, SLOT(updateHeight()));
        timer->start(2000);
        updTimer->start(25);
    }
    qDebug() << "Клиент запущен!";
}

UDPClient::~UDPClient(){}

HeightIndicatorWidget::HeightIndicatorWidget(QWidget* parent) : QWidget(parent) { m_height = 0; }

void HeightIndicatorWidget::setHeight(int height) { m_height = height; update(); }

void HeightIndicatorWidget::paintEvent(QPaintEvent* event)
{
    QPainter painter(this);
    painter.setRenderHint(QPainter::Antialiasing);
    QSize newSize = this->size();
    double a = qMin(width(), height());
    double k = a / 100.0;

    double scaleFactor = (newSize.width() + newSize.height()) / (2 * width() + 2 * height());

    int centerX = newSize.width() / 2;
    int centerY = newSize.height() / 2;

    painter.setBrush(Qt::black);
    painter.fillRect(this->rect(), Qt::black);
    painter.save();
    painter.restore();

    /*painter.translate(this->rect().center());
    painter.setPen(Qt::green);
    painter.scale(k / 2, k / 2);
    for (int j = 0; j < 40; ++j)
    {
        if ((j % 5) != 0)
            painter.drawLine(0, -92, 0, -96);
        painter.rotate(10.0);
    }
    painter.restore();*/
    painter.setPen(Qt::white);
    painter.translate(this->rect().center());
    painter.scale(k / 2, k / 2);
    for (int i = 0; i < 10; ++i)
    {
        painter.drawLine(0, -88, 0, -96);
        if (i == 0)  painter.drawText(-10,-88,20,20,Qt::AlignHCenter | Qt::AlignTop,QString::number(0));
        else  painter.drawText(-10,-88,20,20,Qt::AlignHCenter | Qt::AlignTop,QString::number(i));
        painter.rotate(36.0);
    }


    /*double scaleFactor = (newSize.width() + newSize.height()) / (2 * 350.0 + 2 * 90.0);

    double m_height_range = 9999;
    double points_total = 40;
    double angle_per_point = 360.0 / points_total;
    double angle = (m_height / m_height_range) * points_total * angle_per_point;

    double range = 999;
    double points = 10;
    double anglep = 360.0 / points;
    double hundreds = (m_height / 0.1);
    double anglee = (hundreds / range) * points * anglep;

    static const QPoint fullHand[3] = {
        QPoint(11, 12),
        QPoint(-11, 12),
        QPoint(0, -182)
    };

    static const QPoint precHand[3] = {
        QPoint(11, 12),
        QPoint(-11, 12),
        QPoint(0, -115)
    };

    painter.save();
    painter.translate(width() / 2 - 4, height() / 2 + 2);
    painter.scale(width() / 370.0, height() / 370.0);

    painter.setPen(QPen(Qt::darkGray, 2, Qt::SolidLine));
    painter.setBrush(Qt::white);

    painter.save();
    painter.rotate(anglee);
    painter.drawConvexPolygon(fullHand, 3);
    painter.restore();

    painter.setPen(QPen(Qt::darkGray, 2, Qt::SolidLine));
    painter.setBrush(Qt::white);

    painter.save();
    painter.rotate(static_cast<int>(angle));
    painter.drawConvexPolygon(precHand, 3);

    painter.restore();
    painter.restore();

    QPainterPath path;
    path.addRoundedRect(QRectF(centerX - 53 * scaleFactor,
                               centerY - 33 * scaleFactor,
                               100 * scaleFactor,
                               50 * scaleFactor),
                        10 * scaleFactor,
                        10 * scaleFactor);
    QPen pen(Qt::white, 1 * scaleFactor);
    painter.setPen(pen);
    painter.fillPath(path, Qt::black);
    painter.drawPath(path);

    QFont font = painter.font();
    font.setPointSize(30 * scaleFactor);
    painter.setFont(font);
    painter.setPen(Qt::white);
    painter.drawText(QRectF(centerX - 53 * scaleFactor,
                            centerY - 35 * scaleFactor,
                            100 * scaleFactor,
                            50 * scaleFactor ),
                     Qt::AlignCenter, QString::number(m_height));*/
}

void UDPClient::signalServer()
{
    QByteArray datagram;
    QDataStream out(&datagram, QIODevice::WriteOnly);
    out << msg2.header;
    udpSocket->writeDatagram(datagram.constData(), datagram.size(), QHostAddress(server_ip), server_port);
}

void UDPClient::updateHeight()
{
    widget->setHeight(static_cast<int>(msg1.height));
    widget->update();
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

        if (datagram.size() == sizeof(Message1))
        {
            if (msg1.header == static_cast<int>(0xABCD))
            {
                statusLabel->setText("Связь с сервером: да");
                heightLabel->setText(QString("Текущая высота: %1 м").arg(msg1.height));
            }
        }
        else if (lastMessageTime > twoSecondsAgo) { statusLabel->setText("Связь с сервером: нет"); }
        else { statusLabel->setText("Связь с сервером: нет"); }
    }
}
