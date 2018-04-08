#include "qpaneldbus.h"

QPanelDbus::QPanelDbus(QPanel *panel,QObject *parent):
    QObject(parent)
{
    this->panel=panel;
}

bool QPanelDbus::setLed(unsigned char on1off0,unsigned char index)
{
    unsigned char buff[64];
    int len=0;
    len=this->panel->procotol->getpLed(buff,on1off0,index);
    return this->panel->writePort(buff,len);

}

bool QPanelDbus::setBz(unsigned char on1off0,unsigned char hz)
{
    unsigned char buff[64];
    int len=0;
    len=this->panel->procotol->getpBz(buff,on1off0,hz);
    return this->panel->writePort(buff,len);
}

bool QPanelDbus::setLcd(unsigned char x,unsigned char y,QByteArray data)
{
    unsigned char buff[64];
    int len=0,cnt=data.length()>200?200:data.length();
    x=x/2;
    len=this->panel->procotol->getpLcd(buff,x,y,data.data(),cnt);
    return this->panel->writePort(buff,len);
}

bool QPanelDbus::setLcdLine(unsigned char y,QByteArray data)
{
    unsigned char buff[64];
    int len=0,cnt=data.length()>200?200:data.length();
    len=this->panel->procotol->getpLcdLine(buff,y,data.data(),cnt);
    return this->panel->writePort(buff,len);
}

bool QPanelDbus::setLcdClear(void)
{
    unsigned char buff[64];
    int len=0;
    len=this->panel->procotol->getpLcdClear(buff);
    return this->panel->writePort(buff,len);
}
bool QPanelDbus::setLcdPos(unsigned char on1off0,unsigned char pos)
{
    unsigned char buff[64];
    int len=0;
    len=this->panel->procotol->getpLcdPos(buff,on1off0,pos);
    return this->panel->writePort(buff,len);
}


bool QPanelDbus::setLcdClearLine(unsigned char y)
{
    unsigned char buff[64];
    int len=0;
    len=this->panel->procotol->getpLcdClearLine(buff,y);
    return this->panel->writePort(buff,len);
}

bool QPanelDbus::setLcdFull(unsigned char data)
{
    unsigned char buff[64];
    int len=0;
    len=this->panel->procotol->getpLcdFull(buff,data);
    return this->panel->writePort(buff,len);
}

bool QPanelDbus::setLcdFullLine(unsigned char y,unsigned char data)
{
    unsigned char buff[64];
    int len=0;
    len=this->panel->procotol->getpLcdFullLine(buff,y,data);
    return this->panel->writePort(buff,len);
}

