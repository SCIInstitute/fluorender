#ifndef WORKSPACE_TOOL_LAYOUT_HPP
#define WORKSPACE_TOOL_LAYOUT_HPP

#include <QGridLayout>
#include <QTreeView>

#include <CustomWidgets/fluoLine.hpp>
#include <CustomWidgets/fluoToolButton.hpp>

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

    FluoToolButton *toggleToolbutton            = new FluoToolButton(":/workspace-eye.svg");
    FluoToolButton *addVolumeToolbutton         = new FluoToolButton(":/workspace-newVolume.svg");
    FluoToolButton *addMeshToolbutton           = new FluoToolButton(":/workspace-newMesh.svg");
    FluoToolButton *deleteCurrentToolbutton     = new FluoToolButton(":/workspace-deleteSelection.svg");
    FluoToolButton *highlightToolbutton         = new FluoToolButton(":/workspace-highLight.svg");
    FluoToolButton *diffuseHighlightToolbutton  = new FluoToolButton(":/workspace-diffuse.svg");
    FluoToolButton *resetHighlightToolbutton    = new FluoToolButton(":/workspace-resetSelect.svg");
    FluoToolButton *eraseHighlightToolbutton    = new FluoToolButton(":/workspace-eraseHighlight.svg");
    FluoToolButton *extractHighlightToolbutton  = new FluoToolButton(":/workspace-extract.svg");
    FluoToolButton *resetallHighlightToolbutton = new FluoToolButton(":/workspace-resetAll.svg");

    FluoLine *sepLine1 = new FluoLine(QFrame::VLine);
    FluoLine *sepLine2 = new FluoLine(QFrame::VLine);

};


#endif
