#include "qrebootmenu.h"

QRebootMenu::QRebootMenu(QPanelInterface *iface,QMenu *parentMenu) :
    QMenu(iface,"reboot",true,parentMenu,parentMenu)
{
    reboot=false;
}

void QRebootMenu::showFunc(void)
{
    iface->setLcdLine(0,"reboot system?");
    if(!reboot)
        iface->setLcdLine(1,"  >no    yes   ");
    else
        iface->setLcdLine(1,"  no    >yes   ");

}

void QRebootMenu::domenu(int keycode,int val)
{
    Q_UNUSED(val);
    switch (keycode) {
    case Qt::Key_Enter:
            if(reboot==true)
            {
                iface->setLcdClear();
                iface->setLcdPos(0,0);
                QProcess reboot;
                reboot.execute("/sbin/reboot");
            }
            else
            {
                keyBackMenu();
                reboot=false;
            }
        break;
    case Qt::Key_Right:
        reboot=!reboot;
        this->show();
        break;
    case Qt::Key_Left:
        reboot=!reboot;
        this->show();
        break;
    case Qt::Key_Back:
        reboot=false;
        keyBackMenu();
        break;
    default:
        break;
    }
}
