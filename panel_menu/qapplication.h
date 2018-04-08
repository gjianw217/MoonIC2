#ifndef QAPPLICATION_H
#define QAPPLICATION_H

#include <QCoreApplication>
#include <qpanelinterface.h>

class QApplication : public QCoreApplication
{
    Q_OBJECT
public:
    explicit QApplication(QPanelInterface *iface,int &argc, char **argv);

signals:

public slots:
   // void processKey(void);

private:
//    bool event(QEvent *);
void timerEvent(QTimerEvent *);
    QPanelInterface *iface;

};

#endif // QAPPLICATION_H
