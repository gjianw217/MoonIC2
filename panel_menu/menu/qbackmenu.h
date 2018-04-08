#ifndef QBACKMENU_H
#define QBACKMENU_H

#include <QMenu>

class QBackMenu : public QMenu
{
public:
    QBackMenu(QPanelInterface *iface,QMenu *parentMenu);
};

#endif // QBACKMENU_H
