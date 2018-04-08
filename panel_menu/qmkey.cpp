#include "qmkey.h"
#include <sys/types.h>
#include <unistd.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <signal.h>
#define DEBUG_01
QMkey::QMkey(QObject *parent) :
    QObject(parent)
{
    key0=new QMkey0(this);
    Q_ASSERT(key0!=NULL);
    if(key0->exits==true)
    {
        connect(key0,SIGNAL(sigKey(int,int)),this,SLOT(slotKey(int,int)),Qt::QueuedConnection);
        key0->start();
    }


    key1=new QMkey1(this);
    Q_ASSERT(key1!=NULL);
    if(key1->exits==true)
    {

        connect(key1,SIGNAL(sigKey(int,int)),this,SLOT(slotKey(int,int)),Qt::QueuedConnection);
        key1->start();
    }

#define DEBUG_0
#ifdef DEBUG_0
                    printf("QMkey::QMkey...\n\r");
#endif

}

void QMkey::slotKey(int key,int val)
{
    if(key!=Qt::Key_Pause)
    //if(key!=Qt::Key_Back)
    {

#ifdef DEBUG_0
                    printf("QMkey::slotKey key %d,val %d\n\r",key,val);
#endif
        emit sigKey(key,val);
    }
    else
    {

#ifdef DEBUG_0
                    printf("QMkey::slotKey kill net2pid \n\r");
#endif
        QFile file("/var/volatile/net2spi.pid");
        file.open(QIODevice::ReadOnly);
        if(file.isOpen())
        {
           QString pid=QString(file.readLine());
           if(pid.contains('\r'))
               pid.remove('\r');
           if(pid.contains('\n'))
               pid.remove('\n');

           //QString cmd=QString("kill -%1 %2").arg(SIGUSR2).arg(pid);
           //qDebug()<<"p"<<cmd;
           //system(cmd.toLatin1().data());
           kill(pid.toInt(),SIGUSR2);
        }
    }
}
