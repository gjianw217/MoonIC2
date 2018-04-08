#include <QCoreApplication>
#include <QtDBus>
#include "qpaneladaptor.h"
#include "qpaneldbus.h"
#include "qpanel.h"

int main(int argc, char *argv[])
{
    int ret;
    QCoreApplication a(argc, argv);
    QPanel *panel=new QPanel(&a);
    Q_ASSERT(panel!=NULL);
    if(argc>=2)
        ret=panel->initializtion(argv[1]);
    else
        ret=panel->initializtion(NULL);
    if(ret<0)
    {
        qDebug()<<"err:init serio port";
        exit(-1);
        return -1;
    }

    QPanelDbus *panelbus=new QPanelDbus(panel,&a);
     Q_ASSERT(panelbus!=NULL);

     QDBusConnection bus = QDBusConnection::systemBus();
     QPanelAdaptor adaptor(panelbus);

     if (!bus.registerService("com.mooncell.panel")) {
         qDebug() <<"err:register dbus"<< bus.lastError().message();
         exit(1);
     }

     if(! (bus.registerObject("/panel/path", &adaptor ,QDBusConnection::ExportAllSlots))){
         qDebug() <<"err:register dbus object"<<  bus.lastError().message();
         exit(1);
     }
#define DEBUG_0
#ifdef DEBUG_0
                    printf(" mpanel main ...\n\r");
#endif
    panelbus->setLcdClear();
    //panelbus->setLcdPos(1,1);
    sleep(5);
    panelbus->setLcd(4,0,"mooncell");
    sleep(1);
    panelbus->setLcd(4,1,"welcome");
    printf(" mpanel main ...\n\r");
    sleep(1);
    //panelbus->setLcdPos(1,1);
     //sleep(2);
     //panelbus->setLcdPos(0,8);
    //panelbus->setLcdLine(1,"welcome");

    return a.exec();
}
