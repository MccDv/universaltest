#include "eventsdialog.h"
#include "ui_eventsdialog.h"

EventsDialog::EventsDialog(QWidget *parent, DaqDeviceHandle devHandle) :
    QDialog(parent),
    ui(new Ui::EventsDialog)
{
    ui->setupUi(this);

    mDaqDeviceHandle = devHandle;
    mEvtEndInScanExists = false;
    mEvtEndInScanChanged = false;
    mEvtEndOutScanExists = false;
    mEvtEndOutScanChanged = false;
    mEvtInScanErrorExists = false;
    mEvtInScanErrorChanged = false;
    mEvtOutScanErrorExists = false;
    mEvtOutScanErrorChanged = false;
    mEvtOnDataExists = false;
    mEvtOnDataChanged = false;
    mEvtParamExists = false;
    mEvtParamChanged = false;

    connect(this, SIGNAL(accepted()), this, SLOT(syncControls()));
    connect(ui->chkGetStatus, SIGNAL(clicked(bool)), this, SLOT(updateProperties()));
    connect(ui->chkUseWait, SIGNAL(clicked(bool)), this, SLOT(updateProperties()));
    connect(ui->leWaitTimo, SIGNAL(textChanged(QString)), this, SLOT(updateProperties()));
    connect(ui->chkEndOfScan, SIGNAL(clicked(bool)), this, SLOT(updateEndOfInScan()));
    connect(ui->chkDataAvailable, SIGNAL(clicked(bool)), this, SLOT(updateDataAvailable()));
    connect(ui->chkScanError, SIGNAL(clicked(bool)), this, SLOT(updateInScanError()));
    connect(ui->chkEndOutScan, SIGNAL(clicked(bool)), this, SLOT(updateEndOfOutScan()));
    connect(ui->chkOutScanError, SIGNAL(clicked(bool)), this, SLOT(updateOutScanError()));
    connect(ui->leEventParams, SIGNAL(textChanged(QString)), this, SLOT(updateParamValue()));
}

EventsDialog::~EventsDialog()
{
    delete ui;
}

void EventsDialog::showEvent(QShowEvent *)
{
    bool eventValue, eventsEnabled;
    unsigned long long paramValue;

    if (getEndInScanEvent(mDaqDeviceHandle, eventValue)) {
        ui->chkEndOfScan->setChecked(eventValue);
        eventsEnabled = eventValue;
        mEvtEndInScanExists = true;
    }
    if (getEndOutScanEvent(mDaqDeviceHandle, eventValue)) {
        ui->chkEndOutScan->setChecked(eventValue);
        eventsEnabled |= eventValue;
        mEvtEndOutScanExists = true;
    }
    if (getInScanErrorEvent(mDaqDeviceHandle, eventValue)) {
        ui->chkScanError->setChecked(eventValue);
        eventsEnabled |= eventValue;
        mEvtInScanErrorExists = true;
    }
    if (getOutScanErrorEvent(mDaqDeviceHandle, eventValue)) {
        ui->chkOutScanError->setChecked(eventValue);
        eventsEnabled |= eventValue;
        mEvtOutScanErrorExists = true;
    }
    if (getDataAvailableEvent(mDaqDeviceHandle, eventValue)) {
        ui->chkDataAvailable->setChecked(eventValue);
        eventsEnabled |= eventValue;
        mEvtOnDataExists = true;
        bool paramExists = getEventParameter(mDaqDeviceHandle, paramValue);
        if (paramExists)
            ui->leEventParams->setText(QString("%1").arg(paramValue));
    }
    ui->chkEnableEvents->setChecked(eventsEnabled);
    ui->chkUseWait->setChecked(mWaitForDone);
    ui->leWaitTimo->setText(QString("%1").arg(mWaitTime));
    ui->chkGetStatus->setChecked(mCheckStatusEnabled);
}

void EventsDialog::updateEndOfInScan()
{
    mEvtEndInScanChanged = true;
}

void EventsDialog::updateDataAvailable()
{
    mEvtOnDataChanged = true;
}

void EventsDialog::updateEndOfOutScan()
{
    mEvtEndOutScanChanged = true;
}

void EventsDialog::updateInScanError()
{
    mEvtInScanErrorChanged = true;
}

void EventsDialog::updateOutScanError()
{
    mEvtOutScanErrorChanged = true;
}

void EventsDialog::updateParamValue()
{
    mEvtParamChanged = true;
}

void EventsDialog::syncControls()
{
    DaqEventType eventType = DE_NONE;
    bool eventValue, needsUpdate, eventsExist;

    eventsExist = mEvtEndInScanExists
            | mEvtEndOutScanExists
            | mEvtInScanErrorExists
            | mEvtOutScanErrorExists
            | mEvtOnDataExists;

    if (!(ui->chkEnableEvents->isChecked())) {
        eventType = ALL_EVENT_TYPES;
        if (eventsExist)
            setEventsByHandle(mDaqDeviceHandle, eventType, false);
    } else {
        if (mEvtEndInScanChanged) {
            eventType = DE_ON_END_OF_INPUT_SCAN;
            eventValue = ui->chkEndOfScan->isChecked();
            needsUpdate = ((eventValue) | (mEvtEndInScanExists));
            if (needsUpdate)
                setEventsByHandle(mDaqDeviceHandle, eventType, eventValue);
        }
        if (mEvtEndOutScanChanged) {
            eventType = DE_ON_END_OF_OUTPUT_SCAN;
            eventValue = ui->chkEndOutScan->isChecked();
            needsUpdate = ((eventValue) | (mEvtEndOutScanExists));
            if (needsUpdate)
                setEventsByHandle(mDaqDeviceHandle, eventType, eventValue);
        }
        if (mEvtInScanErrorChanged) {
            eventType = DE_ON_INPUT_SCAN_ERROR;
            eventValue = ui->chkScanError->isChecked();
            needsUpdate = ((eventValue) | (mEvtInScanErrorExists));
            if (needsUpdate)
                setEventsByHandle(mDaqDeviceHandle, eventType, eventValue);
        }
        if (mEvtOutScanErrorChanged) {
            eventType = DE_ON_OUTPUT_SCAN_ERROR;
            eventValue = ui->chkOutScanError->isChecked();
            needsUpdate = ((eventValue) | (mEvtOutScanErrorExists));
            if (needsUpdate)
                setEventsByHandle(mDaqDeviceHandle, eventType, eventValue);
        }
        if (mEvtOnDataChanged | mEvtParamChanged) {
            eventType = DE_ON_DATA_AVAILABLE;
            eventValue = ui->chkDataAvailable->isChecked();
            needsUpdate = ((eventValue) | (mEvtOnDataExists));
            if (needsUpdate) {
                setEventsByHandle(mDaqDeviceHandle, eventType, eventValue);
                unsigned long long eventParameter = ui->leEventParams->text().toULongLong();
                setEventParameterByHandle(mDaqDeviceHandle, eventParameter);
            }
        }
    }
}

void EventsDialog::updateProperties()
{
    mCheckStatusEnabled = ui->chkGetStatus->isChecked();
    mWaitForDone = ui->chkUseWait->isChecked();
    mWaitTime = ui->leWaitTimo->text().toDouble();
}
