#ifndef QSHUTDOWNMENU_H
#define QSHUTDOWNMENU_H

#include "../qmenu.h"
#include <QProcess>

class QShutDownMenu : public QMenu
{
    Q_OBJECT
public:
    explicit QShutDownMenu(QPanelInterface *iface,QMenu *parentMenu);

signals:

public slots:
    void domenu(int keycode,int val);
    void showFunc(void);

private:
    bool shutDown;

};

#endif // QSHUTDOWNMENU_H
