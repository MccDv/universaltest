#ifndef CTRSUBWIDGET_H
#define CTRSUBWIDGET_H

#include <QMdiSubWindow>
#include <QWidget>
#include <QVariant>
#include <QVector>
#include <QTimer>
#include <QTime>
#include "mainwindow.h"
#include <QCheckBox>
#include "qcustomplot.h"
#include "uldaq.h"
#include "testutilities.h"
#include "queuedialog.h"

namespace Ui {
class CtrSubWidget;
}

class CtrSubWidget : public QWidget
{
    Q_OBJECT

public:
    explicit CtrSubWidget(QWidget *parent = 0);
    ~CtrSubWidget();
    void keyPressEvent(QKeyEvent *event);
    static void eventCallback(DaqDeviceHandle, DaqEventType, unsigned long long, void*);
    void callbackHandler(DaqEventType eventType, unsigned long long eventData);

private slots:
    void initDeviceParams();
    void updateParameters();
    void updateText(QString infoText);

    void functionChanged(int utFunction);
    void groupChanged(int newGroup);
    void onClickCmdGo();
    void parseMeasMode();
    void runCConfigScan();
    void checkStatus();
    void checkScanStatus();
    void checkTmrStatus();
    void showPlotWindow(bool);
    void swStopScan();
    void runSelectedFunc();
    void stopGoTimer();
    void runEventSetup();
    void replot();
    void plotSelect();
    void readRegister();
    void showDataGen();
    void showQueueConfig();
    void showData(unsigned long long curTotalCount, unsigned long long curIndex, int blockSize);

private:
    Ui::CtrSubWidget *ui;
    MainWindow* getMainWindow();
    MainWindow *mMainWindow;
    QList<QCheckBox *> ctrCheckBoxes;
    QRadioButton *rbPlotSel[8]={};
    QTimer *tmrCheckStatus;
    UlError err = ERR_NO_ERROR;

    int mFontSize = 0;
    QString mFuncName = "";
    int mUtFunction = 0;
    int mCurGroup = 0;
    bool mUseTimer = false;
    bool scanIncludes0 = false;

    DaqDeviceHandle mDaqDeviceHandle = 0;
    QString mDevUID = "";
    QString mDevName = "";

    int mSamplesPerChan = 0;
    int mCtrResolution = 0;
    int mChanCount = 0;
    int mPrintResolution = 3;
    int mHexResolution = 4;
    bool mShowHex = false;
    bool mCalcTime = false;
    bool mInitPlot = false;
    TriggerType mTriggerType = TRIG_NONE;
    int mTrigChannel = 0;
    unsigned int mRetrigCount = 0;
    double mTrigLevel = 0.0;
    double mTrigVariance = 0.0;
    ScanOption mScanOptions = SO_DEFAULTIO;
    CInScanFlag mCInFlags = CINSCAN_FF_DEFAULT;
    CounterMeasurementMode mMeasMode = CMM_DEFAULT;
    CounterMeasurementType mMeasType = CMT_COUNT;
    CounterTickSize mTickSize = CTS_TICK_20PT83ns;

    bool mUseGetStatus = true;
    bool mAutoStop = true;
    bool mUseWait = false;
    bool mCalcPeriod = false;
    double mWaitTime = 0.0;
    DaqEventType mEventType = DE_NONE;
    unsigned long long mEventParams = 0;
    DaqEventType ALL_EVENT_TYPES =
            (DaqEventType)(DE_ON_DATA_AVAILABLE
            | DE_ON_END_OF_INPUT_SCAN
            | DE_ON_END_OF_OUTPUT_SCAN
            | DE_ON_INPUT_SCAN_ERROR
            | DE_ON_OUTPUT_SCAN_ERROR);

    long long mPlotIndex = 0;
    long long mPlotCount = 0;
    bool mPlot = false;
    int mPlotChan = 0;
    QHash<int, bool>            mPlotList;
    bool mStatusTimerEnabled = false;
    bool mRunning = false;
    bool mGoTmrIsRunning = false;
    bool mStopOnStart = false;

    unsigned long long *buffer;
    QVector<double> xValues;
    QVector<QVector<double>> yChans;
    int mBlockSize = 0;
    long long mBufSize = 0;
    long long mFinalCount = 0;

    //void setUiForGroup();
    void setUiForFunction();
    void runSetTriggerFunc();
    void runEventDisable(DaqEventType eventType);
    void runEventEnable(DaqEventType eventType, unsigned long long eventParam);
    void runCInFunc();
    void runClearFunc();
    void runCLoadFunc();
    void runCInScan();
    UlError stopScan(long long perChan, long long curCount, long long curIndex);
    void userScanStop();
    void runTimerOut();
    void runTimerStop();
    void setupPlot(QCustomPlot *dataPlot, int chanCount);
    void printData(unsigned long long currentCount, long long currentIndex, int blockSize);
    void plotScan(unsigned long long currentCount, long long currentIndex, int blockSize);
    void updatePlot();

signals:
    void dataReady(unsigned long long curTotalCount, unsigned long long curIndex, int blockSize);
};

#endif // CTRSUBWIDGET_H
