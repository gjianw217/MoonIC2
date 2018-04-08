#include "qbackthread.h"
#include <QSettings>
#include <QString>
//#include "qmenu.h"
#include <QFile>
#include <QNetworkInterface>
#include <QProcess>

QBackThread::QBackThread(QObject *parent) :
    QThread(parent)
{

}
unsigned long QBackThread::spi_swap(unsigned long v)
{
    unsigned long t=0;
    t=(v&0xff)<<24;
    t=t|(v&0xff00)<<8;
    t=t|(v&0xff0000)>>8;
    t=t|(v&0xff000000)>>24;
    return t;
}

int QBackThread::spi_pkt(unsigned long *buff,bool rw,int type,unsigned short addr)
{
#define SPI_W   0xa5
#define SPI_R   0x98

    if(rw)
    {
        *buff=((SPI_W&0xFF)<<24)|((type&0xFF)<<16)|(addr&0xFFFF);
    }
    else
    {
        *buff=((SPI_R&0xFF)<<24)|((type&0xFF)<<16)|(addr&0xFFFF);
    }
    *buff=spi_swap(*buff);
    *(buff+1)=0xFFFFFFFF;
    return 2;
}

void QBackThread::cheackCarNumber(void)
{
    #define CLIENT_VOLTAGE_DIR	"/sys/bus/iio/devices/iio:device0/"
    #define FILE_VOLTAGE_24		"in_voltage1_raw"
    #define FILE_VOLTAGE_BAT	"in_voltage0_raw"
    #define FPGA_SPI_DEVICE_NAME  "/dev/fpga_spi2.0"


    int number=0;
    bool voltage=false;

    QFile fil(QString("%1%2").arg(CLIENT_VOLTAGE_DIR).arg(FILE_VOLTAGE_24));
    if(!fil.exists())
        return ;
    if(!fil.open(QIODevice::ReadOnly))
        return ;
    QFile spi(FPGA_SPI_DEVICE_NAME);
    if(!spi.exists())
        return;
    if(!spi.open(QIODevice::ReadWrite))
         return ;

    QSettings cfg(MENU_CONFIG_FILE,QSettings::IniFormat);
    QString num=cfg.value("info/number").toString();

    if(num.isEmpty())
        number=0;
    else{
        number=num.toInt();
        number=number%16;
    }



    char buff[17];
    memset(buff,0,17);

    fil.readLine(buff,16);
    fil.close();
    QString v(buff);
    int value=v.toInt();
    float v24=(float)value*0.006902;
    if(v24>18)
    {
        voltage=true;
    }
    unsigned long spi_buff[16];

    spi_pkt(spi_buff,false,2,1);

    spi.read((char*)spi_buff,(2+1)*4);
    if(((spi_buff[2]&0xF0)==0xa0)&& voltage)
    {
        if(((spi_buff[2]&0x0f)==number))
        {
            spi.close();
            return;
        }
    }
    spi_pkt(spi_buff,true,2,1);
    spi_buff[2]=number&0x0F;
    if(voltage)
    {
        spi_buff[2]=spi_buff[2]|0xa0;
    }

    spi_buff[2]=spi_swap(spi_buff[2]);
    spi.write((char*)spi_buff,(2+1)*4);
    spi.close();

}

void QBackThread::network(void)
{
    QHostAddress addr;
    bool find=false;

   QList<QNetworkInterface> iface= QNetworkInterface::allInterfaces();

   foreach(QNetworkInterface ifn,iface)
   {
       if(ifn.flags().testFlag(QNetworkInterface::IsUp)&&
               ifn.flags().testFlag(QNetworkInterface::IsRunning)&&
               !ifn.flags().testFlag(QNetworkInterface::IsLoopBack))
       {
           if(ifn.name()=="wlan0")
           {
                QList<QNetworkAddressEntry> entryList = ifn.addressEntries();
                foreach(QNetworkAddressEntry entry,entryList)
                {//遍历每一个IP地址条目


                    if(entry.ip().protocol()==QAbstractSocket::IPv4Protocol)
                       {
                        //qDebug()<<entry.ip().toString();
                          addr=entry.ip();
                          find=true;
                          break;
                       }
                }
           }
       }
       else{


       }

   }

   if(find==true)
   {
       QProcess p;
       unsigned long ip=addr.toIPv4Address();

       ip=(ip&0xFFFFFF00)|0x000001;
      // qDebug()<<QHostAddress(ip).toString();
       if(p.execute("ping", QStringList() << QHostAddress(ip).toString()<<"-c"<<"1"))

       {
           if(p.execute("ping", QStringList() << QHostAddress(ip).toString()<<"-c"<<"1"))

           {
               if(p.execute("ping", QStringList() << QHostAddress(ip).toString()<<"-c"<<"1"))

               {
                   p.execute("/mooncell/bin/lcd",QStringList()<<"-clr");
                   p.execute("/mooncell/bin/lcd",QStringList()<<"-l"<<"0"<<"wifi err ping");
                   //p.close();
                   sleep(1);
                   p.execute("/etc/init.d/wlan0", QStringList()<<"restart");
                   p.waitForFinished();
                   p.close();
               }
           }

       }
   }
   else
   {
       QProcess p;
       p.execute("/mooncell/bin/lcd",QStringList()<<"-clr");
       p.execute("/mooncell/bin/lcd",QStringList()<<"-l"<<"0"<<"wifi err find");
       sleep(2);
       p.execute("/etc/init.d/wlan0", QStringList()<<"restart");
       p.waitForFinished();
       p.close();
       //p.execute("killall",QStringList()<<"panel_menu");
      // p.start("/mooncell/bin/panel_menu");

   }



}

void QBackThread::run(void)
{

    while(true)
    {

        cheackCarNumber();
        network();
        sleep(5);

    }

}
