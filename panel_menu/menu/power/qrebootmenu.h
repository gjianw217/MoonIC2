#ifndef QREBOOTMENU_H
#define QREBOOTMENU_H

#include "qmenu.h"
#include <QProcess>

class QRebootMenu : public QMenu
{
    Q_OBJECT
public:
    explicit QRebootMenu(QPanelInterface *iface,QMenu *parentMenu);

signals:

public slots:
    void domenu(int keycode,int val);
    void showFunc(void);

private:
    bool reboot;

};

#endif // QREBOOTMENU_H
