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
        setMinimumSize(280, 300);
        udpSocket = new QUdpSocket(this);
        timer = new QTimer(this);
        updTimer = new QTimer(this);
        setWindowTitle("Клиент");
        statusLabel = new QLabel("Связь с сервером: нет", this);
        heightLabel = new QLabel("Текущая высота: 0 м", this);
        widget = new HeightIndicatorWidget(this);
        layout->setSpacing(2);
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
    double side = qMin(width(), height());
    double scale = side / 200.0;
    double centerX = width() / 2;
    double centerY = height() / 2;

    QPainter painter(this);
    painter.setRenderHint(QPainter::Antialiasing);

    //Квадрат
    painter.setBrush(Qt::black);
    painter.save();
    painter.fillRect(this->rect(), Qt::black);
    painter.restore();

    //Внешний круг
    QPen pen(Qt::white);
    pen.setWidth(1);
    painter.setPen(pen);

    painter.translate(centerX, centerY);
    painter.scale(scale, scale);

    QRectF ellipseRect(-97.8, -97.4, 195, 195);
    painter.save();
    painter.drawEllipse(ellipseRect);
    painter.restore();

    //100-м деления
    pen.setWidth(1);
    painter.setPen(pen);

    painter.save();
    for (int j = 0; j < 60; ++j)
    {
        if ((j % 5) != 0)
            painter.drawLine(0, -91, 0, -97);
        painter.rotate(7.2);
    }
    painter.restore();

    //1000-м деления + цифры
    pen.setWidth(2);
    painter.setPen(pen);
    painter.save();
    for (int i = 0; i < 10; ++i)
    {
        painter.drawLine(0, -86, 0, -96);
        if (i == 0) painter.drawText(-10,-85,20,20,Qt::AlignHCenter | Qt::AlignTop,QString::number(0));
        else painter.drawText(-10,-85,20,20,Qt::AlignHCenter | Qt::AlignTop,QString::number(i));
        painter.rotate(36.0);
    }
    painter.restore();

    //Внутренний круг
    pen.setWidth(1);
    painter.setPen(pen);

    QRectF ellipsesRect(-60, -60, 120, 120);
    painter.save();
    painter.drawEllipse(ellipsesRect);
    painter.restore();

    //Внутренние 1000-м деления
    pen.setWidth(2);
    painter.setPen(pen);
    painter.save();
    for (int i = 0; i < 10; ++i)
    {
        painter.drawLine(0, -56, 0, -66);
        painter.rotate(36.0);
    }
    painter.restore();

    //Надписи в центре
    QFont font = painter.font();
    font.setPointSize(15);
    painter.setFont(font);
    painter.setPen(Qt::gray);
    painter.save();
    painter.drawText(-41, 2, 80, 50, Qt::AlignCenter, QString("Метры"));

    painter.restore();

    font.setPointSize(15);
    painter.setFont(font);
    painter.setPen(Qt::gray);
    painter.save();
    painter.drawText(-40, -60, 80, 50, Qt::AlignCenter, QString("Н"));

    painter.restore();

    //Стрелки
    pen.setWidth(1);
    double height_range = 9999;

    double points_total = 60;
    double angle_per_point = 360.0 / points_total;
    double angle = (m_height / height_range) * points_total * angle_per_point;

    double points = 100;
    double anglep = 360.0 / points;
    double hundreds = (m_height / 0.1);
    double anglee = (hundreds / height_range) * points * anglep;

    static const QPoint hundredsHand[3] = {
        QPoint(8, 9),
        QPoint(-8, 9),
        QPoint(0, -95)
    };

    static const QPoint thousandsHand[3] = {
        QPoint(10, 11),
        QPoint(-10, 11),
        QPoint(0, -57)
    };

    painter.setPen(QPen(Qt::darkGray, 2, Qt::SolidLine));
    painter.setBrush(Qt::white);

    painter.save();
    painter.rotate(anglee);
    painter.drawConvexPolygon(hundredsHand, 3);
    painter.restore();

    painter.setPen(QPen(Qt::darkGray, 2, Qt::SolidLine));
    painter.setBrush(Qt::white);

    painter.save();
    painter.rotate(static_cast<int>(angle));
    painter.drawConvexPolygon(thousandsHand, 3);
    painter.restore();

    //Закругленный квадрат и текст метров
    painter.setBrush(Qt::black);
    QPainterPath path;
    path.addRoundedRect(QRectF(-29.5,
                               -16,
                               58,
                               28),
                        4,
                        4);
    painter.setPen(pen);
    painter.fillPath(path, Qt::black);
    painter.drawPath(path);

    font.setPointSize(15);
    painter.setFont(font);
    painter.setPen(Qt::white);
    painter.drawText(QRectF(-32,
                            -18,
                            65,
                            30),
                     Qt::AlignCenter, QString::number(m_height));
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
