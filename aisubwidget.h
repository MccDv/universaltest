#ifndef AISUBWIDGET_H
#define AISUBWIDGET_H

#include <QWidget>
#include <QVariant>
#include <QVector>
#include <QMdiSubWindow>
#include <QTimer>
#include <QTime>
#include "mainwindow.h"
#include "uldaq.h"
#include "qcustomplot.h"
#include "qmath.h"
#include "testutilities.h"
#include "queuedialog.h"
#include "errordialog.h"

namespace Ui {
class AiSubWidget;
}

class AiSubWidget : public QWidget
{
    Q_OBJECT

public:
    explicit AiSubWidget(QWidget *parent = 0);
    ~AiSubWidget();
    static void eventCallback(DaqDeviceHandle, DaqEventType, unsigned long long, void*);
    void callbackHandler(DaqEventType eventType, unsigned long long eventData);

private slots:
    void initDeviceParams();
    void updateParameters();
    void updateText(QString infoText);
    void functionChanged(int utFunction);
    void groupChanged(int newGroup);

    void onClickCmdGo();
    void onClickCmdStop();

    void runSelectedFunc();
    void checkStatus();

    void showPlotWindow(bool);
    void showQueueConfig();
    void showDataGen();
    void swStopScan();
    void queueDialogResponse();
    void runEventSetup();
    void replot();
    void plotSelect();
    void showData(unsigned long long curTotalCount, unsigned long long curIndex, int blockSize);

private:
    Ui::AiSubWidget *ui;
    MainWindow* getMainWindow();
    MainWindow *mMainWindow;
    QTimer *tmrCheckStatus;
    QRadioButton *rbPlotSel[8]={};
    QueueDialog *queueSetup;
    ErrorDialog mErrorDlg;
    UlError err;

    QString mFuncName;
    int mUtFunction;
    int mCurGroup;
    bool mUseTimer;
    bool mOneSampPerForTotalSamps;

    DaqDeviceHandle mDaqDeviceHandle;
    QString mDevUID;
    QString mDevName;

    int mSamplesPerChan;
    long mTotalRead;
    AiInputMode mInputMode;
    AInFlag mAiFlags;
    struct DaqInChanDescriptor chanDescriptors[32];
    DaqInScanFlag mDaqiFlags;
    ScanOption mScanOptions;
    AInScanFlag mFunctionFlag;
    Range mRange;
    TempScale mScale;
    TInFlag mTiFlags;
    TInArrayFlag mTiArrFlags;

    TriggerType mTriggerType;
    DaqInChanType mTrigChanType;
    Range mTrigRange;
    int mTrigChannel;
    unsigned int mRetrigCount;
    double mTrigLevel;
    double mTrigVariance;
    int mAiResolution;

    QHash<int, int>             mChanList;
    QHash<int, DaqInChanType>   mChanTypeList;
    QHash<int, AiInputMode>     mModeList;
    QHash<int, Range>           mRangeList;
    QHash<int, int>             mResolution;
    QHash<int, double>          mFSRange;
    QHash<int, bool>            mBipolar;
    QHash<int, int>             mDataScale;

    QHash<int, bool>            mPlotList;

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

    bool showIndex;
    bool mRunning;
    bool mGoTmrIsRunning;
    bool mStopOnStart;
    int mChanCount;
    long long mPlotIndex;
    long long mPlotCount;
    bool mPlot;
    int mPlotChan;
    bool mStatusTimerEnabled;

    double *buffer;
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
    void runAInFunc();
    void runTInFunc();
    void runTInArray();
    void runAInScanFunc();
    void runDaqInScanFunc();
    UlError stopScan(long long perChan, long long curCount, long long curIndex);
    void setupPlot(QCustomPlot *dataPlot, int chanCount);
    void printData(unsigned long long currentCount, long long currentIndex, int blockSize);
    void plotScan(unsigned long long currentCount, long long currentIndex, int blockSize);
    void updatePlot();

    void setupQueue();

signals:
    void dataReady(unsigned long long curTotalCount, unsigned long long curIndex, int blockSize);
};

#endif // AISUBWIDGET_H
