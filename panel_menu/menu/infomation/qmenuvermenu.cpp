#include "qmenuvermenu.h"

QMenuVerMenu::QMenuVerMenu(QPanelInterface *iface,QMenu *parentMenu) :
    QMenu(iface,"menu version",true,parentMenu,parentMenu)
{
}
void QMenuVerMenu::domenu(int keycode,int val)
{
    Q_UNUSED(val);
    switch (keycode) {
    case Qt::Key_Enter:
         keyBackMenu();
        break;
    case Qt::Key_Right:
    case Qt::Key_Left:
        break;
    case Qt::Key_Back:
        keyBackMenu();
        break;
    default:
        break;
    }
}

void QMenuVerMenu::showFunc(void)
{
    iface->setLcdLine(0,"menu version:");
    iface->setLcdLine(1," v1.0r2 ");
}
