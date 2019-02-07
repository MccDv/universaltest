#ifndef DIOSUBWIDGET_H
#define DIOSUBWIDGET_H

#include <QWidget>
#include <QVariant>
#include <QVector>
#include <QMdiSubWindow>
#include <QTimer>
#include <QTime>
#include "mainwindow.h"
#include "uldaq.h"
#include "qcustomplot.h"
#include "datamanager.h"
#include "dataselectdialog.h"
#include "qmath.h"
#include "testutilities.h"

namespace Ui {
class DioSubWidget;
}

class DioSubWidget : public QWidget
{
    Q_OBJECT

public:
    explicit DioSubWidget(QWidget *parent = 0);
    ~DioSubWidget();
    void keyPressEvent(QKeyEvent *event);
    static void eventCallback(DaqDeviceHandle, DaqEventType, unsigned long long, void*);
    void callbackHandler(DaqEventType eventType, unsigned long long eventData);

private slots:
    void initDeviceParams();
    void updateParameters();
    void updateText(QString infoText);

    void functionChanged(int utFunction);
    void groupChanged(int newGroup);
    void togglePortSelection();
    void bitToggled(int bitNumber);
    void onClickCmdGo();
    void onClickCmdStop();
    void configureInputs();
    void configureOutputs();
    void runSelectedFunc();
    void checkStatus();
    void stopGoTimer();
    void showPlotWindow(bool);
    void showQueueConfig();
    void showDataGen();
    void swStopScan();
    void dataDialogResponse();
    void runEventSetup();
    void replot();
    void plotSelect();
    void showData(unsigned long long curTotalCount, unsigned long long curIndex, int blockSize);

private:
    Ui::DioSubWidget *ui;
    MainWindow* getMainWindow();
    MainWindow *mMainWindow;
    QTimer *tmrCheckStatus;
    DataSelectDialog *dataSelectDlg;
    int numBitCheckboxes = 128;
    QCheckBox *chkBit[128]={};
    QRadioButton *rbPlotSel[8]={};
    QList<QCheckBox*> portCheckBoxes;
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
    DInScanFlag mDInScanFlag;
    DOutScanFlag mDOutScanFlag;
    ScanOption mScanOptions;

    TriggerType mTriggerType;
    int mTrigChannel;
    unsigned int mRetrigCount;
    double mTrigLevel;
    double mTrigVariance;

    bool mUseGetStatus;
    bool mAutoStop;
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

    bool mRunning;
    bool mGoTmrIsRunning;
    bool mStopOnStart;
    long long mPlotIndex;
    long long mPlotCount;
    bool mPlot;
    int mPlotChan;
    bool mStatusTimerEnabled;
    bool mCancelDScan;
    int mChanCount;
    //int mTotalSamples;
    int mTextIndex;
    int mPrintResolution;
    bool mInitPlot;
    QHash<int, bool>            mPlotList;

    QList<DigitalPortType>      portList;
    QList<DigitalPortType>      validPorts;
    QHash<DigitalPortType, int> portBits;

    unsigned long long *buffer;
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

    int mBlockSize;
    long long mBufSize;
    long long mFinalCount;
    int mDioResolution;

    //void setUiForGroup();
    void setUiForFunction();
    void createBitBoxes();
    void runDConfigBit(DigitalPortType portType, int bitNum, DigitalDirection direction);
    void runDInFunc();
    void runDInArray();
    void runDOutArray();
    void runDBitInFunc();
    void runDOutFunc();
    void runDBitOutFunc(DigitalPortType portType, int bitNum, unsigned int bitValue);
    void runDInScanFunc();
    void runDOutScanFunc();
    void runSetTriggerFunc();
    void runEventDisable(DaqEventType eventType);
    void runEventEnable(DaqEventType eventType, unsigned long long eventParam);
    void readSingleBit();
    UlError stopScan(long long perChan, long long curCount, long long curIndex);
    void setupPlot(QCustomPlot *dataPlot, int chanCount);
    void plotScan(unsigned long long currentCount, long long currentIndex, int blockSize);
    void printData(unsigned long long currentCount, long long currentIndex, int blockSize);
    void updatePlot();
    void disableCheckboxInput(bool disableMouse);
    void setDefaultBits(DigitalPortType portType);
    void updateControlDefaults(bool setAllValidPorts);
    DigitalPortType parsePortFromBitIndex(DigitalPortType tempPort, int &bitNum);
    int getGridOffsetFromPort(DigitalPortType portType);
    void mapGridToPortBit(int gridIndex, DigitalPortType &portType, int &bitInPort);
    long long getIOConfigMask(int portIndex);

    void getDataValues(bool newBuffer);
    void configWaves();
    void updateData();

signals:
    void dataReady(unsigned long long curTotalCount, unsigned long long curIndex, int blockSize);
};

#endif // DIOSUBWIDGET_H
