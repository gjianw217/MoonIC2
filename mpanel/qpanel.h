#ifndef QPANEL_H
#define QPANEL_H

#include <QObject>
#include "qextserialbase.h"
#include "posix_qextserialport.h"
#include <QTimer>
#include <QDebug>
#include "qprotocol.h"


class QPanel : public QObject
{
    Q_OBJECT
public:
    explicit QPanel(QObject *parent = 0);
    int initializtion(char *arg);

    QProtocol *procotol;
    unsigned char Hex2Dec(unsigned char ch);
signals:

public slots:
    void readPort(void);
    bool writePort(unsigned char *data,int len);
    void readDev(void);

private:
    Posix_QextSerialPort *port;
    QTimer *timer;
    QTimer *timerdev;

};

#endif // QPANEL_H
