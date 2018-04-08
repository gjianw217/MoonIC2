#include "qmkey0.h"

QMkey0::QMkey0(QObject *parent) :
    QThread(parent)
{
    exits=false;
    QDir dir("/sys/devices/platform/pwrkey/input/");
    if (!dir.exists())
        return;

    dir.setFilter(QDir::Dirs);

    dir.setSorting(QDir::DirsFirst);

    QFileInfoList list = dir.entryInfoList();
    int i=0;
    bool in=false;
    do
    {
        QFileInfo fileInfo = list.at(i);

       if(fileInfo.fileName()=="input0")
       {
           in=false;
           break;
       }
       if(fileInfo.fileName()=="input1")
       {
           in=true;
           break;
       }
       i++;

    }while(i<list.size());


    if(in==false)
    {
        fd_event0=open("/dev/input/event0",O_RDONLY);
        if(fd_event0<0)
        {
            throw QString("not find pwrkey /dev/input/event0!");
            return;
        }
    }
    else
    {
        fd_event0=open("/dev/input/event1",O_RDONLY);
        if(fd_event0<0)
        {
            throw QString("not find pwrkey /dev/input/event1!");
            return;
        }
    }
    exits=true;
}

void QMkey0::run()
{
    while(1)
    {
        struct input_event ev_key;
        int count;

        count = read(fd_event0,&ev_key,sizeof(struct input_event));

        if(count>0)
        {
            if(EV_KEY==ev_key.type && ev_key.value==0)
            {
                int key=0;
                switch (ev_key.code) {
                case KEY_BACK:
                    key=Qt::Key_Back;
                    break;
                case KEY_POWER:
                    key=Qt::Key_PowerDown;
                    break;
                default:
                    continue;
                    break;
                }
                emit this->sigKey(key,ev_key.value);
            }
        }
    }
}
