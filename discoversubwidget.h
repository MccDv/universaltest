#ifndef DISCOVERSUBWIDGET_H
#define DISCOVERSUBWIDGET_H

#define MAX_DEV_COUNT  100
#include <QWidget>
#include <QHash>
#include "mainwindow.h"
#include "uldaq.h"
#include "unitest.h"

namespace Ui {
class DiscoverSubWidget;
}

class DiscoverSubWidget : public QWidget
{
    Q_OBJECT

public:
    explicit DiscoverSubWidget(QWidget *parent = 0);
    ~DiscoverSubWidget();

private slots:
    void updateParameters();
    void functionChanged(int utFunction);
    void groupChanged(int newGroup);
    void runSelectedFunc();
    void on_listWidget_itemSelectionChanged();
    void on_cmdCreate_clicked();
    void on_cmdRelease_clicked();
    void on_cmdDisconnect_clicked();
    void on_cmdConnect_clicked();
    void on_actionRefresh_Devices_triggered();
    //void on_listWidget_clicked(const QModelIndex &index);

private:
    Ui::DiscoverSubWidget *ui;
    MainWindow* getMainWindow();
    MainWindow *mMainWindow;
    UlError err;
    DaqDeviceHandle mDaqDeviceHandle;
    QString mDevUID;
    QString mDevName;

    QHash<QString, DaqDeviceHandle> devList;
    struct DaqDeviceDescriptor devDescriptors[MAX_DEV_COUNT];
    QString mUidString;
    int mDevListCount;

    QString mFuncName;
    int mUtFunction;
    int mCurGroup;
    bool mUseTimer;

    void setUiForGroup();
    void setUiForFunction();

    void updateList();
    void refreshSelDevice();
    void updateConnectionStatus();
};

#endif // DISCOVERSUBWIDGET_H
