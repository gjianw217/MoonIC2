#ifndef QPROTOCOL_H
#define QPROTOCOL_H

#include <QObject>

class QProtocol : public QObject
{
    Q_OBJECT
public:
    explicit QProtocol(QObject *parent = 0);
    int getpLcd(unsigned char *data,unsigned char x,unsigned char y,char *dis,int len);
    int getpLcdLine(unsigned char *data,unsigned char y,char *dis,int len);
    int getpLcdClear(unsigned char *data);
    int getpLcdClearLine(unsigned char *data,unsigned char y);
    int getpLcdFull(unsigned char *data,unsigned char full);
    int getpLcdFullLine(unsigned char *data,unsigned char y,unsigned char full);
    int getpLed(unsigned char *data,unsigned char on1off0,unsigned char index);
    int getpBz(unsigned char *data,unsigned char on1off0,unsigned char hz);
    void getpHead(unsigned char *data);
    unsigned long getpSum(unsigned char *data,int len);
    int getpLcdPos(unsigned char *data,unsigned char on1off0,unsigned char pos);

signals:

public slots:

};

#endif // QPROTOCOL_H
