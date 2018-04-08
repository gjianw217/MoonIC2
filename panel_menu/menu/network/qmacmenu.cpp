#include "qmacmenu.h"

QMacMenu::QMacMenu(QPanelInterface *iface,QMenu *parentMenu) :
    QMenu(iface,"set mac",true,parentMenu,parentMenu)
{
}

void QMacMenu::domenu(int keycode,int val)
{
    Q_UNUSED(val);
    switch (keycode) {
    case Qt::Key_Enter:
    {
            iface->setLcdLine(0,"input mac");
            iface->setLcdClearLine(1);
            sleep(1);
            QInputMenu::change2Input(this,this->mac);

    }
        break;
    case Qt::Key_Right:
        break;
    case Qt::Key_Left:

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

void QMacMenu::showFunc(void)
{

    iface->setLcdLine(1,"mac:");
        iface->setLcdLine(1,mac.toLatin1());

}

void QMacMenu::exitInput(QMenu *menu, QString input,bool ival)
{
    if(ival)
    {
        this->mac=input;
    }
    else
        this->mac.clear();

    QMenu::deleteMenu(this,menu);
    delete menu;
    keyBackMenu();//add by gjianw217 20160527
    QSettings cfg(MENU_CONFIG_FILE,QSettings::IniFormat);
    cfg.beginGroup("network");
    cfg.setValue("mac",this->mac);
    cfg.endGroup();
}
