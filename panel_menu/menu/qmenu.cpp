#include "qmenu.h"
#define DEBUG_01
QMenu  *QMenu::currentMenu=NULL;

QMenu::QMenu(QPanelInterface *iface,QString name,bool modeFunc,QObject *parent) :
    QObject(parent)
{
    Q_ASSERT(iface);
    parentMenu=NULL;
    preMenu=this;
    nextMenu=this;
    firstMenu=NULL;
    this->modeFunc=modeFunc;
    selectMenu=firstMenu;
    this->name=name;
    this->iface=iface;
}


QMenu::QMenu(QPanelInterface *iface,QString name,bool modeFunc,QMenu *pMenu,QObject *parent):
       QObject(parent)
{
    Q_ASSERT(pMenu);
    Q_ASSERT(iface);
    this->parentMenu=pMenu;
    this->name=name;
    this->modeFunc=modeFunc;
    this->iface=iface;
    registerMenu(this->parentMenu,this);
    selectMenu=firstMenu;
}


void QMenu::keyBackMenu(void)
{
    if( this->parentMenu!=NULL)
    {
        this->selectMenu=this->firstMenu;
        emit currentMenuChange(QMenu::currentMenu,this->parentMenu);
        QMenu::currentMenu=this->parentMenu;;
        this->parentMenu->show();
    }
}

void QMenu::menu(int keycode,int val)
{

#ifdef DEBUG_0
                            printf("QMenu::menu\n\r");
#endif
     if(this->modeFunc==true)
     {

#ifdef DEBUG_0
                            printf("QMenu::menu modeFunc domenu\n\r");
#endif
         this->domenu(keycode,val);
     }
     else
     {
         switch (keycode) {
         case Qt::Key_Enter:

#ifdef DEBUG_0
                            printf("QMenu::menu Qt::Key_Enter\n\r");
#endif
             if(this->selectMenu!=NULL && this->selectMenu!=QMenu::currentMenu)
             {
                 if(this->selectMenu->name=="back")
                 {
                     keyBackMenu();
                 }
                 else
                 {
                     emit currentMenuChange(QMenu::currentMenu,this->selectMenu);
                     QMenu::currentMenu=this->selectMenu;
                     this->selectMenu->show();
                 }
             }
             break;
         case Qt::Key_Right:

#ifdef DEBUG_0
                            printf("QMenu::menu Qt::Key_Right\n\r");
#endif
             QMenu::selectNext(this,true);
             break;
         case Qt::Key_Left:

#ifdef DEBUG_0
                            printf("QMenu::menu Qt::Key_Left\n\r");
#endif
             QMenu::selectNext(this,false);
             break;
         case Qt::Key_Back:

#ifdef DEBUG_0
                            printf("QMenu::menu Qt::Key_Back\n\r");
#endif
             keyBackMenu();
             break;
         default:
             break;
         }
     }
}

void QMenu::exitInput(QMenu *menu, QString input,bool ival)
{
    Q_UNUSED(menu);
    Q_UNUSED(input);
    Q_UNUSED(ival);
}

void QMenu::menuChange(QMenu *newmenu)
{
    emit currentMenuChange(QMenu::currentMenu,newmenu);
    QMenu::currentMenu=newmenu;
    newmenu->show();
}

void QMenu::domenu(int keycode,int val)
{
    Q_UNUSED(keycode);
    Q_UNUSED(val);
}

void QMenu::showFunc(void)
{

}

void QMenu::show(void)
{
    if(this->modeFunc==true)
    {
        this->showFunc();
    }
    else{
        this->showSelect(this);
    }
}

void QMenu::registerMenu(QMenu *parent,QMenu *menu)
{
    Q_ASSERT(parent);
    Q_ASSERT(menu);

    if(parent->firstMenu==NULL)
    {
        parent->firstMenu=menu;
        menu->preMenu=menu;
        menu->nextMenu=menu;
        menu->firstMenu=NULL;
        parent->selectMenu=parent->firstMenu;
    }
    else{
         QMenu *nmenu=parent->firstMenu;
         QMenu *prmenu=nmenu->preMenu;
         menu->nextMenu=nmenu;
         nmenu->preMenu=menu;
         prmenu->nextMenu=menu;
         menu->preMenu=prmenu;
         menu->firstMenu=NULL;
    }
}

QMenu *findMenu(QMenu *parent,QString name)
{
    Q_ASSERT(parent);

    QMenu *menu=parent->firstMenu;
    if(menu==NULL)
        return NULL;
    if(menu->name==name)
        return menu;

    menu=menu->nextMenu;
    while (menu!=parent->firstMenu) {
        if(menu->name==name)
            return menu;
        menu=menu->nextMenu;
    }
    return NULL;
}

void QMenu::deleteMenu(QMenu *parent,QMenu *menu)
{
    Q_ASSERT(parent);
    Q_ASSERT(menu);



     QMenu *nmenu=menu->nextMenu;
     QMenu *prmenu=menu->preMenu;

     if(parent->firstMenu==menu)
     {
         if(menu==nmenu)
         {
             parent->firstMenu=NULL;
             menu->parentMenu=NULL;
             menu->selectMenu=menu->firstMenu;
         }
         else
         {
            prmenu->nextMenu=nmenu;
            nmenu->preMenu=prmenu;
            parent->firstMenu=nmenu;
            if(parent->selectMenu==menu)
            {
                parent->selectMenu=parent->firstMenu;
            }

            menu->parentMenu=NULL;
            menu->preMenu=menu;
            menu->nextMenu=menu;
         }
     }
     else
     {
         prmenu->nextMenu=nmenu;
         nmenu->preMenu=prmenu;
         if(parent->selectMenu==menu)
         {
             parent->selectMenu=parent->firstMenu;
         }
         menu->parentMenu=NULL;
         menu->preMenu=menu;
         menu->nextMenu=menu;
     }
}

QMenu *QMenu::selectNext(QMenu *parent,bool np)
{
    if(parent->selectMenu==NULL)
    {
        parent->selectMenu=parent->firstMenu;

    }
    else
    {
        if(np==true)//next
        {
            parent->selectMenu=parent->selectMenu->nextMenu;
        }
        else
        {
            parent->selectMenu=parent->selectMenu->preMenu;

        }
    }
    showSelect(parent);

    return parent->selectMenu;
}
void QMenu::showSelect()
{
    showSelect(this);
}

void QMenu::showSelect(QMenu *parent)
{
    if(parent->selectMenu!=NULL)
    {
        parent->iface->setLcdLine(0,QString(">%1").arg(parent->selectMenu->name).toLatin1());
        if(parent->selectMenu->nextMenu!=parent->selectMenu)
        {
            parent->iface->setLcdLine(1,QString(" %1").arg(parent->selectMenu->nextMenu->name).toLatin1());
        }
        else
        {
            parent->iface->setLcdClearLine(1);
        }

    }
}
//void QMenu::timeout(void)
//{

//}
