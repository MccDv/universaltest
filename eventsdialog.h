#ifndef EVENTSDIALOG_H
#define EVENTSDIALOG_H

#include <QDialog>
#include "ul.h"
#include "testutilities.h"

namespace Ui {
class EventsDialog;
}

class EventsDialog : public QDialog
{
    Q_OBJECT
    Q_PROPERTY(bool eventsEnabled READ eventsEnabled WRITE setEventsEnabled NOTIFY eventsEnabledChanged)
    Q_PROPERTY(bool checkStatusEnabled READ checkStatusEnabled WRITE setCheckStatusEnabled NOTIFY checkStatusEnabledChanged)
    Q_PROPERTY(bool waitForDone READ waitForDone WRITE setWaitForDone NOTIFY waitForDoneChanged)
    Q_PROPERTY(double waitTime READ waitTime WRITE setWaitTime NOTIFY waitTimeChanged)
    Q_PROPERTY(DaqEventType eventType READ eventType WRITE setEventType NOTIFY eventTypeChanged)
    Q_PROPERTY(unsigned long long eventParams READ eventParams WRITE setEventParams NOTIFY eventParamsChanged)

    //to do: functionType not needed - delete
    Q_PROPERTY(int functionType READ functionType WRITE setFunctionType NOTIFY functionTypeChanged)

public:
    explicit EventsDialog(QWidget *parent = 0, DaqDeviceHandle devHandle = 0);
    ~EventsDialog();

    void showEvent(QShowEvent *);

    void setWaitForDone(bool waitForIt)
    {
        mWaitForDone = waitForIt;
        emit waitForDoneChanged(waitForIt);
    }

    void setWaitTime(double timeToWait)
    {
        mWaitTime = timeToWait;
        emit waitTimeChanged(timeToWait);
    }

    void setEventsEnabled(bool enable)
    {
        mEventsEnabled = enable;
        emit eventsEnabledChanged(enable);
    }

    void setCheckStatusEnabled(bool enable)
    {
        mCheckStatusEnabled = enable;
        emit checkStatusEnabledChanged(enable);
    }

    void setEventType(DaqEventType eventType)
    {
        mEventType = eventType;
        emit eventTypeChanged(eventType);
    }

    void setEventParams(unsigned long long eventParams)
    {
        mEventParams = eventParams;
        emit eventParamsChanged(eventParams);
    }

    void setFunctionType(int funcType)
    {
        mFuncType = funcType;
        emit functionTypeChanged(funcType);
    }

    int functionType() { return mFuncType; }
    bool eventsEnabled() { return mEventsEnabled; }
    bool waitForDone() { return mWaitForDone; }
    double waitTime() { return mWaitTime; }
    bool checkStatusEnabled() { return mCheckStatusEnabled; }
    DaqEventType eventType() { return mEventType; }
    unsigned long long eventParams() { return mEventParams; }

private slots:
    void updateProperties();
    void syncControls();
    void updateEndOfInScan();
    void updateEndOfOutScan();
    void updateInScanError();
    void updateOutScanError();
    void updateDataAvailable();
    void updateParamValue();

private:
    Ui::EventsDialog *ui;

    int mFuncType;
    bool mEventsEnabled;
    bool mWaitForDone;
    double mWaitTime;
    bool mCheckStatusEnabled;
    DaqEventType mEventType;
    DaqDeviceHandle mDaqDeviceHandle;
    unsigned long long mEventParams;
    DaqEventType ALL_EVENT_TYPES =
            (DaqEventType)(DE_ON_DATA_AVAILABLE
            | DE_ON_END_OF_INPUT_SCAN
            | DE_ON_END_OF_OUTPUT_SCAN
            | DE_ON_INPUT_SCAN_ERROR
            | DE_ON_OUTPUT_SCAN_ERROR);

    bool mEvtEndInScanExists, mEvtEndInScanChanged;
    bool mEvtEndOutScanExists, mEvtEndOutScanChanged;
    bool mEvtInScanErrorExists, mEvtInScanErrorChanged;
    bool mEvtOutScanErrorExists, mEvtOutScanErrorChanged;
    bool mEvtOnDataExists, mEvtOnDataChanged;
    bool mEvtParamExists, mEvtParamChanged;

signals:
    void functionTypeChanged(int);
    void eventsEnabledChanged(bool);
    void checkStatusEnabledChanged(bool);
    void eventTypeChanged(DaqEventType);
    void eventParamsChanged(unsigned long long);
    void waitForDoneChanged(bool);
    void waitTimeChanged(double);
};

#endif // EVENTSDIALOG_H
