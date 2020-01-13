#ifndef DATASETS_TOOLBAR_LAYOUT_HPP
#define DATASETS_TOOLBAR_LAYOUT_HPP

#include <QGridLayout>
#include <CustomWidgets/fluoToolButton.hpp>

class DatasetsToolbarLayout : public QGridLayout
{
  Q_OBJECT

  public:
    DatasetsToolbarLayout()
    {
      addRow(0,addToolbutton,renameToolbutton,
             saveToolbutton,bakeToolbutton,
             saveMaskToolbutton,deleteToolbutton,deleteAllToolbutton
             );
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

    FluoToolButton *addToolbutton = new FluoToolButton(":/datasets-addDataset.svg");
    FluoToolButton *renameToolbutton = new FluoToolButton(":/datasets-renameDataset.svg");
    FluoToolButton *saveToolbutton = new FluoToolButton(":/datasets-saveDataset.svg");
    FluoToolButton *bakeToolbutton = new FluoToolButton(":/datasets-bake.svg");
    FluoToolButton *saveMaskToolbutton = new FluoToolButton(":/datasets-saveMask.svg");
    FluoToolButton *deleteToolbutton = new FluoToolButton(":/datasets-deleteDataset.svg");
    FluoToolButton *deleteAllToolbutton = new FluoToolButton(":/datasets-deleteAll.svg");

};

#endif
