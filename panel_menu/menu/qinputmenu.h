#ifndef QINPUTMENU_H
#define QINPUTMENU_H

#include <QMenu>
#include <QTimer>

class QInputMenu : public QMenu
{
    Q_OBJECT
public:
    QInputMenu(QPanelInterface *iface,QMenu *parentMenu,QObject *parent = 0);
    static char* ch[6];
    //void inputMenuChange(void);
    void setBaseInput(QString x){input=x;}

signals:
    void inputExit(QMenu *menu, QString input,bool ival);
public slots:
    void domenu(int keycode,int val);
    void showFunc(void);
    static QMenu * change2Input(QMenu *parent,QString str="");

private:
    int x,line;
    QString input;
    bool showInput;


};

#endif // QINPUTMENU_H
