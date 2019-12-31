#ifndef DATASETS_TOOLBAR_LAYOUT_HPP
#define DATASETS_TOOLBAR_LAYOUT_HPP

#include <QGridLayout>
#include <QToolButton>

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

    QToolButton *addToolbutton = new QToolButton();
    QToolButton *renameToolbutton = new QToolButton();
    QToolButton *saveToolbutton = new QToolButton();
    QToolButton *bakeToolbutton = new QToolButton();
    QToolButton *saveMaskToolbutton = new QToolButton();
    QToolButton *deleteToolbutton = new QToolButton();
    QToolButton *deleteAllToolbutton = new QToolButton();

};

#endif
