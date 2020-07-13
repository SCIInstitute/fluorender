#ifndef FUI_HPP
#define FUI_HPP

#include <QMainWindow>
#include <QScreen>
#include <QFrame>
#include <QSplitter>
#include <QVariant>

#include <iostream>
#include <memory>
#include <functional>
#include <unordered_map>
#include <algorithm>
#include <vector>

#include "renderview.hpp"

namespace Ui {
class FUI;
}

class FUI : public QMainWindow
{
    Q_OBJECT

public:
    explicit FUI(QWidget *parent = nullptr);

    ~FUI();

private slots:
    void on_actionSplit_HalfV_triggered();

    void on_actionSplit_HalfH_triggered();

    void on_actionSplit_Right_Half_triggered();

    void on_actionSplit_Bottom_Half_triggered();

    void on_actionSplit_Left_Half_triggered();

    void on_actionSplit_Top_Half_triggered();

    void on_actionSplit_All_triggered();

    void on_actionOne_View_triggered();

    void on_actionLoad_Volume_0_triggered();

    void on_actionLoad_Mesh_0_triggered();

private:
    Ui::FUI *ui;

    void setDockWidgetSizes();
    void checkFlags(QAction* currentFlag);

    QFrame* createNewRenderWindow(bool hasFeatures);
    QFrame* extractWindow(QObject *toExtract);
    QSplitter* createNewSplitter(Qt::Orientation orientation);
    QSplitter* moveSplitter(Qt::Orientation orientation, int renderIndex);
    QSplitter* flipSplitter(Qt::Orientation orientation, int view1, int view2);

    void updateRenderviewID(RenderView* view);
    void deleteRenderviewID(const RenderView* view);
    QString getFilename();
    QString getSuffix(QString &filename);

    auto getReader(const QString &suffix);

    // These are the indices of the left and right child of the main Splitter
    const int MAINRENDERINDEX = 0;
    const int M_RENDERIND_NEI = 1;

    // This is what determines the size of the QSplitters when split in half.
    int HALFRENDERVIEWSIZE;

    // These are how I keep track of how many windows are inside my main Splitter
    int mainWindowPanelCounter = 1;
    int neighborWindowPanelCounter = 0;

    // This is how I currently label my render views, this will change in the future.
    int renderViewcounter = 0;

    // This is my hash table, where the key is a boolean and the value is a QAction.
    std::unordered_map<bool,QAction*> flagControl;
    std::vector<const RenderView*> renderviews = {nullptr,nullptr,nullptr,nullptr};

};

#endif // FUI_HPP
