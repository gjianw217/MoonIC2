#include <QtCore/QCoreApplication>
#include "qbackthread.h"
#include <QDebug>

int main(int argc, char *argv[])
{
    QCoreApplication a(argc, argv);
    //qDebug()<<"start";
    QBackThread *thread=new QBackThread(&a);
    thread->start();
    return a.exec();
}
