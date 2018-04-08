#include "qpanel.h"

QPanel::QPanel(QObject *parent) :
    QObject(parent)
{
}
int QPanel::initializtion(char *arg)
{
    if(arg==NULL)
        arg=(char*)"/dev/ttyO5";

//    QFile f(arg);
//    if(!f.exists())
//    {
//        qDebug()<<"err: file"<<arg<<"not exists";
//        return -100;
//    }

     this->port=new Posix_QextSerialPort(arg,QextSerialBase::Polling);

    if(port==NULL)
    {

        return -1;
    }

    procotol=new QProtocol();
    if(procotol==NULL)
    {
        delete port;
        port=NULL;
        return -2;
    }

    timer =new QTimer(this);
    if(timer==NULL)
    {
        delete port;
        delete procotol;
        port=NULL;
        procotol=NULL;
        return -3;
    }

    timerdev=new QTimer(this);
    if(timerdev==NULL)
    {
        delete port;
        delete procotol;
        delete timer;
        port=NULL;
        procotol=NULL;
        timer=NULL;
        return -3;
    }

    port->open(QIODevice::ReadWrite);
    port->setBaudRate(BAUD115200);
    port->setDataBits(DATA_8);
    port->setParity(PAR_NONE);
    port->setStopBits(STOP_1);
    port->setFlowControl(FLOW_OFF);
    port->setTimeout(50);

    timer->start(100);
    timerdev->start(50);
    connect(timer,SIGNAL(timeout()),this,SLOT(readPort()));
    connect(timerdev,SIGNAL(timeout()),this,SLOT(readDev()));

    return 0;
}
unsigned char QPanel::Hex2Dec(unsigned char ch)
{
    if(ch>='0' && ch<='9')
    {
        return ch-'0';
    }
    else if(ch>='A' && ch<='Z')
    {
        return ch-'A'+10;
    }
    else if(ch>='a' && ch<='z')
    {
        return ch-'a'+10;
    }

    return ch;
}

void QPanel::readDev(void)
{
    QFile f("/dev/panel");
    if(!f.exists())
        return;
    if(!f.open(QIODevice::ReadOnly))
        return;
    QByteArray dat=f.readAll();
    unsigned char *data=(unsigned char *)dat.data();
    unsigned long len=dat.length();
    if(len<=0)
        return;

    unsigned long i=0;
    int x=0,y=0,n=0;
    unsigned char buff[64];


    unsigned char *p=data;


    unsigned char sw=0;
    bool find=false;
    for(i=0;i<len;i++)
    {
        if(*p==0xA5 && *(p+1)==0x15)
        {
            find=true;
            sw=1;
            break;
        }
        if(strncmp((char*)p,"lcd",3)==0)
        {
            find=true;
            sw=2;
            break;
        }
        if(strncmp((char*)p,"led",3)==0)
        {
            find=true;
            sw=3;
            break;
        }
        if(strncmp((char*)p,"bz",3)==0)
        {
            find=true;
            sw=4;
            break;
        }

    }
    if(find==false)
        return;
    unsigned long cnt=0;
    len=len-i;
    data=p;
    memset(buff,0,64);

    //qDebug()<<"sw"<<sw;
    switch (sw) {
    case 1:
    {
        while(len>0)
        {

            cnt=38;
            if(cnt>len)
                return;
            this->writePort(p,cnt);
            len=len-cnt;
            p=p+cnt;
            usleep(10000);
        }
    }
        break;
    case 2://lcd
    {
        p=p+3;
        unsigned char cmd=*p;
        switch(cmd)
        {
          case '0':
            //lcd0xyn"str"
             x=Hex2Dec(*(p+1));
             y=Hex2Dec(*(p+2));
             n=Hex2Dec(*(p+3));
            x=x/2;
            //qDebug()<<x<<y<<n;
            len=this->procotol->getpLcd(buff,x,y,(char*)p+4,n);
            this->writePort(buff,len);
            break;
        case '1':
            //lcd1yn"str"
            y=Hex2Dec(*(p+1));
            n=Hex2Dec(*(p+2));
            //qDebug()<<y<<n;
            len=this->procotol->getpLcdLine(buff,y,(char*)p+3,n);
            this->writePort(buff,len);
            break;
        case '2':
             //lcd2'full'
            len=this->procotol->getpLcdFull(buff,*(p+1)?1:0);
            this->writePort(buff,len);
            break;
        case '3':
             //lcd3y'full'
            y=Hex2Dec(*(p+1));
            len=this->procotol->getpLcdFullLine(buff,y,*(p+2)?1:0);
            this->writePort(buff,len);
            break;
        case '4':
            //lcd4n/o'pos'
             x=Hex2Dec(*(p+1));//on/off
             n=Hex2Dec(*(p+2));//pos
            len=this->procotol->getpLcdPos(buff,x,n);
            this->writePort(buff,len);
            break;
        }
    }
        break;
    case 3:
    {//led
        p=p+3;
        x=Hex2Dec(*(p));//on/off
        n=Hex2Dec(*(p+1));//idx
        len=this->procotol->getpLed(buff,x,n);
        this->writePort(buff,len);
    }
        break;
    case 4:
    {//bz
        p=p+2;
        x=Hex2Dec(*(p));//on/off
        n=Hex2Dec(*(p+1));//idx
        len=this->procotol->getpBz(buff,x,n);
        this->writePort(buff,len);
    }
        break;
    default:
        break;
    }


}

bool QPanel::writePort(unsigned char *data,int len)
{
    int ret=port->write((char*)data,len);
    if(ret<0)
    {
        qDebug()<<"err:write"<<ret;
        return false;
    }
    usleep(200000);
//    printf("write %d:",len);
//    for(int i=0;i<len;i++)
//    {
//       printf("%x ",*data);
//       data++;

//    }
//    printf("\r\n");
    return true;
}

void QPanel::readPort(void)
{
//????
 //??????
    char data[20];
    bool head_flg=false;
    int i, len=port->bytesAvailable();

    if(len<10)
        return;
    port->read(data,10);
    unsigned char *pdat=(unsigned char *)data;
    unsigned char *p=pdat;

//    printf("read %d:",10);
//    for(i=0;i<10;i++)
//    {
//       printf("%x ",*p);
//       p++;

//    }
//    printf("\r\n");
//    p=pdat;

    for(i=0;i<10;i++)
    {
        if(*p==0xA5 && *(p+1)==0x15)
        {
            head_flg=true;
            break;
        }
        p++;
    }

    if(head_flg==false)
        return;
    if(i!=0)
    {
        disconnect(timer,SIGNAL(timeout()),this,SLOT(readPort()));
        while(port->bytesAvailable()<i)
            usleep(1000);

        port->read(&data[10],i);
        connect(timer,SIGNAL(timeout()),this,SLOT(readPort()));

    }


    pdat=p;
    p=p+2;
    if((*(p+1)+3+i>10))
        return;//?????
    len=*(p+1);

    switch(*p)
    {
        case 0x10:
        {//key
            QFile f("/dev/panel_key");
            p=p+2;
            if(f.exists())
            {
                if(f.open(QIODevice::WriteOnly))
                {

                    f.write((char*)p,1);
                    f.close();
                }
            }
#define DEBUG_0
#ifdef DEBUG_0
                    printf(" mpanel key ...\n\r");
#endif
            //qDebug()<<"key:"<<*p;

        }break;
        case 0x11:
        {//report
            p=p+2;

            if(*p!=0x9A)
            {
                qDebug()<<"err panel:"<<(*p==0x98?"head":"sum");
            }


        }break;
    }

}
