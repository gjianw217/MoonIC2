#include "qapplication.h"
#include <QKeyEvent>

QApplication::QApplication(QPanelInterface *iface,int &argc, char **argv) :
    QCoreApplication(argc,argv)
{
    if(iface==NULL)
        throw QString("no dbus interface");

    this->iface=iface;

    this->startTimer(100);

}

void QApplication::timerEvent(QTimerEvent *)
{


}

//bool QApplication::event(QEvent *e)
//{
//    if(e->type () == QEvent::KeyPress){
//            QKeyEvent *ke=(QKeyEvent *)e;
//            if(ke->key() == Qt::Key_Enter){
//                qDebug()<<"enter";
//                    return true;
//            }
//    }
//    return false;
//}
