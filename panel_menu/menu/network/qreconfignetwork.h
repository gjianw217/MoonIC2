#ifndef QRECONFIGNETWORK_H
#define QRECONFIGNETWORK_H

#include <QObject>
#include <QtNetwork>
#include <QMenu>

class QInterfaceInfo : public QObject
{
    Q_OBJECT
public:
    explicit QInterfaceInfo(QObject *parent = 0);
    QString interface;
    QString name;
    QStringList drv; 
    bool mac;
signals:

public slots:
};

class QReconfigNetwork : public QObject
{
    Q_OBJECT
public:
    explicit QReconfigNetwork(QObject *parent = 0);
    bool dhcp;
    QString essid;
    QString pwd;
    QString macaddr;

    QHostAddress addr;
    QInterfaceInfo info[5];
    QInterfaceInfo *curIf;
    bool have;
signals:

public slots:
    void configNetwork(void);
    void setCurrentEssid(QString essid,QString pwd);
    void setCurrentIf(QString name,QString macaddr);
    void setCurrentDhcp(QString dhcp,QString addr);
    void getConfigFile(void);
    void generateConfig(void);

};

#endif // QRECONFIGNETWORK_H
