#include "qpaneldbus.h"

QPanelDbus::QPanelDbus(QPanel *panel,QObject *parent):
    QObject(parent)
{
    this->panel=panel;
}

bool QPanelDbus::setLed(uchar on1off0,uchar index)
{
    uchar buff[64];
    int len=0;
    len=this->panel->procotol->getpLed(buff,on1off0,index);
    return this->panel->writePort(buff,len);

}

bool QPanelDbus::setBz(uchar on1off0,uchar hz)
{
    uchar buff[64];
    int len=0;
    len=this->panel->procotol->getpBz(buff,on1off0,hz);
    return this->panel->writePort(buff,len);
}

bool QPanelDbus::setLcd(uchar x,uchar y,QByteArray data)
{
    uchar buff[64];
    int len=0,cnt=data.length()>200?200:data.length();
    x=x/2;
    len=this->panel->procotol->getpLcd(buff,x,y,data.data(),cnt);
    return this->panel->writePort(buff,len);
}

bool QPanelDbus::setLcdLine(uchar y,QByteArray data)
{
    uchar buff[64];
    int len=0,cnt=data.length()>200?200:data.length();
    len=this->panel->procotol->getpLcdLine(buff,y,data.data(),cnt);
    return this->panel->writePort(buff,len);
}

bool QPanelDbus::setLcdClear(void)
{
    uchar buff[64];
    int len=0;
    len=this->panel->procotol->getpLcdClear(buff);
    return this->panel->writePort(buff,len);
}
bool QPanelDbus::setLcdPos(uchar on1off0,uchar pos)
{
    uchar buff[64];
    int len=0;
    len=this->panel->procotol->getpLcdPos(buff,on1off0,pos);
    return this->panel->writePort(buff,len);
}


bool QPanelDbus::setLcdClearLine(uchar y)
{
    uchar buff[64];
    int len=0;
    len=this->panel->procotol->getpLcdClearLine(buff,y);
    return this->panel->writePort(buff,len);
}

bool QPanelDbus::setLcdFull(uchar data)
{
    uchar buff[64];
    int len=0;
    len=this->panel->procotol->getpLcdFull(buff,data);
    return this->panel->writePort(buff,len);
}

bool QPanelDbus::setLcdFullLine(uchar y,uchar data)
{
    uchar buff[64];
    int len=0;
    len=this->panel->procotol->getpLcdFullLine(buff,y,data);
    return this->panel->writePort(buff,len);
}

