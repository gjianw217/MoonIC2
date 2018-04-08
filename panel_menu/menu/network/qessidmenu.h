#ifndef QESSIDMENU_H
#define QESSIDMENU_H

#include <QMenu>
#include "qinputmenu.h"
#include "qreconfignetwork.h"

class QEssidMenu : public QMenu
{
public:
    QEssidMenu(QPanelInterface *iface,QMenu *parentMenu);

signals:

public slots:
    void domenu(int keycode,int val);
    void showFunc(void);
    void scanEssid(void);
    void exitInput(QMenu *menu, QString input,bool ival);
private:
    QStringList essid;
    int idx;
    bool scan;
    QMenu *menuInput;
    QString password;

};

#endif // QESSIDMENU_H
