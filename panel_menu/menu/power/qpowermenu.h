#ifndef QPOWERMENU_H
#define QPOWERMENU_H

#include "qmenu.h"

class QPowerMenu : public QMenu
{
    Q_OBJECT
public:
    explicit QPowerMenu(QPanelInterface *iface,QMenu *parentMenu);

signals:

public slots:

};

#endif // QPOWERMENU_H
