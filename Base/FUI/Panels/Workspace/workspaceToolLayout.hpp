#ifndef WORKSPACE_TOOL_LAYOUT_HPP
#define WORKSPACE_TOOL_LAYOUT_HPP

#include <QGridLayout>
#include <QToolButton>
#include <QTreeView>

#include <CustomWidgets/fluoLine.hpp>

class WorkspaceToolLayout : public QGridLayout
{
  Q_OBJECT

  public:
    WorkspaceToolLayout()
    {
      addRow(0,toggleToolbutton,addVolumeToolbutton,addMeshToolbutton,
             deleteCurrentToolbutton,sepLine1,highlightToolbutton,
             diffuseHighlightToolbutton,resetHighlightToolbutton,
             eraseHighlightToolbutton,sepLine2,extractHighlightToolbutton,
             resetallHighlightToolbutton);
    }

  private:
    const int MAXCOLSIZE = 11;

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

    QToolButton *toggleToolbutton            = new QToolButton();
    QToolButton *addVolumeToolbutton         = new QToolButton();
    QToolButton *addMeshToolbutton           = new QToolButton();
    QToolButton *deleteCurrentToolbutton     = new QToolButton();
    QToolButton *highlightToolbutton         = new QToolButton();
    QToolButton *diffuseHighlightToolbutton  = new QToolButton();
    QToolButton *resetHighlightToolbutton    = new QToolButton();
    QToolButton *eraseHighlightToolbutton    = new QToolButton();
    QToolButton *extractHighlightToolbutton  = new QToolButton();
    QToolButton *resetallHighlightToolbutton = new QToolButton();

    FluoLine *sepLine1 = new FluoLine(QFrame::VLine);
    FluoLine *sepLine2 = new FluoLine(QFrame::VLine);

};


#endif
