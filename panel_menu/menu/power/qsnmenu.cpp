#include "qsnmenu.h"

QSnMenu::QSnMenu(QPanelInterface *iface,QMenu *parentMenu) :
    QMenu(iface,"reload sn",true,parentMenu,parentMenu)
{
    reload=false;
}

void QSnMenu::showFunc(void)
{
    iface->setLcdLine(0,"reload sn?");
    if(!reload)
        iface->setLcdLine(1,"  >no    yes   ");
    else
        iface->setLcdLine(1,"  no    >yes   ");

}

void QSnMenu::reloadSn(void)
{
    QByteArray sn;
    QFile fsn(FILE_SN);
    QFile fsnrd(FILE_SN_READ);

    fsnrd.open(QIODevice::ReadOnly);
    if(fsnrd.isOpen())
    {
        fsnrd.close();
        system(QString("rm -f %1").arg(FILE_SN_READ).toLatin1().data());
    }

    fsn.open(QIODevice::ReadOnly);
    if(fsn.isOpen())
    {
        fsn.close();
        system(QString("rm -f %1").arg(FILE_SN).toLatin1().data());
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
        unsigned char sn_buff[17];
        memset(sn_buff,0,17);
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
            sn=QByteArray((char*)sn_buff);
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
    }
}

void QSnMenu::domenu(int keycode,int val)
{
    Q_UNUSED(val);
    switch (keycode) {
    case Qt::Key_Enter:
            if(reload==true)
            {
                //iface->setLcdClear();
                //iface->setLcdPos(0,0);
                //QProcess reboot;
                //reboot.execute("/sbin/reboot");
                reloadSn();
            }
            else
            {
                keyBackMenu();
                reload=false;
            }
        break;
    case Qt::Key_Right:
        reload=!reload;
        this->show();
        break;
    case Qt::Key_Left:
        reload=!reload;
        this->show();
        break;
    case Qt::Key_Back:
        reload=false;
        keyBackMenu();
        break;
    default:
        break;
    }
}
