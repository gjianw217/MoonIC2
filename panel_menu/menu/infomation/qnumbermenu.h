#ifndef QNUMBERMENU_H
#define QNUMBERMENU_H

#include <QMenu>

class QNumberMenu : public QMenu
{
public:
    explicit QNumberMenu(QPanelInterface *iface,QMenu *parentMenu);
signals:

public slots:
    void domenu(int keycode,int val);
    void showFunc(void);

private:
    int number;
};

#endif // QNUMBERMENU_H
