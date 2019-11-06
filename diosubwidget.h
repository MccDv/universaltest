#ifndef DIOSUBWIDGET_H
#define DIOSUBWIDGET_H

#define AUX1_INDEX   16
#define AUX2_INDEX   32
#define AUX3_INDEX   48

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
    int numBitCheckboxes = 144;
    QCheckBox *chkBit[144]={};
    QRadioButton *rbPlotSel[8]={};
    QList<QCheckBox*> portCheckBoxes;
    UlError err;

    int mFontSize;
    QString mFuncName = "";
    int mUtFunction = 0;
    int mCurGroup = 0;
    bool mUseTimer = false;

    DaqDeviceHandle mDaqDeviceHandle = 0;
    QString mDevUID = "";
    QString mDevName = "";

    int mSamplesPerChan = 0;
    DInScanFlag mDInScanFlag = DINSCAN_FF_DEFAULT;
    DOutScanFlag mDOutScanFlag = DOUTSCAN_FF_DEFAULT;
    ScanOption mScanOptions = SO_DEFAULTIO;

    TriggerType mTriggerType = TRIG_NONE;
    int mTrigChannel = 0;
    unsigned int mRetrigCount = 0;
    double mTrigLevel = 0.0;
    double mTrigVariance = 0.0;

    bool mUseGetStatus = true;
    bool mAutoStop = true;
    bool mUseWait = false;
    double mWaitTime = 0.0;
    DaqEventType mEventType = DE_NONE;
    unsigned long long mEventParams = 0;
    DaqEventType ALL_EVENT_TYPES =
            (DaqEventType)(DE_ON_DATA_AVAILABLE
            | DE_ON_END_OF_INPUT_SCAN
            | DE_ON_END_OF_OUTPUT_SCAN
            | DE_ON_INPUT_SCAN_ERROR
            | DE_ON_OUTPUT_SCAN_ERROR);

    bool mRunning = false;
    bool mGoTmrIsRunning = false;
    bool mStopOnStart = false;
    long long mPlotIndex = 0;
    long long mPlotCount = 0;
    bool mPlot = false;
    int mPlotChan = 0;
    bool mStatusTimerEnabled = false;
    bool mCancelDScan = false;
    int mChanCount = 0;
    //int mTotalSamples;
    int mTextIndex = 0;
    int mPrintResolution = 0;
    bool mInitPlot = false;
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

    int mBlockSize = 0;
    long long mBufSize = 0;
    long long mFinalCount = 0;
    int mDioResolution = 0;
    bool mHasAiExp32;

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
    bool disableExpDigital(bool prompt);
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
