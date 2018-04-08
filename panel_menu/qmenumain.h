#ifndef QMENUMAIN_H
#define QMENUMAIN_H

#include <QObject>
#include <qpanelinterface.h>
#include "qmkey.h"
#include <QtNetwork>
#include "./menu/qmenu.h"
#include "./menu/qdefaultmenu.h"
#include "qbackthread.h"

class QMenuMain : public QObject
{
    Q_OBJECT
public:
    explicit QMenuMain(QObject *parent = 0);
    void initialization(void);

    void showIpSn(void);
    unsigned long getLoaclIpAddr(void);

signals:

public slots:
   void curMenuChange(QMenu *oldmenu,QMenu *newmenu);
private:
    QPanelInterface *iface;
    QMkey *key;
    QMenu *defMenu;
    QBackThread *backTherd;
    void timerEvent(QTimerEvent *);
    //QDefaultMenu *menu;

};

#endif // QMENUMAIN_H
