#ifndef QNETDRIVERMENU_H
#define QNETDRIVERMENU_H

#include <QMenu>

class QNetDriverMenu : public QMenu
{
public:
    QNetDriverMenu(QPanelInterface *iface,QMenu *parentMenu);

signals:

public slots:
    void domenu(int keycode,int val);
    void showFunc(void);
    void lnInterface(void);

private:
    int itfc_idx;
    int func;
    int yes;
};

#endif // QNETDRIVERMENU_H
