#ifndef QMENU_H
#define QMENU_H

#include <QObject>
#include <qpanelinterface.h>
#include <QSettings>

#define MENU_CONFIG_FILE "/mooncell/etc/menu.cfg"

class QMenu : public QObject
{
    Q_OBJECT
public:
    explicit QMenu(QPanelInterface *iface,QString name,bool modeFunc,QObject *parent = 0);
    QMenu(QPanelInterface *iface,QString name,bool modeFunc,QMenu *parentMenu,QObject *parent = 0);
    static void registerMenu(QMenu *parent,QMenu *menu);
    static void deleteMenu(QMenu *parent,QMenu *menu);
    static QMenu *findMenu(QMenu *parent,QString name);
    static QMenu *selectNext(QMenu *parent,bool np);
    static void showSelect(QMenu *parent);
    void showSelect();
    QString name;


signals:
    void currentMenuChange(QMenu *oldmenu,QMenu *newmenu);

public slots:
    void menuChange(QMenu *newmenu);
    virtual void domenu(int keycode,int val);
    virtual void    showFunc(void);
    virtual void exitInput(QMenu *menu, QString input,bool ival);
    //virtual void timeout(void);
    void    menu(int keycode,int val);
    void    keyBackMenu(void);
    void    show(void);

public:
    bool modeFunc;//0-menu,1-func
    static QMenu *currentMenu;
    QMenu *parentMenu;
    QMenu *preMenu;
    QMenu *nextMenu;
    QMenu *firstMenu;
    QPanelInterface *iface;
    QMenu *selectMenu;

};

#endif // QMENU_H
