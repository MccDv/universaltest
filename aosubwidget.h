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
    UlError err;

    QString mFuncName;
    int mUtFunction;
    int mCurGroup;
    bool mUseTimer;
    bool mModeChanged;

    DaqDeviceHandle mDaqDeviceHandle;
    QString mDevUID;
    QString mDevName;

    DaqOutChanDescriptor chanDescriptors[32];
    AOutFlag mAoFlags;
    DaqOutScanFlag mDaqoFlags;
    ScanOption mScanOptions;
    AOutScanFlag mFunctionFlag;
    Range mRange;

    TriggerType mTriggerType;
    int mTrigChannel;
    unsigned int mRetrigCount;
    double mTrigLevel;
    double mTrigVariance;

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

    bool mFixedRange;
    bool showIndex;
    bool mRunning;
    bool mGoTmrIsRunning;
    bool mStopOnStart;
    int mChanCount;
    long long mPlotIndex;
    long long mPlotCount;
    bool mPlot;
    int mPlotChan;
    int mLowChan, mHighChan;
    int mSamplesPerChan;
    bool mStatusTimerEnabled;
    bool mCancelAOut;

    double *buffer;

    QList<DigitalPortType> validPorts;
    QHash<DigitalPortType, int> portBits;

    int mBlockSize;
    long long mBufSize;
    long long mFinalCount;
    int mAoResolution;
    int mDioResolution;

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

    void setUiForGroup();
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

    void getDataValues();

signals:
    //void dataReady(unsigned long long dataCount, unsigned long long eventParam);
};

#endif // AOSUBWIDGET_H
