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
#pragma pack(pop)

#pragma pack(push, 1)
struct Message2
{
    quint16 header = 0x1234;
};
#pragma pack(pop)

Message1 msg1;
Message2 msg2;

QString settingsFilePath = "C:\\Users\\user01\\Documents\\projects\\UDPClient\\settings.ini";
QSettings settings("C:\\Users\\user01\\Documents\\projects\\UDPClient\\settings.ini", QSettings::IniFormat);
QString client_ip = settings.value("clienthost/client_ip").toString();
quint16 client_port = settings.value("clienthost/client_port").toUInt();
QString server_ip = settings.value("sendtoserver/server_ip").toString();
quint16 server_port = settings.value("sendtoserver/server_port").toUInt();

UDPClient::UDPClient(QWidget *parent) :
    QWidget(parent)
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
    layout->setContentsMargins(7, 0, 0, 0);
    statusLabel->setSizePolicy(QSizePolicy::MinimumExpanding, QSizePolicy::Fixed);
    statusLabel->setAlignment(Qt::AlignLeft);
    layout->addWidget(statusLabel);
    heightLabel->setSizePolicy(QSizePolicy::MinimumExpanding, QSizePolicy::Fixed);
    heightLabel->setAlignment(Qt::AlignLeft);
    layout->addWidget(heightLabel);
    layout->addWidget(widget);
    setLayout(layout);

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

    connect(udpSocket, SIGNAL(readyRead()), this, SLOT(readingDatagrams()));
    connect(timer, SIGNAL(timeout()), this, SLOT(signalServer()));
    connect(updTimer, SIGNAL(timeout()), this, SLOT(updateHeight()));
    timer->start(2000);
    updTimer->start(25000);

    qDebug() << "Клиент запущен!";
}

UDPClient::~UDPClient(){}

HeightIndicatorWidget::HeightIndicatorWidget(QWidget* parent) : QWidget(parent) { m_height = 0; }

void HeightIndicatorWidget::setHeight(int height) { m_height = height; update(); }

void HeightIndicatorWidget::paintEvent(QPaintEvent* event)
{
    QPainter painter(this);
    painter.setRenderHint(QPainter::Antialiasing);
    QPixmap pixmap("C:\\Users\\user01\\Documents\\projects\\UDPClient\\indicator.png");
    QSize newSize = this->size();
    QPixmap scaledPixmap = pixmap.scaled(newSize, Qt::IgnoreAspectRatio, Qt::SmoothTransformation);
    int marginX = (this->width() - scaledPixmap.width()) / 2;
    int marginY = (this->height() - scaledPixmap.height()) / 2;
    painter.drawPixmap(marginX - 5, marginY - 5, scaledPixmap);

    int centerX = (this->width() - scaledPixmap.width()) / 2 + scaledPixmap.width() / 2;
    int centerY = (this->height() - scaledPixmap.height()) / 2 + scaledPixmap.height() / 2;

    double scaleFactor = (newSize.width() + newSize.height()) / (2 * 350.0 + 2 * 90.0);

    double m_height_range = 9999;
    double points_total = 40;
    double angle_per_point = 360.0 / points_total;
    double angle = (m_height / m_height_range) * points_total * angle_per_point;

    //Arrows
    static const QPoint hourHand[3] = {
        QPoint(11 * scaleFactor, 12 * scaleFactor),
        QPoint(-11 * scaleFactor, 12 * scaleFactor),
        QPoint(0, -175 * scaleFactor)
    };

    static const QPoint minuteHand[3] = {
        QPoint(11 * scaleFactor, 12 * scaleFactor),
        QPoint(-11 * scaleFactor, 12 * scaleFactor),
        QPoint(0, -105 * scaleFactor)
    };

    painter.save();
    painter.translate(centerX - 4, centerY);

    QColor color(255, 255, 255);
    painter.setPen(QPen(Qt::lightGray, 2, Qt::SolidLine));
    painter.setBrush(color);

    painter.save();
    painter.rotate(angle);
    painter.drawConvexPolygon(hourHand, 3);
    painter.restore();

    painter.setPen(QPen(Qt::lightGray, 2, Qt::SolidLine));
    painter.setBrush(color);

    painter.save();
    painter.rotate(15.0 * 20.0);
    painter.drawConvexPolygon(minuteHand, 3);

    painter.restore();
    painter.restore();

    //Text and rect
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
                     Qt::AlignCenter, QString::number(m_height));
}

void UDPClient::signalServer()
{
    QByteArray datagram;
    QDataStream out(&datagram, QIODevice::WriteOnly);
    out << msg2.header;

    udpSocket->writeDatagram(datagram.constData(), datagram.size(), QHostAddress(server_ip), server_port);
    curTime = QTime::currentTime();
    qDebug() << curTime.toString() << "- Ping";
}

void UDPClient::updateHeight()
{
    widget->setHeight(static_cast<int>(msg1.height));
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

        QByteArray data = datagram.data();
        QString str = QString::fromUtf8(data);

        if (msg1.header == 43981)
        {
            if (!updTimer->isActive())
            {
                updTimer->start(25000);
            }
            statusLabel->setText("Связь с сервером: да");
            curTime = QTime::currentTime();
            qDebug() << curTime.toString() << "- Pong";
        }
        else if (msg1.header != 43981 && str.contains("Текущая высота:"))
        {
            heightLabel->setText(QString(datagram));
        }
        else if (lastMessageTime > twoSecondsAgo)
        {
            statusLabel->setText("Связь с сервером: нет");
            heightLabel->setText(QString("Текущая высота: 0 м"));
        }
        else
        {
            statusLabel->setText("Связь с сервером: нет");
            heightLabel->setText(QString("Текущая высота: 0 м"));
        }
    }
}
