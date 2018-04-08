#ifndef QSETTINGMENU_H
#define QSETTINGMENU_H


#include "qmenu.h"

class QSettingMenu : public QMenu
{
    Q_OBJECT
public:
    explicit QSettingMenu(QPanelInterface *iface,QMenu *parentMenu,QObject *parent = 0);

signals:

public slots:


};

#endif // QSETTINGMENU_H
