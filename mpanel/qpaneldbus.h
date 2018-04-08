#ifndef QPANELDBUS_H
#define QPANELDBUS_H

#include <QObject>
#include <QDebug>
#include "qpanel.h"

class QPanelDbus : public QObject
{
    Q_OBJECT
    Q_CLASSINFO("D-Bus Interface", "com.mooncell.panel.interface")
public:
    explicit QPanelDbus(QPanel *panel,QObject *parent = 0);

signals:

public slots:
    bool setLed(uchar on1off0,uchar index);
    bool setBz(uchar on1off0,uchar hz);

    bool setLcd(uchar x,uchar y,QByteArray data);
    bool setLcdLine(uchar y,QByteArray data);
    bool setLcdClear(void);
    bool setLcdClearLine(uchar y);
    bool setLcdFull(uchar data);
    bool setLcdFullLine(uchar y,uchar data);
    bool setLcdPos(uchar on1off0,uchar pos);

private:
    QPanel *panel;

};

#endif // QPANELDBUS_H
