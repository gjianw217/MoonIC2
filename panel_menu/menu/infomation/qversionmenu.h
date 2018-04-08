#ifndef QVERSIONMENU_H
#define QVERSIONMENU_H

#include <QMenu>

#define VERSION_FILE_SYS    "/mooncell/etc/system.ver"
#define VERSION_FILE_FPGA    "/mooncell/etc/fpga.ver"

class QVersionMenu : public QMenu
{
public:
    QVersionMenu(QPanelInterface *iface,QMenu *parentMenu);
signals:

public slots:
    void domenu(int keycode,int val);
    void showFunc(void);
};

#endif // QVERSIONMENU_H
