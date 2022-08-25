#ifndef QVISMAINMANAGER_H
#define QVISMAINMANAGER_H

#include <QWidget>
#include <QSettings>

class QvisSourceManager;
class QvisDatasetManager;
class QvisWorkspaceManager;
class QvisAnimationController;

namespace Ui {
class QvisMainManager;
}

// QvisMainManager
class QvisMainManager : public QWidget
{
    Q_OBJECT

public:
    explicit QvisMainManager(QWidget *parent = nullptr);
    ~QvisMainManager();

    void saveState(QSettings &Settings) const;
    void restoreState(const QSettings &Settings);

    QvisSourceManager       * getSourceManager      () const;
    QvisDatasetManager      * getDatasetManager     () const;
    QvisWorkspaceManager    * getWorkspaceManager   () const;
    QvisAnimationController * getAnimationController() const;

private:
    Ui::QvisMainManager *ui;

    QByteArray saveState(int version = 0) const;
    bool restoreState(const QByteArray &state, int version = 0);

    // Used when saving/restoring state.
    bool mRestoringState {false};
    int  mCurrentVersion {1};
};

#endif // QVISMAINMANAGER_H
