#include "qdhcpmenu.h"

QDhcpMenu::QDhcpMenu(QPanelInterface *iface,QMenu *parentMenu) :
    QMenu(iface,"set dhcp",true,parentMenu,parentMenu)
{
    QSettings cfg(MENU_CONFIG_FILE,QSettings::IniFormat);
    QString def=cfg.value("network/dhcp").toString();
    if(def.isEmpty())
        this->dhcp=true;
    else
    {
        if(def=="true")
            this->dhcp=true;
        else
            this->dhcp=false;
    }

    if(!dhcp)
        addr=cfg.value("network/ip").toString();
}


void QDhcpMenu::domenu(int keycode,int val)
{
    Q_UNUSED(val);
    switch (keycode) {
    case Qt::Key_Enter:
    {
        QSettings cfg(MENU_CONFIG_FILE,QSettings::IniFormat);
        cfg.beginGroup("network");
        if(this->dhcp)
        {
            cfg.setValue("dhcp","true");
        }
        else
        {
            cfg.setValue("dhcp","false");

        }
        cfg.endGroup();

        if(!this->dhcp)
        {
            iface->setLcdLine(0,"input ip");
            iface->setLcdClearLine(1);
            sleep(1);
            QInputMenu::change2Input(this,this->addr);
        }

    }
        break;
    case Qt::Key_Right:
        dhcp=!dhcp;
        this->show();
        break;
    case Qt::Key_Left:
        dhcp=!dhcp;
        this->show();
        break;
    case Qt::Key_Back:
    {
       //生成新配置
        QReconfigNetwork *rn=new  QReconfigNetwork (this);
        rn->generateConfig();
        keyBackMenu();
        delete rn;

    }
        break;
    default:
        break;
    }
}

void QDhcpMenu::showFunc(void)
{
    if(this->dhcp==false)
    {

        iface->setLcdLine(0," dhcp >static_ip");
        iface->setLcdLine(1,addr.toLatin1());

    }
    else
    {
        iface->setLcdLine(0,">dhcp  static_ip");
        iface->setLcdClearLine(1);
    }



}

void QDhcpMenu::exitInput(QMenu *menu, QString input,bool ival)
{
    if(ival)
    {
        this->addr=input;
    }
    else
        this->addr.clear();

    QMenu::deleteMenu(this,menu);
    delete menu;

    QSettings cfg(MENU_CONFIG_FILE,QSettings::IniFormat);
    cfg.beginGroup("network");
    cfg.setValue("ip",this->addr);
    cfg.endGroup();
}
