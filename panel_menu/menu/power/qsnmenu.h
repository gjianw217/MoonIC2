#ifndef QSNMENU_H
#define QSNMENU_H

#include <QMenu>
#include <QtNetwork>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/ioctl.h>
#include <fcntl.h>
#include <unistd.h>
#include <stdio.h>
#include <signal.h>
#include <stdlib.h>
#include "string.h"


#define FLASH_SPI_DEVICE_NAME  "/dev/flash_spi3.0"

#define FLASH_SIZE 1024*1023*4
#define READ_LEN	4096

#define GPIO_FLASH_SPI		0x01
#define GPIO_FLASH_IO_INPUT	0x00

#define FLASH_SPI_MAGIC 'k'
#define FLASH_SPI_CONF    _IOWR(FLASH_SPI_MAGIC,0x50,int)
#define FLASH_SPI_DONE    _IOR (FLASH_SPI_MAGIC,0x51,int)
#define FLASH_SPI_CS1     _IOWR (FLASH_SPI_MAGIC,0x52,int)
#define FLASH_GPIO_MODE   _IOWR (FLASH_SPI_MAGIC,0x54,int)
#define FLASH_SN_READ   	_IOR (FLASH_SPI_MAGIC,0x5B,int)
#define FILE_SN             "/mooncell/etc/sn"
#define FILE_SN_READ        "/mooncell/etc/sn.rd"

class QSnMenu : public QMenu
{
    Q_OBJECT
public:
    explicit QSnMenu(QPanelInterface *iface,QMenu *parentMenu);
    void reloadSn(void);
signals:

public slots:
    void domenu(int keycode,int val);
    void showFunc(void);
private:
    bool reload;

};

#endif // QSNMENU_H
