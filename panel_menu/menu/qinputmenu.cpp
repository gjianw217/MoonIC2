#include "qinputmenu.h"

QInputMenu::QInputMenu(QPanelInterface *iface,QMenu *parentMenu,QObject *parent) :
    QMenu(iface,"input",true,parentMenu,parent)
{
    input.clear();
    x=8;

    showInput=true;

    line=0;

}
char *QInputMenu::ch[]={
        (char*)"0123456789abc\x1B\x1C\x10",
        (char*)"defghijklmnopqrs",
        (char*)"tuvwxyzABCDEFGHI",
        (char*)"JKLMNOPQRSTUVWXY",
        (char*)"Z,./?:;\"'\\|!@#$%",
        (char*)"&*()-+{}[]~_=<> "
    };


void QInputMenu::domenu(int keycode,int val)
{
    Q_UNUSED(val);
    switch (keycode) {
    case Qt::Key_Enter:
        {
            this->line=this->line%6;
            this->x=this->x%16;
            char in=this->ch[this->line][this->x];
            switch(in)
            {
                case 0x1B://del
                {
                    if(this->input.size())
                        this->input.remove(this->input.size()-1,1);
                    showInput=true;
                    this->show();
                }break;
                case 0x1C://enter
                {
                    //if(this->input.size())
                    //    this->input.remove(this->input.size()-1,1);

                    this->line=0;
                    this->x=0;
                    this->showInput=true;

                    iface->setLcdPos(0,0);
                    keyBackMenu();
                    emit inputExit(this, this->input,true);

                }break;
                case 0x10:
                {
                    this->line=0;
                    this->x=0;
                    this->showInput=true;

                    this->input.clear();

                    iface->setLcdPos(0,0);
                    keyBackMenu();
                    emit inputExit(this, this->input,false);
                }break;
                default:
                {
                    this->input=this->input.append(in);
                    showInput=true;
                    this->show();
                }break;
            }
        }
        break;
    case Qt::Key_Right:
        x++;
        if(x==16)
        {
                line=line+1;
                line=line%6;
        }
        x=x%16;
        this->show();

        break;
    case Qt::Key_Left:
        if(x==0){
            x=15;
            if(line==0)
            {
                line=5;
            }
            else{
                line=line-1;
            }
            line=line%6;
        }
        else
            x--;
        this->show();
        break;
    case Qt::Key_Back:
        line=line+1;
        line=line%6;
        this->show();
        break;
    default:
        break;
    }

}

void QInputMenu::showFunc(void)
{


    line=line%6;
    int line1=line+1;
    line1=line1%6;
    iface->setLcdPos(0,0);


    if(showInput==true)
    {
        if(this->input.size())

            iface->setLcdLine(0,this->input.toLatin1());
        else
            iface->setLcdClearLine(0);
        showInput=false;
    }

    if(x%2==0)
        iface->setLcdLine(1,QString(ch[line]).replace(x+1,1,' ').toLatin1());
    else
        iface->setLcdLine(1,QString(ch[line]).replace(x-1,1,' ').toLatin1());

    iface->setLcdPos(1,x/2+8);
}


QMenu * QInputMenu::change2Input(QMenu *parent,QString str)
{
    QInputMenu *input=new QInputMenu(parent->iface,parent);
    if(str!="")
    {
        input->setBaseInput(str);
    }
    parent->menuChange(input);
    connect(input,SIGNAL(inputExit(QMenu*,QString,bool)),parent,SLOT(exitInput(QMenu*,QString,bool)));
    return input;
}
