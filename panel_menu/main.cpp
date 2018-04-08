
#include <QCoreApplication>
#include "qmenumain.h"
//#include "qreconfignetwork.h"

int main(int argc, char *argv[])
{
    QCoreApplication a(argc,argv);
    //QReconfigNetwork *rn=new QReconfigNetwork(&a);
    //rn->configNetwork();

    try{
    QMenuMain *menu=new QMenuMain(&a);
    menu->initialization();
    }
    catch(QString e)
    {
        qDebug()<<e;
        return -1;
    }

    return a.exec();
}
