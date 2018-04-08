#include <QtCore/QCoreApplication>
#include "qpanelinterface.h"
#include <QDebug>

void help(void)
{
    qDebug()<<"-h       help";
    qDebug()<<"-clr     clear lcd";
    qDebug()<<"-l       show line";
    qDebug()<<"     ex. -l 1/0 'string'";
    qDebug()<<"-led     on/off led";
    qDebug()<<"     ex. -led 1/0 idx";
    qDebug()<<"-bz      on/off bz";
    qDebug()<<"     ex. -bz 1/0";
}

int main(int argc, char *argv[])
{
     QPanelInterface *iface;
    QCoreApplication a(argc, argv);

    if(argc<2)
    {
        help();
        goto app_exit;
    }

    if(strncmp(argv[1],"-h",2)==0)
    {

        help();
        goto app_exit;
    }


    iface=new QPanelInterface("com.mooncell.panel","/panel/path",QDBusConnection::systemBus());
    Q_ASSERT(iface!=NULL);
    if(!iface->isValid())
    {
        qDebug()<<qPrintable(QDBusConnection::systemBus().lastError().message());
        delete iface;
        //throw QString("err:dbus inter face");
        qDebug()<<"err:dbus inter face";
        a.exit(-1);
        return -1;
    }

    if(strcmp(argv[1],"-clr")==0)
    {

        iface->setLcdClear();
    }
    else if(strcmp(argv[1],"-l")==0)
    {
        if(argc<4)
        {
            qDebug()<<"-l n 'string'";
            qDebug()<<"n = 0 or 1 ,string length less 16";
        }
        else{
            //qDebug()<<argv[2]<<argv[3];
            if(atoi(argv[2])>1)
            {
                qDebug()<<"n = 0 or 1 ,string length less 16";
            }
            else
            {
                 iface->setLcdLine(atoi(argv[2]),argv[3]);
            }
        }
    }
    else if(strcmp(argv[1],"-led")==0)
    {
        iface->setLed(atoi(argv[2])?1:0,atoi(argv[3]));

    }
    else if(strcmp(argv[1],"-bz")==0)
    {
        iface->setBz(atoi(argv[2])?1:0,100);
    }
    else{
        help();
    }


app_exit:
    a.exit(0);
    return 0;//a.exec();
}
