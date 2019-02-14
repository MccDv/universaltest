#ifndef AOSUBWIDGET_H
#define AOSUBWIDGET_H

#include <QWidget>
#include <QVariant>
#include <QVector>
#include <QMdiSubWindow>
#include <QTimer>
#include <QTime>
#include "mainwindow.h"
#include "uldaq.h"
#include "qcustomplot.h"
#include "queuedialog.h"
#include "datamanager.h"
#include "dataselectdialog.h"
#include "qmath.h"
#include "tgmath.h"
#include "testutilities.h"
#include "errordialog.h"

namespace Ui {
class AoSubWidget;
}

class AoSubWidget : public QWidget
{
    Q_OBJECT

public:
    explicit AoSubWidget(QWidget *parent = 0);
    ~AoSubWidget();
    void keyPressEvent(QKeyEvent *event);
    static void eventCallback(DaqDeviceHandle, DaqEventType, unsigned long long, void*);
    void callbackHandler(DaqEventType eventType, unsigned long long eventData);

private slots:
    void initDeviceParams();
    void updateParameters();
    void updateText(QString infoText);

    void functionChanged(int utFunction);
    void groupChanged(int newGroup);
    void updateValueBox();
    void onClickCmdGo();
    void onClickCmdStop();
    void runSelectedFunc();
    void checkStatus();
    void showPlotWindow(bool);
    void showQueueConfig();
    void showDataGen();
    void swStopScan();
    void dataDialogResponse();
    void queueDialogResponse();
    void runEventSetup();
    //void showData(unsigned long long dataCount, unsigned long long eventParams);

private:
    Ui::AoSubWidget *ui;
    MainWindow* getMainWindow();
    MainWindow *mMainWindow;
    QTimer *tmrCheckStatus;
    QueueDialog *queueSetup;
    DataSelectDialog *dataSelectDlg;
    UlError err = ERR_NO_ERROR;

    QString mFuncName = "";
    int mUtFunction = 0;
    int mCurGroup = 0;
    bool mUseTimer = false;
    bool mModeChanged = false;

    DaqDeviceHandle mDaqDeviceHandle = 0;
    QString mDevUID = "";
    QString mDevName = "";

    DaqOutChanDescriptor chanDescriptors[32];
    AOutFlag mAoFlags = AOUT_FF_DEFAULT;
    DaqOutScanFlag mDaqoFlags = DAQOUTSCAN_FF_DEFAULT;
    ScanOption mScanOptions = SO_DEFAULTIO;
    AOutScanFlag mFunctionFlag = AOUTSCAN_FF_DEFAULT;
    Range mRange = BIP5VOLTS;

    TriggerType mTriggerType = TRIG_NONE;
    int mTrigChannel = 0;
    unsigned int mRetrigCount = 0;
    double mTrigLevel = 0.0;
    double mTrigVariance = 0.0;

    bool mUseGetStatus = true;
    bool mAutoStop = true;
    bool mUseWait = false;
    double mWaitTime = 0.0;
    int mPrintResolution = 4;
    DaqEventType mEventType = DE_NONE;
    unsigned long long mEventParams = 0;
    DaqEventType ALL_EVENT_TYPES =
            (DaqEventType)(DE_ON_DATA_AVAILABLE
            | DE_ON_END_OF_INPUT_SCAN
            | DE_ON_END_OF_OUTPUT_SCAN
            | DE_ON_INPUT_SCAN_ERROR
            | DE_ON_OUTPUT_SCAN_ERROR);

    bool mFixedRange = false;
    bool showIndex = false;
    bool mRunning = false;
    bool mGoTmrIsRunning = false;
    bool mStopOnStart = false;
    int mChanCount = 0;
    long long mPlotIndex = 0;
    long long mPlotCount = 0;
    bool mPlot = false;
    int mPlotChan = 0;
    int mLowChan, mHighChan;
    int mSamplesPerChan = 0;
    int mTextIndex = 0;
    bool mStatusTimerEnabled = true;
    bool mCancelAOut = false;

    double *buffer;

    QList<DigitalPortType> validPorts;
    QHash<DigitalPortType, int> portBits;

    int mBlockSize = 0;
    long long mBufSize = 0;
    long long mFinalCount = 0;
    int mAoResolution = 0;
    int mDioResolution = 0;

    QHash<int, int>             mChanList;
    QHash<int, DaqOutChanType>  mChanTypeList;
    QHash<int, Range>           mRangeList;

    double mDefaultFSRange;
    QVector<double> xValues;
    QVector<QVector<double>> yChans;
    QHash<int, int>         mWaves;
    QHash<int, int>         mCycles;
    QHash<int, double>      mAmplitude;
    QHash<int, double>      mOffset;
    QHash<int, int>         mResolution;
    QHash<int, double>      mFSRange;
    QHash<int, bool>        mBipolar;
    QHash<int, int>         mDataScale;

    //void setUiForGroup();
    void setUiForFunction();
    void runSetTriggerFunc();
    void runEventDisable(DaqEventType eventType);
    void runEventEnable(DaqEventType eventType, unsigned long long eventParam);
    void runAOutFunc();
    void runAOutArray();
    void runAOutScanFunc();
    void runDaqOutScanFunc();
    UlError stopScan(long long curCount, long long curIndex);
    void setupPlot(QCustomPlot *dataPlot, int chanCount);
    void plotScan(unsigned long long currentCount, long long currentIndex, int blockSize);
    void updatePlot();

    void configWaves();
    void updateData();
    void getDataValues(bool newBuffer);

signals:
    //void dataReady(unsigned long long dataCount, unsigned long long eventParam);
};

#endif // AOSUBWIDGET_H
