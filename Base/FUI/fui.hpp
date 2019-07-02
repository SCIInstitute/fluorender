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

private:
    Ui::FUI *ui;

    void setDockWidgetSizes();
    void checkFlags(std::reference_wrapper<QAction*> currentFlag);

    std::unique_ptr<QFrame> createNewRenderWindow(bool hasFeatures);
    std::unique_ptr<QFrame> extractWindow(QObject *toExtract);
    std::unique_ptr<QSplitter> createNewSplitter(Qt::Orientation orientation);
    std::unique_ptr<QSplitter> moveSplitter(Qt::Orientation orientation, int renderIndex);
    std::unique_ptr<QSplitter> flipSplitter(Qt::Orientation orientation, int view1, int view2);


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
};

#endif // FUI_HPP
