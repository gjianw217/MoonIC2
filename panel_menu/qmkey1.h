#ifndef QMKEY1_H
#define QMKEY1_H

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
#include <QFileInfoList>
#include <QDir>

//static unsigned char panel_key_keycode[PANEL_KEY_CODE_SIZE] = {	/* American layout */
//	[0]	 = KEY_LEFT,
//	[1]	 = KEY_ENTER,
//	[2]	 = KEY_PAUSE,
//	[3]	 = KEY_RIGHT,
//};

class QMkey1 : public QThread
{
    Q_OBJECT
public:
    explicit QMkey1(QObject *parent = 0);
    bool exits;

signals:
    void sigKey(int key,int value);
public slots:

private:
    void run(void);
        int fd_event1;

};

#endif // QMKEY1_H
