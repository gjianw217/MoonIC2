#include "qdefaultmenu.h"

QDefaultMenu::QDefaultMenu(QPanelInterface *iface,QObject *parent) :
    QMenu(iface,"default",true,parent)
    //QMenu(iface,"default",false,parent)
{

    QMenu::currentMenu=this;//QMenu::currentMenu=defMenu;//这样仅需修改currentMenu
    setinfMenu=new QSettingMenu(iface,this,parent);


     //QBackMenu *back=new QBackMenu();

    new QBackMenu(iface,setinfMenu);
    new QNet2spiMenu(iface,setinfMenu);
    QNetWorkMenu    *network=new QNetWorkMenu(iface,setinfMenu);
    QPowerMenu      *power= new QPowerMenu(iface,setinfMenu);
    QInfomationMenu *info=new QInfomationMenu(iface,setinfMenu);



    new QBackMenu(iface,power);
    new QRebootMenu(iface,power);
    new QShutDownMenu(iface,power);
    new QSnMenu(iface,power);

    new QBackMenu(iface,info);
    new QNumberMenu(iface,info);
    new QVersionMenu(iface,info);
    new QMenuVerMenu(iface,info);
    new QShowSnMenu(iface,info);

    //network
    new QBackMenu(iface,network);
    new QReloadMenu(iface,network);
    new QNetDriverMenu(iface,network);
    new QEssidMenu(iface,network);
    new QDhcpMenu(iface,network);
    new QMacMenu(iface,network);
    //new QInputMenu(iface,setinfMenu);
    show();
}

void QDefaultMenu::showFunc(void)
{
    int number=0;
    unsigned long ip=getLoaclIpAddr();
    iface->setLcdPos(0,0);
    iface->setLcd(0,0,QString("ip:%1").arg(QHostAddress(ip).toString()).toLatin1());

    QByteArray sn=getSn();

    QString ssn=sn2string((unsigned char*)sn.data());
    if(ssn.size()>7)
        ssn=ssn.remove(0,ssn.size()-7);

    QSettings cfg(MENU_CONFIG_FILE,QSettings::IniFormat);
    QString num=cfg.value("info/number").toString();

    if(num.isEmpty())
      number=0;
    else
      number=num.toInt();
    number=number%16;

    iface->setLcd(0,1,QString("sn:%1 id:%2").arg(ssn).arg(number).toLatin1());
}

unsigned char  QDefaultMenu::ch2hex(unsigned char d)
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

QString QDefaultMenu::int2hex(quint32 d)
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

QString QDefaultMenu::sn2string(unsigned char buff[])
{
    unsigned char sn_buff[16];

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
    //qDebug()<<sn;

    return sn;
}


QByteArray QDefaultMenu::getSn(void)
{
   unsigned char sn_buff[64];
    memset(sn_buff,0,64);

   // unsigned char buff[64];
    QByteArray sn;

    QFile fsn(FILE_SN);
    fsn.open(QIODevice::ReadOnly);
    if(fsn.isOpen())
    {
        fsn.read((char*)sn_buff,16);
        fsn.close();
        sn=QByteArray((char*)sn_buff,16);
        //sn=QByteArray((char*)sn_buff,16);
        return sn;
    }
    else
    {
        QFile fsnrd(FILE_SN_READ);
        fsnrd.open(QIODevice::ReadOnly);
        if(fsnrd.isOpen())
        {
            fsnrd.close();
            return "unknow";
        }

        unsigned char buff[4096];
        int fd_spi=open(FLASH_SPI_DEVICE_NAME,O_RDWR);
        if(fd_spi<0)
        {
            fsnrd.open(QIODevice::ReadWrite);
            if(fsnrd.isOpen())
            {
                fsnrd.write("1");
                fsnrd.close();
            }
            return "unknow";
        }
        else
        {
            iface->setLcd(0,0,QString("init sn file").toLatin1());
            iface->setLcd(0,1,QString("reboot").toLatin1());

            int val=GPIO_FLASH_SPI;
            ioctl(fd_spi,FLASH_GPIO_MODE,&val);

            val=0;
            ioctl(fd_spi,FLASH_SPI_CONF,&val);

            val=1;
            ioctl(fd_spi,FLASH_SPI_CS1,&val);

            memset(buff,0,4096);
            ioctl(fd_spi,FLASH_SN_READ,buff);

            memcpy(sn_buff,buff,16);
            if(sn_buff[0]==0x00 || sn_buff[0]==0xFF||sn_buff[1]==0x00 || sn_buff[1]==0xFF)
            {
                fsnrd.open(QIODevice::ReadWrite);
                if(fsnrd.isOpen())
                {
                    fsnrd.write("1");
                    fsnrd.close();
                }
                sn="unknow";
            }
            else
            {
                sn=QByteArray((char*)sn_buff,16);
                fsn.open(QIODevice::ReadWrite);
                if(fsn.isOpen())
                {
                    fsn.write(sn);
                    fsn.write("\n",1);
                    fsn.close();
                }
            }
            close(fd_spi);
            sleep(2);
            iface->setLcdClear();
            system("reboot");
            sleep(100);
            return sn;
        }
    }
    return "unknow";
}

void QDefaultMenu::domenu(int keycode,int val)
{
    Q_UNUSED(val);
    switch (keycode) {
    case Qt::Key_Enter:
        if(this->firstMenu!=NULL && this->firstMenu!=QMenu::currentMenu)
        {
            emit currentMenuChange(QMenu::currentMenu,this->firstMenu);
            QMenu::currentMenu=this->firstMenu;
            this->selectMenu=this->firstMenu;
            this->selectMenu->show();
        }
        break;
    case Qt::Key_Right:
    case Qt::Key_Left:
        //QMenu::selectNext(this,false);
        break;
    default:
        show();
        break;
    }
}

unsigned long QDefaultMenu::getLoaclIpAddr(void)
{
    QList<QNetworkInterface> list = QNetworkInterface::allInterfaces();

       foreach(QNetworkInterface i, list) {
           if(i.name()=="wlan0")
           {
               QList<QNetworkAddressEntry> adre= i.addressEntries();
               if(!adre.isEmpty())
               {
                  QHostAddress addr=adre.at(0).ip();
                   return addr.toIPv4Address();
               }
           }
           else if(i.name()=="eth0")
           {
               QList<QNetworkAddressEntry> adre= i.addressEntries();
               if(!adre.isEmpty())
               {
                  QHostAddress addr=adre.at(0).ip();
                   return addr.toIPv4Address();
               }
           }
           else if(i.name()=="ra0")
           {
               QList<QNetworkAddressEntry> adre= i.addressEntries();
               if(!adre.isEmpty())
               {
                  QHostAddress addr=adre.at(0).ip();
                   return addr.toIPv4Address();
               }
           }
       }
       return 0;
}


