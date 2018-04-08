#include "qnet2spimenu.h"

QNet2spiMenu::QNet2spiMenu(QPanelInterface *iface,QMenu *parentMenu) :
    QMenu(iface,"net2spi reconn..",true,parentMenu,parentMenu)
{
        connect=false;
}
void QNet2spiMenu::domenu(int keycode,int val)
{
    Q_UNUSED(val);
    switch (keycode) {
    case Qt::Key_Enter:
    {
        if( connect==false)
        {
                keyBackMenu();
        }
        else
        {
            system("killall net2spi");
            connect=false;
            sleep(1);
            system("/mooncell/bin/net2spi &");
            keyBackMenu();
        }

    }
        break;
    case Qt::Key_Right:
    case Qt::Key_Left:
        connect=!connect;
        this->show();
        break;
    case Qt::Key_Back:
    {
       //生成新配置
        connect=false;
        keyBackMenu();
    }
        break;
    default:
        break;
    }
}

void QNet2spiMenu::showFunc(void)
{
    iface->setLcdLine(0," reconnect... ?");
    if( connect==false)
        iface->setLcdLine(1,">no    yes");
    else
        iface->setLcdLine(1," no   >yes");

}
