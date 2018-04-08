#include "qshowsnmenu.h"

QShowSnMenu::QShowSnMenu(QPanelInterface *iface,QMenu *parentMenu) :
    QMenu(iface,"sn",true,parentMenu,parentMenu)
{
}
void QShowSnMenu::domenu(int keycode,int val)
{
    Q_UNUSED(val);
    switch (keycode) {
    case Qt::Key_Enter:
         keyBackMenu();
        break;
    case Qt::Key_Right:
    case Qt::Key_Left:
        break;
    case Qt::Key_Back:
        keyBackMenu();
        break;
    default:
        break;
    }
}

void QShowSnMenu::showFunc(void)
{
    //getSn;
    QString sn=sn2string((unsigned char*)getSn().data());
    if(sn.size()>29)
        sn=sn.remove(29,sn.size()-29);

    QString sn2;
    if(sn.size()>13)
    {
        sn2=sn.right(sn.size()-13);
        sn=sn.remove(13,sn.size()-13);
    }

    iface->setLcdLine(0,QString("sn:%1").arg(sn).toLatin1());
    iface->setLcdLine(1,sn2.toLatin1());
}

unsigned char  QShowSnMenu::ch2hex(unsigned char d)
{
    if(d<=9)
    {
        return '0'+d;
    }
    else
     {
        return 'A'+d-10;
    }
}

QString QShowSnMenu::int2hex(quint32 d)
{
    unsigned char buff[9];
    buff[8]=0;
    for(int i=0;i<8;i++)
    {
        buff[7-i]=ch2hex(d&0xf);
        d=d>>4;
    }
    QString ret=QString((char*)buff);
    return ret;
}

QString QShowSnMenu::sn2string(unsigned char buff[])
{
    unsigned char sn_buff[16];
    if(buff==(unsigned char*)"unknow")
        return "unknow";

    memset(sn_buff,0,16);
    memcpy(sn_buff,buff,5);
    sn_buff[5]=((buff[5]>>4)&0xF)+'0';
    sn_buff[6]=(buff[5]&0xF)+'0';

    sn_buff[7]=((buff[6]>>4)&0xF)+'0';
    sn_buff[8]=(buff[6]&0xF)+'0';

    sn_buff[9]=((buff[7]>>4)&0xF)+'0';
    sn_buff[10]=(buff[7]&0xF)+'0';

    unsigned long sn1=buff[11]|(buff[10]<<8)|(buff[9]<<16)|(buff[8]<<24);
    unsigned long sn0=buff[15]|(buff[14]<<8)|(buff[13]<<16)|(buff[12]<<24);
    QString ssn0=int2hex(sn0);
    QString ssn1=int2hex(sn1);
    //qDebug()<<QByteArray((char*)buff,16).toHex();
    QString sn= QString("%1%2%3").arg((char*)sn_buff).arg(ssn1).arg(ssn0);

    return sn;
}


QByteArray QShowSnMenu::getSn(void)
{
    unsigned char sn_buff[17];
    memset(sn_buff,0,17);

    QByteArray sn;
    QFile fsn(FILE_SN);

    fsn.open(QIODevice::ReadOnly);
    if(fsn.isOpen())
    {
        fsn.read((char*)sn_buff,16);

        fsn.close();
        sn=QByteArray((char*)sn_buff,16);
        return sn;
    }

    return "unknow";
}
