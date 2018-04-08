#ifndef QMkey0_H
#define QMkey0_H

#include <QThread>
#include <linux/input.h>
#include <stdio.h>
#include <stdint.h>
#include <unistd.h>
#include <sys/ioctl.h>
#include <sys/fcntl.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <QDebug>
#include <QDir>

//KEY_POWER
//KEY_BACK
class QMkey0 : public QThread
{
    Q_OBJECT
public:
    explicit QMkey0(QObject *parent = 0);
    bool exits;

signals:
    void sigKey(int key,int value);
public slots:

private:
    void run(void);
        int fd_event0;


};

#endif // QMkey0_H
