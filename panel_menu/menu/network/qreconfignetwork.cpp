#include "qreconfignetwork.h"


/*
 *     char * hw_itfc[HW_INTERFACE_COUNT]={
        (char*)"wf111(def)",
        (char*)"wl1271",
        (char*)"rtl8189",
        (char*)"usb(rt5370)",
        (char*)"eth0(rj45)",
    };
    */

 QInterfaceInfo::QInterfaceInfo(QObject *parent) :
     QObject(parent)
 {

 }

QReconfigNetwork::QReconfigNetwork(QObject *parent) :
    QObject(parent)
{
    info[0].interface="wf111";
    info[0].drv.append("/lib/modules/3.2.0/extra/unifi_sdio.ko");
    info[0].mac=false;
    info[0].name="wlan0";

    info[1].interface="wl1271";
    info[1].drv.append("/lib/modules/3.2.0/kernel/drivers/wifi/wl12xx.ko");
    info[1].drv.append("/lib/modules/3.2.0/kernel/drivers/wifi/wl12xx_sdio.ko");
    info[1].mac=true;
    info[1].name="wlan0";

    info[2].interface="rtl8189";
    info[2].drv.append("/lib/modules/3.2.0/kernel/drivers/wifi/wlan.ko");
    info[2].mac=false;
    info[2].name="ra0";

    info[3].interface="usb";
    info[3].drv.append("/lib/modules/3.2.0/kernel/drivers/wifi/rt5370sta.ko");
    info[3].mac=false;
    info[3].name="usb0";

    info[4].interface="eth0";
    info[4].mac=false;
    info[4].name="eth0";
    this->dhcp=true;

    this->curIf=&this->info[0];
}

void QReconfigNetwork::configNetwork(void)
{
    QFile file(":/file/network");
    file.open(QIODevice::ReadOnly);
    if(!file.isOpen())
        return;

    system("rm -f /etc/init.d/wlan0");

    QFile f_dst("/etc/init.d/wlan0");
    f_dst.open(QIODevice::ReadWrite);
    if(!f_dst.isOpen())
        return;
    have=true;
    while(!file.atEnd())
    {
        QByteArray r=file.readLine();
        if(!have)
        {
            if(r.contains("%endif%"))
                have=true;
            continue;
        }

        if(r.contains("%if DRV0%"))
        {
            if(this->curIf->drv.count()==0)
            {
                    have=false;
            }
            continue;
        }
        if(r.contains("%if DRV1%"))
        {
            if(this->curIf->drv.count()<2)
            {
                    have=false;
            }
            continue;
        }
        if(r.contains("%if mac%"))
        {
            if(!this->curIf->mac || this->macaddr.isEmpty())
            {
                    have=false;
            }
            continue;
        }

        if(r.contains("%if wlan%"))
        {
            if(this->curIf->name=="eth0")
            {
                    have=false;
            }
            continue;
        }

        if(r.contains("%if dhcp%"))
        {
            if(!this->dhcp)
            {
                    have=false;
            }
            continue;
        }

        if(r.contains("%if ipaddr%"))
        {
            if(this->dhcp)
            {
                    have=false;
            }
            continue;
        }

        if(r.contains("%endif%"))
            continue;

        if(r.contains("%DRV0%"))
        {
            int idx=r.indexOf('%');
            r=r.remove(idx,6);
            if(this->curIf->drv.count())
             r=r.insert(idx,this->curIf->drv.at(0));

        }

        if(r.contains("%DRV1%"))
        {
            int idx=r.indexOf('%');
            r=r.remove(idx,6);
            if(this->curIf->drv.count()>1)
             r=r.insert(idx,this->curIf->drv.at(1));
        }

        if(r.contains("%ifname%"))
        {
            int idx=r.indexOf('%');
            r=r.remove(idx,8);
            r=r.insert(idx,this->curIf->name);
        }
        if(r.contains("%mac%"))
        {
            int idx=r.indexOf('%');
            r=r.remove(idx,5);
            r=r.insert(idx,this->macaddr);
        }

        if(r.contains("%essidname%"))
        {
            int idx=r.indexOf('%');
            r=r.remove(idx,11);
            r=r.insert(idx,this->essid);
        }
        if(r.contains("%key%"))
        {
            int idx=r.indexOf('%');
            r=r.remove(idx,5);
            if(!this->pwd.isEmpty())
                r=r.insert(idx,QString("key s:%1").arg(this->pwd));
        }

        if(r.contains("%ipaddr%"))
        {
            int idx=r.indexOf('%');
            r=r.remove(idx,8);
            r=r.insert(idx,this->addr.toString());
        }

    f_dst.write(r);
    }
    file.close();
    f_dst.close();
    system("chmod +x /etc/init.d/wlan0");
}

void QReconfigNetwork::setCurrentEssid(QString essid,QString pwd)
{
    this->essid=essid;
    this->pwd=pwd;
}

void QReconfigNetwork::setCurrentIf(QString name,QString macaddr)
{
    for(int i=0;i<5;i++)
    {
        if(this->info[i].interface==name){
            this->curIf=&this->info[i];
            break;
        }
    }
    this->macaddr=macaddr;
}

void QReconfigNetwork::setCurrentDhcp(QString dhcp,QString addr)
{
    if(dhcp.isEmpty())
        this->dhcp=true;
    else
    {
        if(dhcp=="true")
            this->dhcp=true;
        else
            this->dhcp=false;
    }
    if(this->dhcp)
        this->addr.clear();
    else
        this->addr=QHostAddress(addr);
}

void QReconfigNetwork::getConfigFile(void)
{
    QSettings cfg(MENU_CONFIG_FILE,QSettings::IniFormat);
    this->setCurrentIf(cfg.value("network/interface").toString(),cfg.value("network/mac").toString());
    this->setCurrentEssid(cfg.value("network/essid").toString(),cfg.value("network/password").toString());
    this->setCurrentDhcp(cfg.value("network/dhcp").toString(),cfg.value("network/ip").toString());
}

void QReconfigNetwork::generateConfig(void)
{
    this->getConfigFile();
    this->configNetwork();
}
