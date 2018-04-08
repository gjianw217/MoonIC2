#include "qpowermenu.h"

QPowerMenu::QPowerMenu(QPanelInterface *iface,QMenu *parentMenu) :
    QMenu(iface,"power",false,parentMenu,parentMenu)
{
}
