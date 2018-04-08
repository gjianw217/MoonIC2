#ifndef QNET2SPIMENU_H
#define QNET2SPIMENU_H

#include <QMenu>

class QNet2spiMenu : public QMenu
{
public:
    QNet2spiMenu(QPanelInterface *iface,QMenu *parentMenu);

signals:

public slots:
    void domenu(int keycode,int val);
    void showFunc(void);
    bool connect;
};

#endif // QNET2SPIMENU_H
