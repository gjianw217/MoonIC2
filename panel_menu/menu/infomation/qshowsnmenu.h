#ifndef QShowSnMenu_H
#define QShowSnMenu_H

#include <QMenu>
#define FILE_SN             "/mooncell/etc/sn"
class QShowSnMenu : public QMenu
{
    Q_OBJECT
public:
    explicit QShowSnMenu(QPanelInterface *iface,QMenu *parentMenu);

signals:

public slots:
    void domenu(int keycode,int val);
    void showFunc(void);
private:
    unsigned char  ch2hex(unsigned char d);
    QString int2hex(quint32 d);
    QString sn2string(unsigned char buff[]);
    QByteArray getSn(void);
};

#endif // QShowSnMenu_H
