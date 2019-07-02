#include "fui.hpp"
#include <QApplication>
#include <QDockWidget>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    FUI w;

    w.show();

    return a.exec();
}
