#include "qnumbermenu.h"
#include <QSettings>
#include <QString>

QNumberMenu::QNumberMenu(QPanelInterface *iface,QMenu *parentMenu) :
    QMenu(iface,"set number",true,parentMenu,parentMenu)
{
    QSettings cfg(MENU_CONFIG_FILE,QSettings::IniFormat);
    QString num=cfg.value("info/number").toString();

    if(num.isEmpty())
        this->number=1;
    else
        this->number=num.toInt();

    if(this->number<1)this->number=15;
    if(this->number>15)this->number=1;
}
void QNumberMenu::domenu(int keycode,int val)
{
    Q_UNUSED(val);



    switch (keycode) {
    case Qt::Key_Enter:
    {
        QSettings cfg(MENU_CONFIG_FILE,QSettings::IniFormat);
        cfg.beginGroup("info");
        cfg.setValue("number",this->number);
        cfg.endGroup();

         keyBackMenu();
    }
        break;
    case Qt::Key_Right:
        this->number++;
        if(this->number>15)this->number=1;
        this->show();
        break;
    case Qt::Key_Left:
        this->number--;
        if(this->number<1)this->number=15;
        this->show();
        break;
    case Qt::Key_Back:
        keyBackMenu();
        break;
    default:
        break;
    }
}

void QNumberMenu::showFunc(void)
{

    iface->setLcdLine(0,"number:");
    iface->setLcdLine(1,QString("   %1").arg(this->number).toAscii());
}
