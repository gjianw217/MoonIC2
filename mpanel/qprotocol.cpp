#include "qprotocol.h"


#define DATA_BUFF_LEN 34

QProtocol::QProtocol(QObject *parent) :
    QObject(parent)
{
}

int QProtocol::getpLcd(unsigned char *data,unsigned char x,unsigned char y,char *dis,int len)
{
    unsigned char *p=data,*psum=0;
    getpHead(p);
    p=p+2;
    psum=p+DATA_BUFF_LEN;
    *p=0x01;//type lcd
    p=p+1;

    len=len%(DATA_BUFF_LEN+1);
    *p=3+len;//len
    *(p+1)=0x01;//cmd
    *(p+2)=x;//arg0
    *(p+3)=y;//arg1
    p=p+4;
    //memset(p,0,DATA_BUFF_LEN);
    memcpy(p,dis,len);//arg2
    p=psum;
    unsigned long sum=this->getpSum(data,p-data);
    *p=sum&0xFF;
    *(p+1)=(sum>>8)&0xff;

    p=p+2;
    return p-data;
}


int QProtocol::getpLcdLine(unsigned char *data,unsigned char y,char *dis,int len)
{
    unsigned char *p=data,*psum=0;
    getpHead(p);
    p=p+2;
    psum=p+DATA_BUFF_LEN;
    *p=0x01;//type lcd
    p=p+1;

    len=len%(DATA_BUFF_LEN+1);
    *p=2+len;
    *(p+1)=0x02;
    *(p+2)=y;
    p=p+3;
    //memset(p,0,DATA_BUFF_LEN);
    memcpy(p,dis,len);
    p=psum;
    unsigned long sum=this->getpSum(data,p-data);
    *p=sum&0xFF;
    *(p+1)=(sum>>8)&0xff;

    p=p+2;
    return p-data;
}

int QProtocol::getpLcdClear(unsigned char *data)
{
    return getpLcdFull(data,0);
}

int QProtocol::getpLcdClearLine(unsigned char *data,unsigned char y)
{
    return getpLcdFullLine(data,y,0);

}

int QProtocol::getpLcdFull(unsigned char *data,unsigned char full)
{
    unsigned char *p=data;
    getpHead(p);
    p=p+2;
    unsigned char *psum=p+DATA_BUFF_LEN;
    *p=0x01;//type lcd
    p=p+1;

    *p=2;
    *(p+1)=0x03;
    *(p+2)=full;
    //memset(p,0,DATA_BUFF_LEN);
    p=psum;
    unsigned long sum=this->getpSum(data,p-data);
    *p=sum&0xFF;
    *(p+1)=(sum>>8)&0xff;

    p=p+2;
    return p-data;
}

int QProtocol::getpLcdFullLine(unsigned char *data,unsigned char y,unsigned char full)
{
    unsigned char *p=data;
    getpHead(p);
    p=p+2;
    unsigned char *psum=p+DATA_BUFF_LEN;
    *p=0x01;//type lcd
    p=p+1;

    *p=2;//len
    *(p+1)=0x04;//cmd
    *(p+2)=y;//arg0
    *(p+3)=full;//arg1
    //memset(p,0,DATA_BUFF_LEN);
    p=psum;
    unsigned long sum=this->getpSum(data,p-data);
    *p=sum&0xFF;
    *(p+1)=(sum>>8)&0xff;

    p=p+2;
    return p-data;

}


int QProtocol::getpLcdPos(unsigned char *data,unsigned char on1off0,unsigned char pos)
{
    unsigned char *p=data,*psum=0;
    getpHead(p);
    p=p+2;
    psum=p+DATA_BUFF_LEN;
    *p=0x01;//type lcd
    p=p+1;

    if(on1off0)
    {
        *p=2;//len
        *(p+1)=0x05;//cmd
        *(p+2)=pos;//arg0
    }
    else
    {
        *p=1;//len
        *(p+1)=0x06;//cmd
    }
    p=psum;
    unsigned long sum=this->getpSum(data,p-data);
    *p=sum&0xFF;
    *(p+1)=(sum>>8)&0xff;

    p=p+2;
    return p-data;
}

int QProtocol::getpLed(unsigned char *data,unsigned char on1off0,unsigned char index)
{
    unsigned char *p=data;
    getpHead(p);
    p=p+2;

    *p=0x02;//type led
    *(p+1)=2;//len
    *(p+2)=on1off0;//arg1
    *(p+3)=index;//arg2
    //memset(p,0,DATA_BUFF_LEN);
    p=p+DATA_BUFF_LEN;
    unsigned long sum=this->getpSum(data,p-data);
    *p=sum&0xFF;
    *(p+1)=(sum>>8)&0xff;

    p=p+2;
    return p-data;
}

int QProtocol::getpBz(unsigned char *data,unsigned char on1off0,unsigned char hz)
{
    unsigned char *p=data;
    getpHead(p);
    p=p+2;

    *p=0x03;//type bz
    *(p+1)=1;//len
    *(p+2)=on1off0;//arg1
    *(p+3)=hz;//arg2

    p=p+DATA_BUFF_LEN;
    unsigned long sum=this->getpSum(data,p-data);
    *p=sum&0xFF;
    *(p+1)=(sum>>8)&0xff;

    p=p+2;
    return p-data;

}

void QProtocol::getpHead(unsigned char *data)
{
    memset(data,0,DATA_BUFF_LEN+8);
    *data=0xA5;
    *(data+1)=0x15;

}

unsigned long QProtocol::getpSum(unsigned char *data,int len)
{
    unsigned long sum=0;
    for(int i=0;i<len;i++)
    {
        sum=sum+data[i];
    }

    return sum;
}
