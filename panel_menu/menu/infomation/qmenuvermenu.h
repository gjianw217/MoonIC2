#ifndef QMENUVERMENU_H
#define QMENUVERMENU_H

#include <QMenu>

class QMenuVerMenu : public QMenu
{
    Q_OBJECT
public:
    explicit QMenuVerMenu(QPanelInterface *iface,QMenu *parentMenu);
signals:

public slots:
    void domenu(int keycode,int val);
    void showFunc(void);

};

#endif // QMENUVERMENU_H
