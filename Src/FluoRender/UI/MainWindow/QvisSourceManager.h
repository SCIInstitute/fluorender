#ifndef QVISSOURCEMANAGER_H
#define QVISSOURCEMANAGER_H

#include "QvisInterfaceBase.h"

#ifdef QT_CREATOR_ONLY
  class SourceAgent;
#else
  namespace fluo {
    class SourceAgent;
  }
  using namespace fluo;
#endif

#include <QWidget>

namespace Ui {
class QvisSourceManager;
}

// QvisSourceManager
class QvisSourceManager : public QWidget, public QvisInterfaceBase
{
    Q_OBJECT

public:
    explicit QvisSourceManager(QWidget *parent = nullptr);
    ~QvisSourceManager();

    virtual void SetAgent(InterfaceAgent* agent) override;
    virtual void updateWindow(bool doAll = true) override { Q_UNUSED(doAll); };

signals:
    void postInformationalMessage(QString message);
    void postWarningMessage(QString message);
    void postErrorMessage(QString message);
    void clearMessage();

    void projectDirectoryChanged(QString directory);

public slots:
    // Sources
    void ProjectOpenClicked(QString filename = "");
    void ProjectSaveClicked();
    void ProjectSaveAsClicked();
    void VolumeOpenClicked(QString filename = "");
    void   MeshOpenClicked(QString filename = "");

private:
    Ui::QvisSourceManager *ui;

    // Project management.
    QString mProjectName;

    SourceAgent* mAgent {nullptr};
};

#endif // QVISSOURCEMANAGER_H
