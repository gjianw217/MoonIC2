#include "qmkey1.h"
#define DEBUG_01

QMkey1::QMkey1(QObject *parent) :
    QThread(parent)
{
    exits=false;
    QDir dir("/sys/devices/platform/panel_key/input/");
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


    if(in==true)
    {
        fd_event1=open("/dev/input/event1",O_RDONLY);
        if(fd_event1<0)
        {
            throw QString("not find panel_key /dev/input/event1!");
            return;
        }
        else
        {
            printf("open fd_devent1 success\n\r");
        }
    }
    else
    {
        fd_event1=open("/dev/input/event0",O_RDONLY);
        if(fd_event1<0)
        {
            throw QString("not find panel_key /dev/input/event0!");

            return;
        }

    }

    exits=true;
}

void QMkey1::run()
{

#ifdef DEBUG_0
                    printf(" QMkey1::run()\n\r");
#endif
    while(1)
    {
        struct input_event ev_key;
        int count;
         printf(" read before ...\n\r");
        count = read(fd_event1,&ev_key,sizeof(struct input_event));
          printf("read after ....\n\r");

        if(count>0)
        {
            if(EV_KEY==ev_key.type && ev_key.value==0)
            {
                int key=0;
                switch (ev_key.code)
                {
                    case KEY_LEFT:
                        key=Qt::Key_Left;
                        break;
                    case KEY_RIGHT:
                        key=Qt::Key_Right;
                        break;
                    case KEY_ENTER:
                        key=Qt::Key_Enter;
                        break;
                    case KEY_PAUSE:
                    key=Qt::Key_Pause;
                        break;
                    default:
                        continue;
                        break;
                    }
                    emit this->sigKey(key,ev_key.value);

#ifdef DEBUG_0
                    printf("key code %d\n\r",ev_key.code);
#endif
            }
            printf("key code %d,%d,%d\n\r",ev_key.code,ev_key.type,ev_key.value);
        }
        printf("read  return %d\n\r",count);
    }//end while

}
