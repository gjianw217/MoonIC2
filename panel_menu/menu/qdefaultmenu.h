#ifndef QDEFAULTMENU_H
#define QDEFAULTMENU_H

#include "qmenu.h"
#include <QtNetwork>
#include "qsettingmenu.h"
#include "qrebootmenu.h"
#include "qpowermenu.h"
#include "qshutdownmenu.h"
#include "qnetworkmenu.h"
#include "qinfomationmenu.h"
#include "qversionmenu.h"
#include "qnetdrivermenu.h"
#include "qessidmenu.h"
#include "qinputmenu.h"
#include "qdhcpmenu.h"
#include "qmacmenu.h"
#include "qreloadmenu.h"
#include "qmenuvermenu.h"
#include "qsnmenu.h"
#include "qshowsnmenu.h"
#include "qbackmenu.h"
#include "qnumbermenu.h"
#include "qnet2spimenu.h"

class QDefaultMenu : public QMenu
{
    Q_OBJECT
public:
    explicit QDefaultMenu(QPanelInterface *iface,QObject *parent = 0);

    QSettingMenu *setinfMenu;
signals:

public slots:
    void domenu(int keycode,int val);
    void showFunc(void);
    unsigned long getLoaclIpAddr(void);
    QByteArray getSn(void);
    unsigned char  ch2hex(unsigned char d);
    QString int2hex(quint32 d);
    QString sn2string(unsigned char buff[]);

};

#endif // QDEFAULTMENU_H
