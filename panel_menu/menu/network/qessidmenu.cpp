#include "qessidmenu.h"
#define DEBUG_01
QEssidMenu::QEssidMenu(QPanelInterface *iface,QMenu *parentMenu) :
    QMenu(iface,"wifi essid",true,parentMenu,parentMenu)
{
    essid.clear();
    scan=false;
    idx=0;
    menuInput=NULL;
}
void QEssidMenu::showFunc(void)
{
#ifdef DEBUG_0
    printf("QEssidMenu::showFunc\r\n");
#endif

    if(!scan)
        scanEssid();

    if(this->essid.count())
    {
        int next=idx+1;
        next=next%this->essid.count();

        QSettings cfg(MENU_CONFIG_FILE,QSettings::IniFormat);
        QString def=cfg.value("network/essid").toString();


        if(def.size()>11)
            def=def.remove(10,def.size()-10);


        if(this->essid.at(idx).contains(def))
        {
            def=QString(">%1").arg(this->essid.at(idx));
            def=def.replace(3,1,"\x10");
            iface->setLcdLine(0,def.toLatin1());
        }
         else
            iface->setLcdLine(0,QString(">%1").arg(this->essid.at(idx)).toLatin1());

        if(this->essid.at(next).contains(def))
        {
            def=QString(" %1").arg(this->essid.at(next));
            def=def.replace(3,1,"\x10");
            iface->setLcdLine(1,def.toLatin1());
        }
        else
            iface->setLcdLine(1,QString(" %1").arg(this->essid.at(next)).toLatin1());
    }
    else
    {
        iface->setLcdLine(0,"scan fail!");
        iface->setLcdClearLine(1);
        // adding the back by gjianw217 20160527
        keyBackMenu();
        scan=false;
    }
}


void QEssidMenu::exitInput(QMenu *menu, QString input,bool ival)
{
#ifdef DEBUG_0
    printf("QEssidMenu::exitInput\r\n");
#endif

    if(ival)
    {
        password=input;
    }
    else
        password.clear();

    QMenu::deleteMenu(this,menu);
    delete menu;


    //keyBackMenu();//add by gjianw217 20160527
    this->menuInput=NULL;
    QSettings cfg(MENU_CONFIG_FILE,QSettings::IniFormat);
    cfg.beginGroup("network");
    cfg.setValue("password",password);
    cfg.endGroup();
}

void QEssidMenu::domenu(int keycode,int val)
{
#ifdef DEBUG_0
    printf("QEssidMenu::domenu\r\n");
#endif
    Q_UNUSED(val);
    switch (keycode) {
    case Qt::Key_Enter:
    {
        QFile fessid("essid.log");
        fessid.open(QIODevice::ReadOnly);
        if(!fessid.isOpen())
        {
            return;
        }
        for(int i=0;i<idx;i++)
            fessid.readLine();

        QSettings cfg(MENU_CONFIG_FILE,QSettings::IniFormat);
        cfg.beginGroup("network");
        QByteArray essid=fessid.readLine();
        cfg.setValue("essid",QString(essid.remove(essid.size()-1,1)));

        fessid.close();

        if(this->essid.at(idx).at(0).toLatin1()=='*')
        {//input password
            cfg.endGroup();
            iface->setLcdLine(0,"not suport pwd!");
            iface->setLcdClearLine(1);
            sleep(1);
            //menuInput=QInputMenu::change2Input(this,"");
            keyBackMenu();
            scan=false;

        }
        else
        {
            password.clear();
            cfg.setValue("password",password);
            cfg.endGroup();
           // this->show();
            //生成新配置
             QReconfigNetwork *rn=new  QReconfigNetwork (this);
             rn->generateConfig();
             keyBackMenu();
             scan=false;
             delete rn;
        }


    }
        break;
    case Qt::Key_Right:
        idx++;
        idx=idx%this->essid.count();
        this->show();
        break;
    case Qt::Key_Left:
        if(idx==0)
            idx=this->essid.count()-1;
        else
            idx=idx-1;

        this->show();
        break;
    case Qt::Key_Back:
    {
       //生成新配置
        QReconfigNetwork *rn=new  QReconfigNetwork (this);
        rn->generateConfig();
        keyBackMenu();
        scan=false;
        delete rn;

    }
        break;
    default:
        break;
    }

}

void QEssidMenu::scanEssid(void)
{
#ifdef DEBUG_0
    printf("QEssidMenu::scanEssid\r\n");
#endif

    system("/usr/bin/iwlist wlan0 scan >scan.log");
    system("cat scan.log |grep ESSID |awk -F ':' '{print $2}'|awk -F '\"' '{print $2}' >essid.log");
    system("cat scan.log |grep Quality |awk -F ' ' '{print $1}'|awk -F '=' '{print $2}' >signal.log");
    system("cat scan.log |grep key |awk -F ':' '{print $2}' >key.log");
    essid.clear();

    QFile fessid("essid.log");
    fessid.open(QIODevice::ReadOnly);
    if(!fessid.isOpen())
    {
        return;
    }

    QFile fsignal("signal.log");
    fsignal.open(QIODevice::ReadOnly);
    if(!fsignal.isOpen())
    {
        return;
    }

    QFile fkey("key.log");
    fkey.open(QIODevice::ReadOnly);
    if(!fkey.isOpen())
    {
        return;
    }
    while(!fkey.atEnd() && !fsignal.atEnd() &&!fessid.atEnd() )
    {
        QString item;

        QByteArray key=fkey.readLine();
        if(key.contains('n'))
        {
            item=item.append('*');
        }
        else
        {
            item=item.append(' ');
        }

        QByteArray sig=fsignal.readLine();
        sig=sig.remove(sig.size()-1,1);
        QList<QByteArray> s=sig.split('/');
        float fm=s.at(1).toFloat();
        if(fm==0)
            fm=70.0;
        float q= (s.at(0).toFloat()*5)/fm;
        int qi=((int)q)%5;

        item=item.append(QString("%1 ").arg(qi));

        QByteArray esid=fessid.readLine();
        if(esid.size()>12)
            esid.remove(11,esid.size()-11);
        else
            esid.remove(esid.size()-1,1);

        item=item.append(esid);
        //qDebug()<<item;
        this->essid.append(item);

    }
    fsignal.close();
    fessid.close();
    fkey.close();
    scan=true;
}


