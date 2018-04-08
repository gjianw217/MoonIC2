#include "qnetdrivermenu.h"
#include "qreconfignetwork.h"
#define HW_INTERFACE_COUNT  5

QNetDriverMenu::QNetDriverMenu(QPanelInterface *iface,QMenu *parentMenu) :
    QMenu(iface,"interface(HW)",true,parentMenu,parentMenu)
{
    itfc_idx=0;
    func=0;
    yes=0;
}

void QNetDriverMenu::showFunc(void)
{

    char * hw_itfc[HW_INTERFACE_COUNT]={
        (char*)"wf111(def)",
        (char*)"wl1271",
        (char*)"rtl8189",
        (char*)"usb(rt5370)",
        (char*)"eth0(rj45)",
    };

    itfc_idx=itfc_idx%HW_INTERFACE_COUNT;
    int next_idx=(itfc_idx+1)%HW_INTERFACE_COUNT;

    switch(func)
    {
    case 0x00:
        iface->setLcdLine(0,QString(">%1").arg(hw_itfc[itfc_idx]).toLatin1());
        iface->setLcdLine(1,QString(" %1").arg(hw_itfc[next_idx]).toLatin1());
        break;
    case 0x01:
    {
        switch(yes)
        {
        case 0x00:
            iface->setLcdLine(0,">cancel shutdown");
            iface->setLcdLine(1," reboot reload ");
            break;
        case 0x01:
            iface->setLcdLine(0,"cancel >shutdown");
            iface->setLcdLine(1,"reboot  reload ");
            break;
        case 0x02:
            iface->setLcdLine(0," cancel shutdown");
            iface->setLcdLine(1,">reboot reload ");
            break;
        case 0x03:
            iface->setLcdLine(0,"cancel  shutdown");
            iface->setLcdLine(1,"reboot >reload ");
            break;
        }


    } break;

    }




}

void QNetDriverMenu::lnInterface(void)
{
    QProcess proc;
    QStringList arg;

    QSettings cfg(MENU_CONFIG_FILE,QSettings::IniFormat);
    cfg.beginGroup("network");


    //arg<<"-f"<<"/etc/init.d/wlan0";
    //proc.execute("/bin/rm",arg);
    //arg.clear();
   // arg<<"-s";
    switch (itfc_idx) {
    case 0x00:
        //arg<<"/etc/init.d/wlan_wf111";
        cfg.setValue("interface","wf111");
        break;
    case 0x01:
        //arg<<"/etc/init.d/wlan_wl1271";
        cfg.setValue("interface","wl1271");
        break;
    case 0x02:
        //arg<<"/etc/init.d/wlan_rtl8189";
        cfg.setValue("interface","rtl8189");
        break;
    case 0x03:
        //arg<<"/etc/init.d/wlanusb";
        cfg.setValue("interface","usb");
        break;
    case 0x04:
        //arg<<"/etc/init.d/eth0";
        cfg.setValue("interface","eth0");
        break;
    default:
        //arg<<"/etc/init.d/wlan_wf111";
        cfg.setValue("interface","wf111");
        break;
    }

    cfg.endGroup();


    //arg<<"/etc/init.d/wlan0";
    //proc.execute("/bin/ln",arg);



}

void QNetDriverMenu::domenu(int keycode,int val)
{
    Q_UNUSED(val);

    switch (keycode) {
    case Qt::Key_Enter:
    {
        switch(func)
        {
            case 0x00:
            {
                func=0x01;
                this->show();
            }break;
            case 0x01:
            {
                QProcess proc;
                QStringList arg;
                switch(yes)
                {
                    case 0x00://cancel
                    {
                        yes=0;
                        func=0;
                        this->show();
                        //itfc_idx=0;
                        //keyBackMenu();
                    }
                    break;
                    case 0x01://shutdown
                    {
                        lnInterface();
                        iface->setLcdClear();
                        iface->setLcdPos(0,0);
                        QReconfigNetwork rn(this);
                        rn.generateConfig();
                        arg<<"-h"<<"-f"<<"now";
                        proc.execute("/sbin/shutdown",arg);
                    }
                    break;
                    case 0x02://reboot
                    {
                        lnInterface();
                        iface->setLcdClear();
                        iface->setLcdPos(0,0);
                        QReconfigNetwork rn(this);
                        rn.generateConfig();
                        proc.execute("/sbin/reboot");
                    }
                    break;
                    case 0x03://reload
                    {
                        arg<<"stop";
                        proc.execute("/etc/init.d/wlan0",arg);
                        lnInterface();
                        QReconfigNetwork rn(this);
                        rn.generateConfig();
                        arg.clear();
                        arg<<"start";
                        proc.execute("/etc/init.d/wlan0",arg);
                        yes=0;
                        func=0;
                        itfc_idx=0;

                        keyBackMenu();
                    }
                    break;
                }

            }break;
        }


    }
        break;
    case Qt::Key_Right:
        switch(func)
        {
        case 0x00:
            itfc_idx=itfc_idx+1;
            itfc_idx=itfc_idx%HW_INTERFACE_COUNT;
            break;
        case 0x01:
            yes++;
            yes=yes%4;
            break;
        }


        this->show();
        break;
    case Qt::Key_Left:
        switch(func)
        {
        case 0x00:
            {
                if(itfc_idx==0)
                {
                    itfc_idx=HW_INTERFACE_COUNT-1;
                }
                else
                {
                    itfc_idx=itfc_idx-1;
                }
                itfc_idx=itfc_idx%HW_INTERFACE_COUNT;
            }
            break;
        case 0x01:
        {
            if(yes==0)
                yes=0x03;
            else
                yes--;
            yes=yes%4;
        }
            break;
        }
        this->show();
        break;
    case Qt::Key_Back:
        yes=0;
        func=0;
        itfc_idx=0;
        keyBackMenu();
        break;
    default:
        break;
    }
}
