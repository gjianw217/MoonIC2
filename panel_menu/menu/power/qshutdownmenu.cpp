#include "qshutdownmenu.h"

QShutDownMenu::QShutDownMenu(QPanelInterface *iface,QMenu *parentMenu) :
    QMenu(iface,"ShutDown",true,parentMenu,parentMenu)
{
    shutDown=false;
}

void QShutDownMenu::domenu(int keycode,int val)
{
    Q_UNUSED(val);
    switch (keycode) {
    case Qt::Key_Enter:
            if(shutDown==true)
            {
                iface->setLcdClear();
                iface->setLcdPos(0,0);
                QProcess reboot;
                QStringList arg;
                arg<<"-h"<<"-f"<<"now";
                reboot.execute("/sbin/shutdown",arg);
            }
            else
            {
                keyBackMenu();
                shutDown=false;
            }
        break;
    case Qt::Key_Right:
        shutDown=!shutDown;
        this->show();
        break;
    case Qt::Key_Left:
        shutDown=!shutDown;
        this->show();
        break;
    case Qt::Key_Back:
        shutDown=false;
        keyBackMenu();
        break;
    default:
        break;
    }
}

void QShutDownMenu::showFunc(void)
{
    iface->setLcdLine(0,"ShutDown system?");
    if(!shutDown)
        iface->setLcdLine(1,"  >no    yes   ");
    else
        iface->setLcdLine(1,"  no    >yes   ");
}
