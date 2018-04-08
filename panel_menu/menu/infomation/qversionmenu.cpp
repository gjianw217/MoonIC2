#include "qversionmenu.h"

QVersionMenu::QVersionMenu(QPanelInterface *iface,QMenu *parentMenu) :
    QMenu(iface,"system version",true,parentMenu,parentMenu)
{
}
void QVersionMenu::domenu(int keycode,int val)
{
    Q_UNUSED(val);
    switch (keycode) {
    case Qt::Key_Enter:
         keyBackMenu();
        break;
    case Qt::Key_Right:
    case Qt::Key_Left:
        break;
    case Qt::Key_Back:
        keyBackMenu();
        break;
    default:
        break;
    }
}

void QVersionMenu::showFunc(void)
{
    QFile fsys(VERSION_FILE_SYS);
    fsys.open(QIODevice::ReadOnly);
    QString vsys;
    if(!fsys.isOpen())
    {
        vsys="unknow";
    }
    else
    {
        vsys=QString(fsys.readLine());
        if(vsys.contains('\n'))
           vsys=vsys.remove('\n');
        if(vsys.contains('\r'))
           vsys=vsys.remove('\r');
        if(vsys.size()>14)
           vsys=vsys.remove(14,vsys.size()-14);
        fsys.close();
    }

    iface->setLcdLine(0,QString("s:%1").arg(vsys).toLatin1());

    QFile fpga(VERSION_FILE_FPGA);
    fpga.open(QIODevice::ReadOnly);
    QString vfpga;
    if(!fpga.isOpen())
    {
        vfpga="unknow";
    }
    else
    {
        vfpga=QString(fpga.readLine());
        if(vfpga.contains('\n'))
           vfpga=vfpga.remove('\n');
        if(vfpga.contains('\r'))
           vfpga=vfpga.remove('\r');
        if(vfpga.size()>14)
           vfpga=vfpga.remove(14,vsys.size()-14);
        fpga.close();
    }

    iface->setLcdLine(1,QString("f:%1").arg(vfpga).toLatin1());
}
