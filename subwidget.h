#ifndef SUBWIDGET_H
#define SUBWIDGET_H

#include <QMdiSubWindow>
#include <QWidget>
#include <QVariant>
#include <QDateTime>
#include <QTimer>
#include <QTime>
#include "uldaq.h"
#include "mainwindow.h"

namespace Ui {
class subWidget;
}

class subWidget : public QWidget
{
    Q_OBJECT

public:
    explicit subWidget(QWidget *parent = 0);
    ~subWidget();
    void keyPressEvent(QKeyEvent *event);

private slots:
    void updateParameters();
    void runSelectedFunc();
    void functionChanged(int utFunction);
    void groupChanged(int newGroup);
    void setConfigItemsForType();
    void setConfiguration();
    void setMiscFunction();
    void onStopCmd();
    void memReadWrite();

    void showPlotWindow(bool);
    void showQueueConfig();
    void showDataGen();
    void initDeviceParams();
    void updateText(QString infoText);
    void swStopScan();
    void runEventSetup();

private:
    Ui::subWidget *ui;
    MainWindow* getMainWindow();
    MainWindow *mMainWindow;
    DaqDeviceHandle mDaqDeviceHandle = -1;
    UlError err = ERR_NO_ERROR;
    QString mDevUID = "";
    QString mDevName = "";
    QString mFuncName = "";
    bool mUseTimer = false;
    bool mGoTmrIsRunning = false;

    //int AO_CFG_CHAN_SENSE_MODE = 2;
    /*typedef enum
    {
        AOSM_DISABLED = 1,
        AOSM_ENABLED  = 2
    }AOutSenseMode;
    DevConfigItem DEV_CFG_RESET = (DevConfigItem)4;*/

    int mUtFunction = 0;
    bool showIndex = false;
    int mCurGroup = 0;
    bool mSigsConfigured = false;

    int mPrintResolution = 3;
    int mHexResolution = 4;
    bool mShowHex = false;
    QString showInfo(int InfoType, int infoItem, QString showItem);
    QString showInfoDbl(int infoType, int infoItem, QString showItem);
    QString showInfoStr(int infoType, int infoItem, QString showItem);
    QString showInfoMem(MemRegion memRegion);
    QString showConfig(int cfgType, int cfgItem, QString showItem);
    QString showConfigDbl(int cfgType, int cfgItem, QString showItem);
    QString showConfigStr(int cfgType, int cfgItem, QString showItem);
    void readInfo();
    void readConfig();
    void readMem();
    void memRead();
    void memWrite();
    void setUiForGroup();
    void setUiForFunction();
    void getErrorMessage();
    UlError ctrGetConfig(DaqDeviceHandle daqDeviceHandle, CtrConfigItem configItem, unsigned int index, long long &configValue);
    UlError ctrSetConfig(DaqDeviceHandle daqDeviceHandle, CtrConfigItem configItem, unsigned int index, long long configValue);
};

#endif // SUBWIDGET_H
