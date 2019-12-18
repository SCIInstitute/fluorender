#ifndef DATASETS_LAYOUT_HPP
#define DATASETS_LAYOUT_HPP

#include <QGridLayout>
#include <QToolButton>
#include <QTreeView> //TODO: create custom treeview

#include <CustomWidgets/fluoLine.hpp>


class DatasetsLayout : public QGridLayout
{
  Q_OBJECT

  public:
    DatasetsLayout()
    {
      addRow(0,addToolbutton,renameToolbutton,saveToolbutton,bakeToolbutton,saveMaskToolbutton,deleteToolbutton,deleteAllToolbutton);
      addSingle(1,sepLine);
      addSingle(2,tempTreeview);
    }

  private:

   const int MAXCOLSIZE = 6;

   template<typename T>
   void addToWidget(T* widget,int row,int size)
   {
     int col = std::abs(size - MAXCOLSIZE);
     this->addWidget(widget,row,col,Qt::AlignHCenter);
   }

   template<typename T>
   void addRow(int row, T* widget)
   {
     this->addWidget(widget,row,MAXCOLSIZE,Qt::AlignHCenter);
   }

   template<typename Widget, typename... T>
   void addRow(int row, Widget *w,T*...t)
   {
     constexpr std::size_t n = sizeof...(T);
     addToWidget(w,row,n);
     addRow(row,t...);
   }

   template<typename T>
   void addSingle(int row, T *widget)
   {
     this->addWidget(widget,row,0,1,MAXCOLSIZE+1,Qt::AlignVCenter);
   }

    QToolButton *addToolbutton = new QToolButton();
    QToolButton *renameToolbutton = new QToolButton();
    QToolButton *saveToolbutton = new QToolButton();
    QToolButton *bakeToolbutton = new QToolButton();
    QToolButton *saveMaskToolbutton = new QToolButton();
    QToolButton *deleteToolbutton = new QToolButton();
    QToolButton *deleteAllToolbutton = new QToolButton();

    FluoLine *sepLine = new FluoLine();

    QTreeView *tempTreeview = new QTreeView();
};

#endif
