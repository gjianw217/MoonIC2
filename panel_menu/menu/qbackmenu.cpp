#include "qbackmenu.h"

QBackMenu::QBackMenu(QPanelInterface *iface,QMenu *parentMenu) :
    QMenu(iface,"back",false,parentMenu,parentMenu)
{
}
