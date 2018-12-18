#ifndef CTRSUBWIDGET_H
#define CTRSUBWIDGET_H

#include <QWidget>
#include <QVariant>
#include <QMdiSubWindow>
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
    UlError err;

    int mFontSize;
    QString mFuncName;
    int mUtFunction;
    int mCurGroup;
    bool mUseTimer;

    DaqDeviceHandle mDaqDeviceHandle;
    QString mDevUID;
    QString mDevName;

    int mSamplesPerChan;
    int mCtrResolution;
    int mChanCount;
    int mPrintResolution;
    bool mCalcTime;
    bool mInitPlot;
    TriggerType mTriggerType;
    int mTrigChannel;
    unsigned int mRetrigCount;
    double mTrigLevel;
    double mTrigVariance;
    ScanOption mScanOptions;
    CInScanFlag mCInFlags;
    CounterMeasurementMode mMeasMode;
    CounterMeasurementType mMeasType;
    CounterTickSize mTickSize;

    bool mUseGetStatus;
    bool mUseWait;
    double mWaitTime;
    DaqEventType mEventType;
    unsigned long long mEventParams;
    DaqEventType ALL_EVENT_TYPES =
            (DaqEventType)(DE_ON_DATA_AVAILABLE
            | DE_ON_END_OF_INPUT_SCAN
            | DE_ON_END_OF_OUTPUT_SCAN
            | DE_ON_INPUT_SCAN_ERROR
            | DE_ON_OUTPUT_SCAN_ERROR);

    long long mPlotIndex;
    long long mPlotCount;
    bool mPlot;
    int mPlotChan;
    QHash<int, bool>            mPlotList;
    bool mStatusTimerEnabled;
    bool mRunning;
    bool mGoTmrIsRunning;
    bool mStopOnStart;

    unsigned long long *buffer;
    QVector<double> xValues;
    QVector<QVector<double>> yChans;
    int mBlockSize;
    long long mBufSize;
    long long mFinalCount;

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
