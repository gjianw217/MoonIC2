#include "qreloadmenu.h"

QReloadMenu::QReloadMenu(QPanelInterface *iface,QMenu *parentMenu) :
    QMenu(iface,"reload network",true,parentMenu,parentMenu)
{
    reload=false;
}
void QReloadMenu::domenu(int keycode,int val)
{
    Q_UNUSED(val);
    switch (keycode) {
    case Qt::Key_Enter:
    {
        if( reload==false)
        {
                keyBackMenu();
        }
        else
        {
            system("/etc/init.d/wlan0 restart");
            reload=false;
            keyBackMenu();
        }

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
    {
       //生成新配置
        reload=false;
        keyBackMenu();
    }
        break;
    default:
        break;
    }
}

void QReloadMenu::showFunc(void)
{
    iface->setLcdLine(0," reload ?");
    if( reload==false)
        iface->setLcdLine(1,">no    yes");
    else
        iface->setLcdLine(1," no   >yes");

}
