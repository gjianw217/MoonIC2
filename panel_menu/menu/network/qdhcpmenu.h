#ifndef QDHCPMENU_H
#define QDHCPMENU_H

#include <QMenu>
#include "qinputmenu.h"
#include "qreconfignetwork.h"

class QDhcpMenu : public QMenu
{
    Q_OBJECT
public:
    explicit QDhcpMenu(QPanelInterface *iface,QMenu *parentMenu);

signals:

public slots:
    void domenu(int keycode,int val);
    void showFunc(void);
    void exitInput(QMenu *menu, QString input,bool ival);
private:
    QString addr;
    bool dhcp;

};

#endif // QDHCPMENU_H
