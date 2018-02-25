#ifndef CHILDWINDOW_H
#define CHILDWINDOW_H

#include <QWidget>
#include <QtGui>
#include <QMdiSubWindow>
#include <QTimer>
#include "tmrdialog.h"
#include "subwidget.h"
#include "unitest.h"
//#include "plotwindow.h"
//#include "aisubwidget.h"
#include "ul.h"

namespace Ui {
    class ChildWindow;
}

class ChildWindow : public QMdiSubWindow
{
    Q_OBJECT
    Q_PROPERTY(DaqDeviceHandle devHandle READ devHandle WRITE setDevHandle NOTIFY devHandleChanged)
    Q_PROPERTY(QString devName READ devName WRITE setDevName NOTIFY devNameChanged)
    Q_PROPERTY(QString devUID READ devUID WRITE setDevUID NOTIFY devUIDChanged)

    Q_PROPERTY(AiInputMode inputMode READ inputMode WRITE setInputMode NOTIFY inputModeChanged)
    Q_PROPERTY(AInFlag aiFlags READ aiFlags WRITE setAiFlags NOTIFY aiFlagsChanged)
    Q_PROPERTY(AOutFlag aoFlags READ aoFlags WRITE setAoFlags NOTIFY aoFlagsChanged)
    Q_PROPERTY(DaqOutScanFlag daqOutFlag READ daqOutFlag WRITE setDaqOutFlag NOTIFY daqOutFlagChanged)
    Q_PROPERTY(DaqInScanFlag daqiFlags READ daqiFlags WRITE setDaqiFlags NOTIFY daqiFlagsChanged)
    Q_PROPERTY(CInScanFlag ciFlags READ ciFlags WRITE setCiFlags NOTIFY ciFlagsChanged)
    Q_PROPERTY(ScanOption scanOptions READ scanOptions WRITE setScanOptions NOTIFY scanOptionsChanged)
    Q_PROPERTY(TriggerType triggerType READ triggerType WRITE setTriggerType NOTIFY triggerTypeChanged)
    Q_PROPERTY(int trigChannel READ trigChannel WRITE setTrigChannel NOTIFY trigChannelChanged)
    Q_PROPERTY(unsigned int retrigCount READ retrigCount WRITE setRetrigCount NOTIFY retrigCountChanged)
    Q_PROPERTY(double trigLevel READ trigLevel WRITE setTrigLevel NOTIFY trigLevelChanged)
    Q_PROPERTY(double trigVariance READ trigVariance WRITE setTrigVariance NOTIFY trigVarianceChanged)

    Q_PROPERTY(bool tmrEnabled READ tmrEnabled WRITE setTmrEnabled NOTIFY tmrEnabledChanged)
    Q_PROPERTY(int tmrInterval READ tmrInterval WRITE setTmrInterval NOTIFY tmrIntervalChanged)
    Q_PROPERTY(bool tmrStopOnStart READ tmrStopOnStart WRITE setTmrStopOnStart NOTIFY tmrStopOnStartChanged)
    Q_PROPERTY(bool tmrRunning READ tmrRunning WRITE setTmrRunning NOTIFY tmrRunningChanged)
    Q_PROPERTY(bool showPlot READ showPlot WRITE setShowPlot NOTIFY showPlotChanged)
    Q_PROPERTY(bool statusEnabled READ statusEnabled WRITE setStatusEnabled NOTIFY statusEnabledChanged)
    Q_PROPERTY(bool waitEnabled READ waitEnabled WRITE setWaitEnabled NOTIFY waitEnabledChanged)
    Q_PROPERTY(double waitTime READ waitTime WRITE setWaitTime NOTIFY waitTimeChanged)

    Q_PROPERTY(DaqEventType eventTypes READ eventTypes WRITE setEventTypes NOTIFY eventTypesChanged)
    Q_PROPERTY(unsigned long long eventParams READ eventParams WRITE setEventParams NOTIFY eventParamsChanged)

public:
    explicit ChildWindow(QWidget *parent = 0, UtFunctionGroup funcGroup = FUNC_GROUP_CONFIG);
    ~ChildWindow();
    void closeEvent(QCloseEvent *event);

    //property sets:

    void setStatusEnabled(bool statusEnabled)
    {
        mStatusEnabled = statusEnabled;
        emit statusEnabledChanged(statusEnabled);
    }

    void setWaitEnabled(bool waitIsEnabled)
    {
        mWaitEnabled = waitIsEnabled;
        emit waitEnabledChanged(waitIsEnabled);
    }

    void setWaitTime(double timeToWait)
    {
        mWaitTime = timeToWait;
        emit waitTimeChanged(timeToWait);
    }

    void setEventTypes(DaqEventType eventTypes)
    {
        mEventTypes = eventTypes;
        emit eventTypesChanged(eventTypes);
    }

    void setEventParams(unsigned long long eventParams)
    {
        mEventParams = eventParams;
        emit eventParamsChanged(eventParams);
    }

    void setTmrEnabled(bool tmrEnabled)
    {
        mTmrEnabled = tmrEnabled;
        emit tmrEnabledChanged(tmrEnabled);
    }

    void setTmrInterval(int tmrInterval)
    {
        mTmrInterval = tmrInterval;
        emit tmrIntervalChanged(tmrInterval);
    }

    void setTmrRunning(bool runTimer)
    {
        mTmrRunning = runTimer;
        emit tmrRunningChanged(runTimer);
    }

    void setTmrStopOnStart(bool stopOnStart)
    {
        mStopOnStart = stopOnStart;
        emit tmrStopOnStartChanged(stopOnStart);
    }

    void setDevHandle(DaqDeviceHandle devHandle)
    {
        mDevHandle = devHandle;
        emit devHandleChanged(devHandle);
    }

    void setDevName(QString devName)
    {
        mDevName = devName;
        emit devNameChanged(devName);
    }

    void setDevUID(QString devUID)
    {
        mDevUID = devUID;
        emit devUIDChanged(devUID);
    }

    void setInputMode(AiInputMode inputMode)
    {
        mInputMode = inputMode;
        emit inputModeChanged(inputMode);
    }

    void setAiFlags(AInFlag aiFlags)
    {
        mAiFlags = aiFlags;
        emit aiFlagsChanged(aiFlags);
    }

    void setAoFlags(AOutFlag aoFlags)
    {
        mAoFlags = aoFlags;
        emit aoFlagsChanged(aoFlags);
    }

    void setDaqOutFlag(DaqOutScanFlag daqOutFlags)
    {
        mDaqOutFlags = daqOutFlags;
        emit daqOutFlagChanged(daqOutFlags);
    }

    void setDaqiFlags(DaqInScanFlag daqiFlags)
    {
        mDaqiFlags = daqiFlags;
        emit daqiFlagsChanged(daqiFlags);
    }

    void setCiFlags(CInScanFlag ciFlag)
    {
        mCiFlags = ciFlag;
        emit ciFlagsChanged(ciFlag);
    }

    void setScanOptions(ScanOption scanOptions)
    {
        mScanOptions = scanOptions;
        emit scanOptionsChanged(scanOptions);
    }

    void setTriggerType(TriggerType trigType)
    {
        mTriggerType = trigType;
        emit triggerTypeChanged(trigType);
    }

    void setTrigChannel(int trigChan)
    {
        mTrigChannel = trigChan;
        emit trigChannelChanged(trigChan);
    }

    void setTrigVariance(double trigVariance)
    {
        mTrigVariance = trigVariance;
        emit trigVarianceChanged(trigVariance);
    }

    void setTrigLevel(double trigLevel)
    {
        mTrigLevel = trigLevel;
        emit trigLevelChanged(trigLevel);
    }

    void setRetrigCount(unsigned int retrigCount)
    {
        mRetrigCount = retrigCount;
        emit retrigCountChanged(retrigCount);
    }

    void setShowPlot(bool showIt)
    {
        mShowPlot = showIt;
        emit showPlotChanged(showIt);
    }

    void setCurFunction(int utFunction);

    void setCurRange(Range curRange);

    //PlotWindow *plotWindow
    //property gets:

    bool statusEnabled() { return mStatusEnabled; }
    bool waitEnabled() { return mWaitEnabled; }
    double waitTime() { return mWaitTime; }

    DaqEventType eventTypes() { return mEventTypes; }
    unsigned long long eventParams() { return mEventParams; }

    bool tmrEnabled() { return mTmrEnabled; }
    int tmrInterval() { return mTmrInterval; }
    bool tmrRunning() { return mTmrRunning; }
    bool tmrStopOnStart() { return mStopOnStart; }
    bool showPlot() { return mShowPlot; }

    DaqDeviceHandle devHandle() { return mDevHandle; }
    QString devName() { return mDevName; }
    QString devUID() { return mDevUID; }

    AiInputMode inputMode() { return mInputMode; }
    AInFlag aiFlags() { return mAiFlags; }
    AOutFlag aoFlags() { return mAoFlags; }
    DaqOutScanFlag daqOutFlag() { return mDaqOutFlags; }
    DaqInScanFlag daqiFlags() { return mDaqiFlags; }
    CInScanFlag ciFlags() { return mCiFlags; }
    ScanOption scanOptions() { return mScanOptions; }
    TriggerType triggerType() { return mTriggerType; }
    int trigChannel() { return mTrigChannel; }
    unsigned int retrigCount() { return mRetrigCount; }
    double trigLevel() { return mTrigLevel; }
    double trigVariance() { return mTrigVariance; }

    void refreshBoardParams() { emit refreshBoads(); }
    void refreshSelectedFunc() { emit refreshData(); }

    int currentFunctionGroup() { return curFunctionGroup; }
    int currentFunction() { return curFunction; }
    Range getCurrentRange()  {return mRange; }

    void setUpTimer();
    void updateEventSetup();
    void disableEvents();
    void stopScan();
    void showDataConfig() { emit configData(); }
    void showQueueConfig() { emit configQueue(); }

private slots:
    void goTimerRun(bool enable);
    void tmrDialogResponse();

private:
    QString windowName;
    QWidget *subwidget;
    QTimer *tmrRunFunc;
    TmrDialog *tmrDialog;

    DaqDeviceHandle mDevHandle;
    QString mDevName;
    QString mDevUID;

    AiInputMode mInputMode;
    AInFlag mAiFlags;
    AOutFlag mAoFlags;
    DaqOutScanFlag mDaqOutFlags;
    DaqInScanFlag mDaqiFlags;
    CInScanFlag mCiFlags;
    ScanOption mScanOptions;
    TriggerType mTriggerType;
    int mTrigChannel;
    unsigned int mRetrigCount;
    double mTrigVariance;
    double mTrigLevel;

    bool mStatusEnabled;
    bool mWaitEnabled;
    double mWaitTime;
    DaqEventType mEventTypes;
    unsigned long long mEventParams;
    bool mTmrEnabled = false;
    bool mStopOnStart = false;
    int mTmrInterval;
    bool mTmrRunning = false;
    int curFunction;
    int curFunctionGroup;
    Range mRange;
    bool mShowPlot = false;

    void readWindowPosition();
    void writeWindowPosition();

signals:
    void refreshBoads();
    void refreshData();
    void statusEnabledChanged(bool);
    void waitEnabledChanged(bool);
    void waitTimeChanged(double);
    void eventTypesChanged(DaqEventType);
    void eventParamsChanged(unsigned long long);
    void eventsReadyForUpdate();
    void eventsReadyForDisable();

    void tmrEnabledChanged(bool);
    void tmrIntervalChanged(int);
    void tmrRunningChanged(bool);
    void tmrStopOnStartChanged(bool);

    void devHandleChanged(DaqDeviceHandle);
    void devNameChanged(QString);
    void devUIDChanged(QString);

    void inputModeChanged(AiInputMode);
    void aiFlagsChanged(long long);
    void aoFlagsChanged(long long);
    void daqiFlagsChanged(DaqInScanFlag);
    void daqOutFlagChanged(DaqOutScanFlag);
    void ciFlagsChanged(CInScanFlag);
    void scanOptionsChanged(ScanOption);
    void triggerTypeChanged(TriggerType);
    void trigChannelChanged(int);
    void trigVarianceChanged(double);
    void trigLevelChanged(double);
    void retrigCountChanged(unsigned int);
    void stopScanRequest();
    void configData();
    void configQueue();

    void funcChanged(int);
    void groupChanged(int);
    //void changeRange(int);
    void showPlotChanged(bool);
    void rangeChanged(Range newRange);
    //PlotWindow *
};

#endif // CHILDWINDOW_H
