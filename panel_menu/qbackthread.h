#ifndef QBACKTHREAD_H
#define QBACKTHREAD_H

#include <QThread>

class QBackThread : public QThread
{
    Q_OBJECT
public:
    explicit QBackThread(QObject *parent = 0);
    
signals:
    
public slots:

private:
    void run(void);
    void cheackCarNumber(void);
    int spi_pkt(unsigned long *buff,bool rw,int type,unsigned short addr);
    unsigned long spi_swap(unsigned long v);
void network(void);
};

#endif // QBACKTHREAD_H
