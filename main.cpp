#include "udpclient.h"
#include <QApplication>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);

    UDPClient client;
    client.show();

    return a.exec();
}
