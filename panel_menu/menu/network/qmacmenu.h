#ifndef QMACMENU_H
#define QMACMENU_H

#include <QMenu>
#include "qinputmenu.h"
#include "qreconfignetwork.h"

class QMacMenu : public QMenu
{
    Q_OBJECT
public:
    explicit QMacMenu(QPanelInterface *iface,QMenu *parentMenu);

signals:

public slots:
    void domenu(int keycode,int val);
    void showFunc(void);
    void exitInput(QMenu *menu, QString input,bool ival);
private:
    QString mac;
};

#endif // QMACMENU_H
