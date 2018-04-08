#ifndef QRELOADMENU_H
#define QRELOADMENU_H

#include <QMenu>

class QReloadMenu : public QMenu
{
    Q_OBJECT
public:
    explicit QReloadMenu(QPanelInterface *iface,QMenu *parentMenu);

signals:

public slots:
    void domenu(int keycode,int val);
    void showFunc(void);
private:
    bool reload;


};

#endif // QRELOADMENU_H
