#include "qmenumain.h"

QMenuMain::QMenuMain(QObject *parent) :
    QObject(parent)
{

}

void QMenuMain::initialization(void)
{
#define DEBUG_0
#ifdef DEBUG_0
                    printf("QMenuMain::initialization)\n\r");
#endif
   iface=new QPanelInterface("com.mooncell.panel","/panel/path",QDBusConnection::systemBus());
   Q_ASSERT(iface!=NULL);
   if(!iface->isValid())
   {
       qDebug()<<QDBusConnection::systemBus().lastError().message()<<qPrintable(QDBusConnection::systemBus().lastError().message());
       delete iface;

       qDebug()<<"err:dbus inter face";
       throw QString("err:dbus inter face");
       return;
   }
   key=new QMkey(this);
   //backTherd=new QBackThread(this);
   //backTherd->start();

   Q_ASSERT(key!=NULL);

   defMenu=new QDefaultMenu(iface,this);//QMenu::currentMenu=defMenu;//这样仅需修改currentMenu
   connect(key,SIGNAL(sigKey(int,int)),QMenu::currentMenu,SLOT(menu(int,int)));
   connect(QMenu::currentMenu,SIGNAL(currentMenuChange(QMenu*,QMenu *)),this,SLOT(curMenuChange(QMenu*,QMenu *)));

   this->startTimer(10000);

}

void QMenuMain::curMenuChange(QMenu *oldmenu,QMenu *newmenu)
{

    disconnect(key,SIGNAL(sigKey(int,int)),oldmenu,SLOT(menu(int,int)));
    disconnect(oldmenu,SIGNAL(currentMenuChange(QMenu*,QMenu *)),this,SLOT(curMenuChange(QMenu*,QMenu *)));

    connect(key,SIGNAL(sigKey(int,int)),newmenu,SLOT(menu(int,int)));
    connect(newmenu,SIGNAL(currentMenuChange(QMenu*,QMenu *)),this,SLOT(curMenuChange(QMenu*,QMenu *)));
}

void QMenuMain::timerEvent(QTimerEvent *)
{
       if(QMenu::currentMenu==defMenu)
                defMenu->showFunc();
}

//void QMenuMain::settingMenu(int keycode,int val)
//{
//Q_UNUSED(val);
//    switch (keycode) {
//    case Qt::Key_Right:
//    case Qt::Key_Left:
//        break;
//    default:
//        iface->setLcdPos(0,0);
//        iface->setLcd(0,0,"> set wifi");
//        iface->setLcd(0,1,"  reboot");
//        iface->setLcdPos(1,0);
//        break;
//    }
//}
