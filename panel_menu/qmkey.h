#ifndef QMKEY_H
#define QMKEY_H

#include <QObject>
#include "qmkey0.h"
#include "qmkey1.h"
#include <QFile>

class QMkey : public QObject
{
    Q_OBJECT
public:
    explicit QMkey(QObject *parent = 0);

signals:
    void sigKey(int key,int val);
public slots:
    void slotKey(int key,int val);
private:
    QMkey0 *key0;
    QMkey1 *key1;

};

#endif // QMKEY_H
