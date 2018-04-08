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
    bool setLed(unsigned char on1off0,unsigned char index);
    bool setBz(unsigned char on1off0,unsigned char hz);

    bool setLcd(unsigned char x,unsigned char y,QByteArray data);
    bool setLcdLine(unsigned char y,QByteArray data);
    bool setLcdClear(void);
    bool setLcdClearLine(unsigned char y);
    bool setLcdFull(unsigned char data);
    bool setLcdFullLine(unsigned char y,unsigned char data);
    bool setLcdPos(unsigned char on1off0,unsigned char pos);

private:
    QPanel *panel;

};

#endif // QPANELDBUS_H
