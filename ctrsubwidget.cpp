#include "ctrsubwidget.h"
#include "ui_ctrsubwidget.h"
#include <qfont.h>
#include "childwindow.h"
#include "unitest.h"
#include "uldaq.h"
#include "../Test/errordialog.h"


CtrSubWidget::CtrSubWidget(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::CtrSubWidget)
{
    ui->setupUi(this);

    tmrCheckStatus = new QTimer(this);
    mUseGetStatus = true;
    mUseWait = false;
    ui->cmbSelReg->addItem("Load Resister", CRT_LOAD);
    ui->cmbSelReg->addItem("Min Register", CRT_MIN_LIMIT);
    ui->cmbSelReg->addItem("Max Register", CRT_MAX_LIMIT);
    ui->cmbSelReg->addItem("Out0 Register", CRT_OUTPUT_VAL0);
    ui->cmbSelReg->addItem("Out1 Register", CRT_OUTPUT_VAL1);

    ui->cmbReadReg->addItem("CIn Register", 0);
    ui->cmbReadReg->addItem("Count Register", CRT_COUNT);
    ui->cmbReadReg->addItem("Load Register", CRT_LOAD);
    ui->cmbReadReg->addItem("Min Register", CRT_MIN_LIMIT);
    ui->cmbReadReg->addItem("Max Register", CRT_MAX_LIMIT);
    ui->cmbReadReg->addItem("Out0 Register", CRT_OUTPUT_VAL0);
    ui->cmbReadReg->addItem("Out1 Register", CRT_OUTPUT_VAL1);

    ui->cmbOptions->addItem("Default", PO_DEFAULT);
    ui->cmbOptions->addItem("Trigger", PO_EXTTRIGGER);
    ui->cmbOptions->addItem("Retrigger", PO_RETRIGGER);
    ui->cmbIdleState->addItem("Idle Low", TMRIS_LOW);
    ui->cmbIdleState->addItem("Idle High", TMRIS_HIGH);

    ui->lblDelayReturn->setStyleSheet("QLabel { background-color : white; color : blue; } ");
    ui->lblDutyReturn->setStyleSheet("QLabel { background-color : white; color : blue; }" );
    ui->lblFreqReturn->setStyleSheet("QLabel { background-color : white; color : blue; }" );
    ui->lblRateReturned->setStyleSheet("QLabel { background-color : white; color : blue; }" );
    ui->lblStatus->setStyleSheet("QLabel { color : blue; }" );
    ui->lblInfo->setStyleSheet("QLabel { color : blue; }" );
    ui->teShowValues->setStyleSheet("QTextEdit { background-color : white; color : blue; }" );

    ui->lblDelayReturn->setFont(QFont ("Courier", 8));
    ui->lblDutyReturn->setFont(QFont ("Courier", 8));
    ui->lblFreqReturn->setFont(QFont ("Courier", 8));
    ui->lblRateReturned->setFont(QFont ("Courier", 8));
    ui->teShowValues->setFont(QFont ("Courier", 8));
    ui->lblStatus->setFont(QFont ("Courier", 8));

    connect(tmrCheckStatus, SIGNAL(timeout()), this, SLOT(checkStatus()));
    connect(ui->cmdGo, SIGNAL(clicked(bool)), this, SLOT(onClickCmdGo()));
    connect(ui->cmdStop, SIGNAL(clicked(bool)), this, SLOT(stopGoTimer()));
    connect(ui->cmdLoad, SIGNAL(clicked(bool)), this, SLOT(onClickCmdGo()));
    connect(ui->lwMeasMode, SIGNAL(itemSelectionChanged()), this, SLOT(parseMeasMode()));
    connect(ui->lwTypeModes, SIGNAL(itemSelectionChanged()), this, SLOT(parseMeasMode()));
    connect(ui->cmdApplyCCfg, SIGNAL(clicked(bool)), this, SLOT(runCConfigScan()));
    connect(ui->plotCtr->xAxis, SIGNAL(rangeChanged(QCPRange)),
            ui->plotCtr->xAxis2, SLOT(setRange(QCPRange)));
    connect(ui->plotCtr->yAxis, SIGNAL(rangeChanged(QCPRange)),
            ui->plotCtr->yAxis2, SLOT(setRange(QCPRange)));
    connect(ui->rbAutoScale, SIGNAL(clicked(bool)), this, SLOT(replot()));
    connect(ui->rbFullScale, SIGNAL(clicked(bool)), this, SLOT(replot()));
    connect(this, SIGNAL(dataReady(unsigned long long, unsigned long long, int)), this,
            SLOT(showData(unsigned long long, unsigned long long, int)));

    rbPlotSel[0] = ui->rbPlot0;
    rbPlotSel[1] = ui->rbPlot1;
    rbPlotSel[2] = ui->rbPlot2;
    rbPlotSel[3] = ui->rbPlot3;
    rbPlotSel[4] = ui->rbPlot4;
    rbPlotSel[5] = ui->rbPlot5;
    rbPlotSel[6] = ui->rbPlot6;
    rbPlotSel[7] = ui->rbPlot7;
    for (int i = 0; i<8; i++) {
        connect(rbPlotSel[i], SIGNAL(clicked(bool)), this, SLOT(plotSelect()));
        rbPlotSel[i]->setVisible(false);
    }

    ui->lwMeasMode->addItem("Default");
    ui->lwMeasMode->addItem("ClearOnRead");
    ui->lwMeasMode->addItem("CountDown");
    ui->lwMeasMode->addItem("GateControlsDir");
    ui->lwMeasMode->addItem("GateClearsCtr");
    ui->lwMeasMode->addItem("GateTriggersCtr");
    ui->lwMeasMode->addItem("CtrOutputOn");
    ui->lwMeasMode->addItem("InitStateHigh");
    ui->lwMeasMode->addItem("NoRecycle");
    ui->lwMeasMode->addItem("RangeLimitOn");
    ui->lwMeasMode->addItem("GateOn");
    ui->lwMeasMode->addItem("InvertGate");

    ui->lwTypeModes->addItem("Default");
    ui->lwTypeModes->addItem("PeriodX10");
    ui->lwTypeModes->addItem("PeriodX100");
    ui->lwTypeModes->addItem("PeriodX1000");
    ui->lwTypeModes->addItem("PeriodGateOn");
    ui->lwTypeModes->addItem("PerInvertGate");
    ui->lwTypeModes->addItem("PulseGateOn");
    ui->lwTypeModes->addItem("PlsInvertGate");
    ui->lwTypeModes->addItem("TmgInvertGate");
    ui->lwTypeModes->addItem("EncoderX2");
    ui->lwTypeModes->addItem("EncoderX4");
    ui->lwTypeModes->addItem("EncLatchOnZ");
    ui->lwTypeModes->addItem("EncClearOnZ");
    ui->lwTypeModes->addItem("EncNoRecycle");
    ui->lwTypeModes->addItem("EncRangeLimOn");
    ui->lwTypeModes->addItem("EncZActiveEdge");

    ui->cmbDebounceMode->addItem("Diasble", CDM_NONE);
    ui->cmbDebounceMode->addItem("Trig After Stable", CDM_TRIGGER_AFTER_STABLE);
    ui->cmbDebounceMode->addItem("Trig Before Stable", CDM_TRIGGER_BEFORE_STABLE);

    ui->cmbDebounce->addItem("Debounce None", CDT_DEBOUNCE_0ns);
    ui->cmbDebounce->addItem("Debounce 500ns", CDT_DEBOUNCE_500ns);
    ui->cmbDebounce->addItem("Debounce 1.5us", CDT_DEBOUNCE_1500ns);
    ui->cmbDebounce->addItem("Debounce 3.5us", CDT_DEBOUNCE_3500ns);
    ui->cmbDebounce->addItem("Debounce 7.5us", CDT_DEBOUNCE_7500ns);
    ui->cmbDebounce->addItem("Debounce 15.5us", CDT_DEBOUNCE_15500ns);
    ui->cmbDebounce->addItem("Debounce 31.5us", CDT_DEBOUNCE_31500ns);
    ui->cmbDebounce->addItem("Debounce 63.5us", CDT_DEBOUNCE_63500ns);
    ui->cmbDebounce->addItem("Debounce 1.275ms", CDT_DEBOUNCE_127500ns);
    ui->cmbDebounce->addItem("Debounce 100us", CDT_DEBOUNCE_100us);
    ui->cmbDebounce->addItem("Debounce 300us", CDT_DEBOUNCE_300us);
    ui->cmbDebounce->addItem("Debounce 700us", CDT_DEBOUNCE_700us);
    ui->cmbDebounce->addItem("Debounce 1.5ms", CDT_DEBOUNCE_1500us);
    ui->cmbDebounce->addItem("Debounce 3.1ms", CDT_DEBOUNCE_3100us);
    ui->cmbDebounce->addItem("Debounce 6.3ms", CDT_DEBOUNCE_6300us);
    ui->cmbDebounce->addItem("Debounce 12.7ms", CDT_DEBOUNCE_12700us);
    ui->cmbDebounce->addItem("Debounce 25.5ms", CDT_DEBOUNCE_25500us);

    ui->cmbTickSize->addItem("Tick Size 20ns", CTS_TICK_20ns);
    ui->cmbTickSize->addItem("Tick Size 20.83ns", CTS_TICK_20PT83ns);
    ui->cmbTickSize->addItem("Tick Size 200ns", CTS_TICK_200ns);
    ui->cmbTickSize->addItem("Tick Size 208.3ns", CTS_TICK_208PT3ns);
    ui->cmbTickSize->addItem("Tick Size 2us", CTS_TICK_2000ns);
    ui->cmbTickSize->addItem("Tick Size 2.083us", CTS_TICK_2083PT3ns);
    ui->cmbTickSize->addItem("Tick Size 20us", CTS_TICK_20000ns);
    ui->cmbTickSize->addItem("Tick Size 20.83us", CTS_TICK_20833PT3ns);

    ui->cmbMeasType->addItem("Count", CMT_COUNT);
    ui->cmbMeasType->addItem("Period", CMT_PERIOD);
    ui->cmbMeasType->addItem("Pulse Width", CMT_PULSE_WIDTH);
    ui->cmbMeasType->addItem("Timing", CMT_TIMING);
    ui->cmbMeasType->addItem("Encoder", CMT_ENCODER);

    ui->cmdStop->setVisible(false);
    buffer = NULL;
    mPlot = false;
    mPlotChan = -1;
    mEventType = DE_NONE;
    mMeasMode = CMM_DEFAULT;
    mGoTmrIsRunning = false;
    ui->cmdStop->setEnabled(false);
    setupPlot(ui->plotCtr, 1);
    ui->plotCtr->replot();
    mMainWindow = getMainWindow();
}

CtrSubWidget::~CtrSubWidget()
{
    delete ui;
}

MainWindow *CtrSubWidget::getMainWindow()
{
    foreach (QWidget *w, QApplication::topLevelWidgets())
        if (QMainWindow* mainWin = qobject_cast<QMainWindow*>(w))
            return qobject_cast<MainWindow *>(mainWin);
    return nullptr;
}

void CtrSubWidget::initDeviceParams()
{
    QString nameOfFunc, funcArgs, argVals, funcStr;
    CtrInfoItem infoItem;
    long long infoValue;
    QTime t;
    QString sStartTime;

    nameOfFunc = "ulCtrGetInfo";
    funcArgs = "(mDaqDeviceHandle, infoItem, 0, &infoValue)\n";

    infoItem = CTR_INFO_RESOLUTION;
    sStartTime = t.currentTime().toString("hh:mm:ss.zzz") + "~";
    err = ulCtrGetInfo(mDaqDeviceHandle, infoItem, 0, &infoValue);
    argVals = QStringLiteral("(%1, %2, %3, %4)")
            .arg(mDaqDeviceHandle)
            .arg(infoItem)
            .arg(0)
            .arg(infoValue);

    funcStr = nameOfFunc + funcArgs + "Arg vals: " + argVals;
    if (err == ERR_NO_ERROR) {
        mMainWindow->addFunction(sStartTime + funcStr);
        mCtrResolution = infoValue;
    } else {
        mMainWindow->setError(err, sStartTime + funcStr);
    }
    ui->cmdGo->setFocus();
}

void CtrSubWidget::updateParameters()
{
    bool showStop;
    ChildWindow *parentWindow;

    parentWindow = qobject_cast<ChildWindow *>(this->parent());
    mDaqDeviceHandle = parentWindow->devHandle();
    mDevName = parentWindow->devName();
    mDevUID = parentWindow->devUID();

    mUseGetStatus = parentWindow->statusEnabled();
    mUseWait = parentWindow->waitEnabled();
    mWaitTime = parentWindow->waitTime();

    mCInFlags = parentWindow->ciFlags();
    mTriggerType = parentWindow->triggerType();
    mTrigChannel = parentWindow->trigChannel();
    mTrigLevel = parentWindow->trigLevel();
    mTrigVariance = parentWindow->trigVariance();
    mRetrigCount = parentWindow->retrigCount();
    mScanOptions = parentWindow->scanOptions();
    QString optNames = getOptionNames(mScanOptions);
    if ((mUtFunction == UL_C_INSCAN) |
            (mUtFunction == UL_TMR_OUT)) {
        if (mUtFunction == UL_C_INSCAN)
            ui->lblInfo->setText("UlCinScan(" + optNames + ")");
        showStop = true;
    }
    mUseTimer = parentWindow->tmrEnabled();
    mStopOnStart = parentWindow->tmrStopOnStart();
    mGoTmrIsRunning = parentWindow->tmrRunning();
    if (mUseTimer | showStop) {
        ui->cmdStop->setVisible(true);
        ui->cmdStop->setEnabled(mGoTmrIsRunning);
    }
    ui->leBlockSize->setEnabled(mUseGetStatus);
    this->setWindowTitle(mFuncName + ": " + mDevName + QString(" [%1]").arg(mDaqDeviceHandle));
}

void CtrSubWidget::groupChanged(int newGroup)
{
    mCurGroup = newGroup;
    //setUiForGroup();
}

void CtrSubWidget::setUiForGroup()
{
    return;
}

void CtrSubWidget::functionChanged(int utFunction)
{
    mUtFunction = utFunction;
    this->setUiForFunction();
}

void CtrSubWidget::setupPlot(QCustomPlot *dataPlot, int chanCount)
{
    QColor penColor;
    QPalette brushColor;

    //brushColor = QPalette::background();
    int chanCycle;
    int curChanCount;
    dataPlot->clearGraphs();
    dataPlot->setBackground(brushColor.background());
    dataPlot->axisRect()->setBackground(Qt::white);
    chanCycle = -1;

    if(mPlotChan == -1)
        curChanCount = chanCount;
    else
        curChanCount = 1;

    for(int chan=0; chan<curChanCount; chan++)
    {
        if(mPlotChan == -1)
            chanCycle += 1;
        else
            chanCycle = mPlotChan;
        if(chanCycle>7)
            chanCycle = chanCycle % 8;
        switch(chanCycle)
        {
        case 0:
            penColor = Qt::blue;
            break;
        case 1:
            penColor = Qt::red;
            break;
        case 2:
            penColor = Qt::green;
            break;
        case 3:
            penColor = Qt::cyan;
            break;
        case 4:
            penColor = Qt::darkCyan;
            break;
        case 5:
            penColor = Qt::magenta;
            break;
        case 6:
            penColor = Qt::gray;
            break;
        default:
            penColor = Qt::black;
            break;
        }
        dataPlot->addGraph();
        dataPlot->addGraph();
        dataPlot->graph(chan)->setPen(penColor);
    }
    dataPlot->xAxis2->setVisible(true);
    dataPlot->xAxis2->setTickLabels(false);
    dataPlot->axisRect(0)->setAutoMargins(QCP::msLeft|QCP::msBottom);
    dataPlot->axisRect(0)->setMargins(QMargins(0,2,2,0));
    dataPlot->yAxis2->setVisible(true);
    dataPlot->yAxis2->setTickLabels(false);
    dataPlot->xAxis->setTickLabelFont(QFont(QFont().family(), 8));
    dataPlot->yAxis->setTickLabelFont(QFont(QFont().family(), 8));
    dataPlot->xAxis->setTickLabelColor(Qt::blue);
    dataPlot->yAxis->setTickLabelColor(Qt::blue);
    dataPlot->xAxis->setAutoTickCount(3);
}

void CtrSubWidget::showPlotWindow(bool showIt)
{
    QFrame::Shape frameShape;

    mPlot = showIt;
    int curIndex = 0;
    //frameShape = QFrame::Box;
    frameShape = QFrame::NoFrame;

    if (showIt) {
        curIndex = 2;
    }
    ui->stackedWidget->setFrameShape(frameShape);
    ui->stackedWidget->setCurrentIndex(curIndex);
}

void CtrSubWidget::setUiForFunction()
{
    bool textVisible, scanVisible, ctrSelectVisible;
    bool scanParamsVisible, setNumberVisible, stopVisible;
    bool regSelectorVisible;
    ChildWindow *parentWindow;

    parentWindow = qobject_cast<ChildWindow *>(this->parent());
    ctrCheckBoxes.clear();
    ctrCheckBoxes = ui->fraCtrSelect->findChildren<QCheckBox*>();

    scanParamsVisible = false;
    setNumberVisible = false;
    textVisible = false;
    ctrSelectVisible = true;
    regSelectorVisible = false;
    scanVisible = false;
    stopVisible = false;
    int stackLevel = 0;
    mPlot = false;

    switch (mUtFunction) {
    case UL_C_SELECT:
        mFuncName = "Select Ctr";
        break;
    case UL_C_IN:
        mFuncName = "ulCIn";
        textVisible = true;
        setNumberVisible = true;
        regSelectorVisible = true;
        scanVisible = true;
        break;
    case UL_C_CLEAR:
        mFuncName = "ulCClear";
        textVisible = true;
        scanVisible = true;
        break;
    case UL_C_LOAD:
        mFuncName = "ulCLoad";
        stackLevel = 1;
        break;
    case UL_C_INSCAN:
        mFuncName = "ulCInScan";
        textVisible = true;
        mPlot = true;
        ctrSelectVisible = false;
        scanParamsVisible = true;
        setNumberVisible = true;
        scanVisible = true;
        stopVisible = true;
        break;
    case UL_TMR_OUT:
        mFuncName = "ulTmrPulseOut";
        scanVisible = true;
        stopVisible = true;
        stackLevel = 3;
        break;
    case UL_C_CONFIG_SCAN:
        mFuncName = "ulCConfigScan";
        stackLevel = 4;
        break;
    default:
        break;
    }
    ui->fraScanParams->setVisible(scanParamsVisible);
    ui->fraText->setVisible(textVisible);
    ui->cmbReadReg->setVisible(regSelectorVisible);
    ui->fraCtrSelect->setVisible(ctrSelectVisible);
    ui->fraScan->setVisible(scanVisible);
    ui->spnLowChan->setVisible(scanParamsVisible);
    ui->spnHighChan->setVisible(scanParamsVisible);
    ui->leNumSamples->setVisible(setNumberVisible);
    ui->cmdStop->setVisible(stopVisible);
    ui->cmdStop->setEnabled(false);
    ui->lblInfo->setText(mFuncName);
    showPlotWindow(mPlot);
    ui->stackedWidget->setCurrentIndex(stackLevel);
    this->setWindowTitle(mFuncName + ": " + mDevName + QString(" [%1]").arg(mDaqDeviceHandle));
    ui->cmdGo->setFocus();
    parentWindow->adjustSize();
}

void CtrSubWidget::parseMeasMode()
{
    //get measurement mode for CConfigScan from list widgets
    CounterMeasurementMode selMode;
    long long tempMode, tempAggMode;
    mMeasMode = CMM_DEFAULT;
    tempMode = 0;
    tempAggMode = 0;

    int listModeItems = ui->lwMeasMode->count();
    if (listModeItems)
        if (ui->lwMeasMode->item(0)->isSelected())
            ui->lwMeasMode->clearSelection();
    for (int i = 0; i < listModeItems; i++) {
        if (ui->lwMeasMode->item(i)->isSelected()) {
            selMode = getMeasModeFromListIndex(i);
            tempMode = static_cast<long long>(selMode);
            tempAggMode |= tempMode;
        }
    }

    int listTypeItems = ui->lwTypeModes->count();
    if (listTypeItems)
        if (ui->lwTypeModes->item(0)->isSelected())
                ui->lwTypeModes->clearSelection();
    for (int i = 0; i < listTypeItems; i++) {
        if (ui->lwTypeModes->item(i)->isSelected()) {
            selMode = getMeasModeFromListIndex(i + listModeItems);
            tempMode = static_cast<long long>(selMode);
            tempAggMode |= tempMode;
        }
    }
    mMeasMode = static_cast<CounterMeasurementMode>(tempAggMode);
}

void CtrSubWidget::onClickCmdGo()
{
    ChildWindow *parentWindow;
    parentWindow = qobject_cast<ChildWindow *>(this->parent());
    bool tmrIsEnabled;

    mPlotIndex = 0;
    mPlotCount = 0;
    tmrIsEnabled = parentWindow->tmrEnabled();
    mUseTimer = tmrIsEnabled;
    ui->teShowValues->clear();
    ui->lblRateReturned->clear();
    ui->lblStatus->clear();
    ui->lblInfo->clear();
    runSelectedFunc();
}

void CtrSubWidget::runSelectedFunc()
{
    ChildWindow *parentWindow;
    QFont goFont = ui->cmdGo->font();
    bool tmrIsEnabled, tmrIsRunning;
    bool makeBold, showStop;

    parentWindow = qobject_cast<ChildWindow *>(this->parent());

    switch (mUtFunction) {
    case UL_C_CLEAR:
        runClearFunc();
        break;
    case UL_C_IN:
        runCInFunc();
        break;
    case UL_C_LOAD:
        runCLoadFunc();
        break;
    case UL_TMR_OUT:
        if (!mTriggerType == TRIG_NONE)
            runSetTriggerFunc();
        runTimerOut();
        break;
    case UL_C_INSCAN:
        if (!mTriggerType == TRIG_NONE)
            runSetTriggerFunc();
        runCInScan();
        break;
    default:
        break;
    }

    tmrIsEnabled = parentWindow->tmrEnabled();
    tmrIsRunning = parentWindow->tmrRunning();

    showStop = ((mUtFunction == UL_C_INSCAN)
                | (mUtFunction == UL_TMR_OUT));
    if (!tmrIsEnabled) {
        if (tmrIsRunning)
            parentWindow->setTmrRunning(false);
        ui->cmdStop->setVisible(false);
        mUseTimer = false;
        goFont.setBold(false);
        ui->cmdGo->setFont(goFont);
    } else {
        if (mUseTimer) {
            if (!tmrIsRunning) {
                parentWindow->setTmrRunning(true);
                ui->cmdStop->setEnabled(true);
            }
            showStop = true;
            makeBold = !ui->cmdGo->font().bold();
            goFont.setBold(makeBold);
            ui->cmdGo->setFont(goFont);
        } else {
            if (tmrIsRunning) {
                parentWindow->setTmrRunning(false);
                ui->cmdStop->setEnabled(false);
            }
            goFont.setBold(false);
            ui->cmdGo->setFont(goFont);
        }
    }
    ui->cmdStop->setVisible(showStop);
}

void CtrSubWidget::runSetTriggerFunc()
{
    QString nameOfFunc, funcArgs, argVals, funcStr;
    QTime t;
    QString sStartTime;

    funcArgs = "(mDaqDeviceHandle, trigType, trigChan, level, variance, trigCount)\n";

    if (mUtFunction == UL_TMR_OUT) {
        nameOfFunc = "ulTmrSetTrigger";
        sStartTime = t.currentTime().toString("hh:mm:ss.zzz") + "~";
        err = ulTmrSetTrigger(mDaqDeviceHandle, mTriggerType, mTrigChannel, mTrigLevel, mTrigVariance, mRetrigCount);
    } else {
        nameOfFunc = "ulCInSetTrigger";
        sStartTime = t.currentTime().toString("hh:mm:ss.zzz") + "~";
        err = ulCInSetTrigger(mDaqDeviceHandle, mTriggerType, mTrigChannel, mTrigLevel, mTrigVariance, mRetrigCount);
    }
    argVals = QStringLiteral("(%1, %2, %3, %4, %5, %6)")
            .arg(mDaqDeviceHandle)
            .arg(mTriggerType)
            .arg(mTrigChannel)
            .arg(mTrigLevel)
            .arg(mTrigVariance)
            .arg(mRetrigCount);
    ui->lblInfo->setText(nameOfFunc + argVals + QString(" [Error = %1]").arg(err));

    funcStr = nameOfFunc + funcArgs + "Arg vals: " + argVals;
    if (!err==ERR_NO_ERROR) {
        mMainWindow->setError(err, sStartTime + funcStr);
    } else {
        mMainWindow->addFunction(sStartTime + funcStr);
    }
}

void CtrSubWidget::runEventSetup()
{
    bool disableAll = true;
    bool eventValue = false;
    bool reEnable = true;
    unsigned long long eventParam = 0;
    DaqEventType eventsEnabled = DE_NONE;
    DaqEventType eventsDisabled = DE_NONE;
    int enableList = 0;
    int disableList = 0;

    if (getEndInScanEvent(mDaqDeviceHandle, eventValue)) {
        //if this exists, enable or disable according to eventValue
        if (eventValue) {
            disableAll = false;
            if (!(mEventType & DE_ON_END_OF_INPUT_SCAN))
                enableList = DE_ON_END_OF_INPUT_SCAN;
        } else {
            disableList = DE_ON_END_OF_INPUT_SCAN;
        }
    }
    if (getDataAvailableEvent(mDaqDeviceHandle, eventValue)) {
        //if this exists, enable or disable according to eventValue
        if (eventValue) {
            disableAll = false;
            if (!(mEventType & DE_ON_DATA_AVAILABLE)) {
                reEnable = false;
                enableList |= DE_ON_DATA_AVAILABLE;
            }
            getEventParameter(mDaqDeviceHandle, eventParam);
            if (!(mEventParams == eventParam)) {
                if (reEnable) {
                    mEventParams = eventParam;
                    runEventDisable(DE_ON_DATA_AVAILABLE);
                    enableList |= DE_ON_DATA_AVAILABLE;
                    setEventsByHandle(mDaqDeviceHandle, DE_ON_DATA_AVAILABLE, true);
                    ui->leBlockSize->setText(QString("%1").arg(mEventParams));
                }
            }
        } else {
            disableList |= DE_ON_DATA_AVAILABLE;
        }
    }
    if (getInScanErrorEvent(mDaqDeviceHandle, eventValue)) {
        //if this exists, enable or disable according to eventValue
        if (eventValue) {
            disableAll = false;
            if (!(mEventType & DE_ON_INPUT_SCAN_ERROR))
                enableList |= DE_ON_INPUT_SCAN_ERROR;
        } else {
            disableList |= DE_ON_INPUT_SCAN_ERROR;
        }
    }
    if (disableAll)
        disableList = (int)ALL_EVENT_TYPES;

    eventsEnabled = (DaqEventType)enableList;
    eventsDisabled = (DaqEventType)disableList;

    if (!(eventsEnabled == DE_NONE))
        runEventEnable(eventsEnabled, eventParam);
    if (!(eventsDisabled == DE_NONE))
        runEventDisable(eventsDisabled);
}

void CtrSubWidget::runEventEnable(DaqEventType eventType, unsigned long long eventParam)
{
    QString nameOfFunc, funcArgs, argVals, funcStr;
    QTime t;
    QString sStartTime;

    DaqEventCallback eventCallbackFunction = &CtrSubWidget::eventCallback;

    nameOfFunc = "ulEnableEvent";
    funcArgs = "(mDaqDeviceHandle, eventTypes, eventParameter, *eventCallbackFunction, this)\n";
    sStartTime = t.currentTime().toString("hh:mm:ss.zzz") + "~";
    err = ulEnableEvent(mDaqDeviceHandle, eventType, eventParam, *eventCallbackFunction, this);
    argVals = QStringLiteral("(%1, %2, %3, %4, %5)")
            .arg(mDaqDeviceHandle)
            .arg(eventType)
            .arg(eventParam)
            .arg("eventCallback")
            .arg(this->windowTitle());
    ui->lblInfo->setText(nameOfFunc + argVals + QString(" [Error = %1]").arg(err));

    funcStr = nameOfFunc + funcArgs + "Arg vals: " + argVals;
    int newEventType = 0;
    if (!err==ERR_NO_ERROR) {
        mMainWindow->setError(err, sStartTime + funcStr);
    } else {
        mMainWindow->addFunction(sStartTime + funcStr);
        newEventType = mEventType | eventType;
        mEventType = DaqEventType(newEventType);
        mEventParams = eventParam;
    }
}

void CtrSubWidget::runEventDisable(DaqEventType eventType)
{
    QString nameOfFunc, funcArgs, argVals, funcStr;
    QTime t;
    QString sStartTime;

    nameOfFunc = "ulDisableEvent";
    funcArgs = "(mDaqDeviceHandle, eventTypes)\n";
    sStartTime = t.currentTime().toString("hh:mm:ss.zzz") + "~";
    err = ulDisableEvent(mDaqDeviceHandle, eventType);
    argVals = QStringLiteral("(%1, %2)")
            .arg(mDaqDeviceHandle)
            .arg(eventType);
    ui->lblInfo->setText(nameOfFunc + argVals + QString(" [Error = %1]").arg(err));

    funcStr = nameOfFunc + funcArgs + "Arg vals: " + argVals;
    if (!err==ERR_NO_ERROR) {
        mMainWindow->setError(err, sStartTime + funcStr);
    } else {
        mMainWindow->addFunction(sStartTime + funcStr);
        int mask = eventType ^ (int)ALL_EVENT_TYPES;
        DaqEventType newEvent = (DaqEventType)(mask & mEventType);
        mEventType = newEvent;
        removeEventsByHandle(mDaqDeviceHandle, eventType);
    }
}

void CtrSubWidget::eventCallback(DaqDeviceHandle devHandle, DaqEventType eventType, unsigned long long eventData, void* userData)
{
    if (devHandle == 0)
        eventType = DE_ON_INPUT_SCAN_ERROR;
    CtrSubWidget *ctrChild = (CtrSubWidget *)userData;
    ctrChild->callbackHandler(eventType, eventData);
}

void CtrSubWidget::callbackHandler(DaqEventType eventType, unsigned long long eventData)
{
    //called from eventCallback in testUtilities
    int finalBlockSize;

    switch (eventType) {
    case DE_ON_DATA_AVAILABLE:
        ui->lblStatus->setText(QString("Data Available: %1, TotalCount: %2, Index: %3")
                               .arg(eventData)
                               .arg(mPlotCount)
                               .arg(mPlotIndex));
        emit dataReady(mPlotCount, mPlotIndex, mEventParams);
        break;
    case DE_ON_INPUT_SCAN_ERROR:
        ui->cmdStop->setEnabled(false);
        ui->lblStatus->setText(QString("Error event: %1").arg(eventData));
        break;
    case DE_ON_END_OF_INPUT_SCAN:
        ui->cmdStop->setEnabled(false);
        ui->lblStatus->setText(QString("Scan end event: %1").arg(eventData));
        finalBlockSize = eventData - (mPlotCount / mChanCount);
        if (finalBlockSize > 1)
            emit dataReady(mPlotCount, mPlotIndex, finalBlockSize);
        break;
    default:
        break;
    }
    mPlotCount = eventData * mChanCount;
    mPlotIndex = mPlotCount % (mSamplesPerChan * mChanCount);
}

void CtrSubWidget::showData(unsigned long long curTotalCount, unsigned long long curIndex, int blockSize)
{
    if (mPlot) {
        plotScan(curTotalCount, curIndex, blockSize);
    } else {
        printData(curTotalCount, curIndex, blockSize);
    }
}

void CtrSubWidget::runCLoadFunc()
{
    QString nameOfFunc, funcArgs, argVals, funcStr;
    unsigned long long loadValue;
    int ctrNum;
    CounterRegisterType regType;
    QList<int> ctrsSelected;
    QTime t;
    QString sStartTime;

    loadValue = ui->leLoadReg->text().toULongLong();
    regType = static_cast<CounterRegisterType>(ui->cmbSelReg->currentData(Qt::UserRole).toInt());
    ctrsSelected.clear();
    foreach (QCheckBox *chkCtrSel, ctrCheckBoxes) {
        if (chkCtrSel->isChecked()) {
            ctrNum = chkCtrSel->property("counterNum").toInt();
            ctrsSelected.append(ctrNum);
        }
    }

    nameOfFunc = "ulCLoad";
    funcArgs = "(mDaqDeviceHandle, ctrNum, regType, loadValue)\n";
    foreach (int ctrNum, ctrsSelected) {
        sStartTime = t.currentTime().toString("hh:mm:ss.zzz") + "~";
        err = ulCLoad(mDaqDeviceHandle, ctrNum, regType, loadValue);
        argVals = QStringLiteral("(%1, %2, %3, %4)")
                .arg(mDaqDeviceHandle)
                .arg(ctrNum)
                .arg(regType)
                .arg(loadValue);
        ui->lblInfo->setText(nameOfFunc + argVals + QString(" [Error = %1]").arg(err));

        funcStr = nameOfFunc + funcArgs + "Arg vals: " + argVals;
        if(!err==ERR_NO_ERROR) {
            mMainWindow->setError(err, sStartTime + funcStr);
            return;
        } else {
            mMainWindow->addFunction(sStartTime + funcStr);
        }
    }
}

void CtrSubWidget::runCInFunc()
{
    QString dataText, str;
    int ctrNum, numCtrChans, numSamples;
    int curIndex = 0;
    QString nameOfFunc, funcArgs, argVals, funcStr;
    unsigned long long data;
    QList<int> ctrsSelected;
    CounterRegisterType regType;
    QString regName;
    QTime t;
    QString sStartTime;

    ctrsSelected.clear();
    foreach (QCheckBox *chkCtrSel, ctrCheckBoxes) {
        if (chkCtrSel->isChecked()) {
            ctrNum = chkCtrSel->property("counterNum").toInt();
            ctrsSelected.append(ctrNum);
        }
    }
    numCtrChans = ctrsSelected.count();
    numSamples = ui->leNumSamples->text().toInt();
    QVector<unsigned long long> dataVal(numCtrChans);
    QVector<QVector<unsigned long long>>  dataArray(numSamples);
    for (int sample=0; sample<numSamples; sample++)
        dataArray[sample].resize(numCtrChans);
    regType = (CounterRegisterType)ui->cmbReadReg->currentData().toInt();

    for (int sampleNum = 0; sampleNum < numSamples; sampleNum++) {
        curIndex = 0;
        foreach (int ctrNum, ctrsSelected) {
            if (regType == 0) {
                nameOfFunc = "ulCIn";
                funcArgs = "(mDaqDeviceHandle, ctrNum, data)\n";
                sStartTime = t.currentTime().toString("hh:mm:ss.zzz") + "~";
                err = ulCIn(mDaqDeviceHandle, ctrNum, &data);
            } else {
                nameOfFunc = "ulCRead";
                funcArgs = "(mDaqDeviceHandle, ctrNum, regType, data)\n";
                sStartTime = t.currentTime().toString("hh:mm:ss.zzz") + "~";
                err = ulCRead(mDaqDeviceHandle, ctrNum, regType, &data);
                regName = QString("%1, ").arg(regType);
            }
            dataVal[curIndex] = data;
            dataArray[sampleNum][curIndex] = dataVal[curIndex];
            curIndex++;
            argVals = QStringLiteral("(%1, %2, %3%4)") //intentional missing comma
                    .arg(mDaqDeviceHandle)
                    .arg(ctrNum)
                    .arg(regName)
                    .arg(data);
            ui->lblInfo->setText(nameOfFunc + argVals + QString(" [Error = %1]").arg(err));

            funcStr = nameOfFunc + funcArgs + "Arg vals: " + argVals;
            if(!err==ERR_NO_ERROR) {
                mMainWindow->setError(err, sStartTime + funcStr);
                return;
            } else {
                mMainWindow->addFunction(sStartTime + funcStr);
            }
        }
    }

    QString temp;
    ui->teShowValues->clear();
    dataText = "<style> th, td { padding-right: 10px;}</style><tr>";
    for (int sampleNum = 0; sampleNum < numSamples; sampleNum++) {
        curIndex = 0;
        dataText.append("<td>" + str.setNum(sampleNum * numCtrChans) + "</td>");
        foreach (int ctrNum, ctrsSelected) {
            data = dataArray[sampleNum][curIndex];
            dataText.append("<td>C" + str.setNum(ctrNum) + ": " + temp.setNum(data) + "</td>");
            curIndex++;
        }
        dataText.append("</tr><tr>");
    }
    dataText.append("</td></tr>");
    ui->teShowValues->setHtml(dataText);
}

void CtrSubWidget::runClearFunc()
{
    int ctrNum;
    QString nameOfFunc, funcArgs, argVals, funcStr;
    QTime t;
    QString sStartTime;

    nameOfFunc = "ulCClear";
    funcArgs = "(mDaqDeviceHandle, ctrNum)\n";
    ui->teShowValues->clear();
    foreach (QCheckBox *chkCtrSel, ctrCheckBoxes) {
        if (chkCtrSel->isChecked()) {
            ctrNum = chkCtrSel->property("counterNum").toInt();
            sStartTime = t.currentTime().toString("hh:mm:ss.zzz") + "~";
            err = ulCClear(mDaqDeviceHandle, ctrNum);
            argVals = QStringLiteral("(%1, %2)")
                    .arg(mDaqDeviceHandle)
                    .arg(ctrNum);
            ui->lblInfo->setText(nameOfFunc + argVals + QString(" [Error = %1]").arg(err));

            funcStr = nameOfFunc + funcArgs + "Arg vals: " + argVals;
            if(!err==ERR_NO_ERROR) {
                mMainWindow->setError(err, sStartTime + funcStr);
            } else {
                mMainWindow->addFunction(sStartTime + funcStr);
                ui->teShowValues->append(QString("Ctr %1 cleared").arg(ctrNum));
            }
        }
    }
}

void CtrSubWidget::runCConfigScan()
{
    int ctrNum;
    QString nameOfFunc, funcArgs, argVals, funcStr;
    CounterMeasurementType measType;
    CounterEdgeDetection edgeDetection;
    CounterTickSize tickSize;
    CounterDebounceMode debounceMode;
    CounterDebounceTime debounceTime;
    CConfigScanFlag confFlags = CF_DEFAULT;
    QTime t;
    QString sStartTime;

    measType = static_cast<CounterMeasurementType>(ui->cmbMeasType->currentData(Qt::UserRole).toLongLong());
    debounceMode = static_cast<CounterDebounceMode>(ui->cmbDebounceMode->currentData(Qt::UserRole).toLongLong());
    debounceTime = static_cast<CounterDebounceTime>(ui->cmbDebounce->currentData(Qt::UserRole).toLongLong());
    tickSize = static_cast<CounterTickSize>(ui->cmbTickSize->currentData(Qt::UserRole).toLongLong());
    edgeDetection = CED_RISING_EDGE;
    if (ui->rdoEdgeNeg->isChecked())
        edgeDetection = CED_FALLING_EDGE;

    nameOfFunc = "ulCConfigScan";
    funcArgs = "(mDaqDeviceHandle, ctrNum, measType, measMode, "
            "edgeDetection, tickSize, debounceMode, debounceTime, confFlags)\n";
    foreach (QCheckBox *chkCtrSel, ctrCheckBoxes) {
        if (chkCtrSel->isChecked()) {
            ctrNum = chkCtrSel->property("counterNum").toInt();
            sStartTime = t.currentTime().toString("hh:mm:ss.zzz") + "~";
            err = ulCConfigScan(mDaqDeviceHandle, ctrNum, measType, mMeasMode,
                                edgeDetection, tickSize, debounceMode, debounceTime, confFlags);
            argVals = QStringLiteral("(%1, %2, %3, %4, %5, %6, %7, %8, %9)")
                    .arg(mDaqDeviceHandle)
                    .arg(ctrNum)
                    .arg(measType)
                    .arg(mMeasMode)
                    .arg(edgeDetection)
                    .arg(tickSize)
                    .arg(debounceMode)
                    .arg(debounceTime)
                    .arg(confFlags);
            ui->lblInfo->setText(nameOfFunc + argVals + QString(" [Error = %1]").arg(err));

            funcStr = nameOfFunc + funcArgs + "Arg vals: " + argVals;
            if(!err==ERR_NO_ERROR) {
                mMainWindow->setError(err, sStartTime + funcStr);
                return;
            } else {
                mMainWindow->addFunction(sStartTime + funcStr);
                //ui->teShowValues->append(QString("Ctr %1 cleared").arg(ctrNum));
            }
        }
    }
}

void CtrSubWidget::runCInScan()
{
    QString nameOfFunc, funcArgs, argVals, funcStr;
    int highCtr, lowCtr;
    mSamplesPerChan = ui->leNumSamples->text().toLong();
    QTime t;
    QString sStartTime;

    lowCtr = ui->spnLowChan->value();
    highCtr = ui->spnHighChan->value();
    mBlockSize = ui->leBlockSize->text().toLongLong();
    mChanCount = (highCtr - lowCtr) + 1;
    if(mChanCount < 1) mChanCount = 1;
    mSamplesPerChan = ui->leNumSamples->text().toLong();
    double rate = ui->leRate->text().toDouble();
    //mCInFlags = CINSCAN_FF_DEFAULT;

    long long bufSize = mChanCount * mSamplesPerChan;
    if(mPlot)
        setupPlot(ui->plotCtr, mChanCount);

    if (buffer) {
        delete[] buffer;
        buffer = NULL;
    }

    mBufSize = bufSize;
    buffer = new unsigned long long [bufSize];
    memset(buffer, 0, mBufSize * sizeof(*buffer));

    nameOfFunc = "ulCInScan";
    funcArgs = "(mDaqDeviceHandle, lowCtr, highCtr, mSamplesPerChan, &rate, options, ctrFlags, &data)\n";
    sStartTime = t.currentTime().toString("hh:mm:ss.zzz") + "~";
    err = ulCInScan(mDaqDeviceHandle, lowCtr, highCtr, mSamplesPerChan, &rate, mScanOptions, mCInFlags, buffer);
    argVals = QStringLiteral("(%1, %2, %3, %4, %5, %6, %7, %8)")
            .arg(mDaqDeviceHandle)
            .arg(lowCtr)
            .arg(highCtr)
            .arg(mSamplesPerChan)
            .arg(rate)
            .arg(mScanOptions)
            .arg(mCInFlags)
            .arg(buffer[0]);
    ui->lblInfo->setText(nameOfFunc + argVals + QString(" [Error = %1]").arg(err));

    funcStr = nameOfFunc + funcArgs + "Arg vals: " + argVals;
    if(!err==ERR_NO_ERROR) {
        mMainWindow->setError(err, sStartTime + funcStr);
    } else {
        mMainWindow->addFunction(sStartTime + funcStr);
        ui->lblRateReturned->setText(QString("%1").arg(rate, 1, 'f', 4, '0'));
        if (mUseWait) {
            qApp->processEvents();
            WaitType waitType = WAIT_UNTIL_DONE;
            long long waitParam = 0;
            nameOfFunc = "ulCInScanWait";
            funcArgs = "(mDaqDeviceHandle, waitType, waitParam, timeout)\n";
            sStartTime = t.currentTime().toString("hh:mm:ss.zzz") + "~";
            err = ulCInScanWait(mDaqDeviceHandle, waitType, waitParam, mWaitTime);
            argVals = QStringLiteral("(%1, %2, %3, %4)")
                    .arg(mDaqDeviceHandle)
                    .arg(waitType)
                    .arg(waitParam)
                    .arg(mWaitTime);

            funcStr = nameOfFunc + funcArgs + "Arg vals: " + argVals;
            if (!err==ERR_NO_ERROR) {
                mMainWindow->setError(err, sStartTime + funcStr);
            } else {
                mMainWindow->addFunction(sStartTime + funcStr);
            }
        }
        if(mUseGetStatus) {
            mStatusTimerEnabled = true;
            tmrCheckStatus->start(300);
        } else {
            ui->cmdStop->setEnabled(true);
        }
    }
}

void CtrSubWidget::userScanStop()
{
    QString nameOfFunc, funcArgs, argVals, funcStr;

    mUseTimer = false;
    if (mUtFunction == UL_C_INSCAN) {
        ScanStatus status;
        struct TransferStatus xferStatus;
        unsigned long long currentScanCount;
        unsigned long long currentTotalCount;
        long long currentIndex;
        QTime t;
        QString sStartTime;

        nameOfFunc = "ulCInScanStatus";
        funcArgs = "(mDaqDeviceHandle, &status, &currentScanCount, &currentTotalCount, &currentIndex)\n";
        mStatusTimerEnabled = false;
        sStartTime = t.currentTime().toString("hh:mm:ss.zzz") + "~";
        err = ulCInScanStatus(mDaqDeviceHandle, &status, &xferStatus);
        currentScanCount = xferStatus.currentScanCount;
        currentTotalCount = xferStatus.currentTotalCount;
        currentIndex = xferStatus.currentIndex;
        argVals = QStringLiteral("(%1, %2, %3, %4, %5)")
                .arg(mDaqDeviceHandle)
                .arg(status)
                .arg(currentScanCount)
                .arg(currentTotalCount)
                .arg(currentIndex);
        mRunning = (status == SS_RUNNING);
        ui->lblStatus->setText(QStringLiteral("Idle at %1, (%2 perChan), %3")
                               .arg(currentTotalCount)
                               .arg(currentScanCount)
                               .arg(currentIndex));

        funcStr = nameOfFunc + funcArgs + "Arg vals: " + argVals;
        if (!(err == ERR_NO_ERROR)) {
            mMainWindow->setError(err, sStartTime + funcStr);
        } else {
            mMainWindow->addFunction(sStartTime + funcStr);
        }

        err = stopScan(currentScanCount, currentTotalCount, currentIndex);
        ui->cmdStop->setEnabled(mGoTmrIsRunning | mRunning);
    }
}

void CtrSubWidget::runTimerOut()
{
    QString nameOfFunc, funcArgs, argVals, funcStr;
    int timerNum;
    double frequency, dutyCycle, initialDelay;
    unsigned long long pulseCount;
    TmrIdleState idleState;
    PulseOutOption options;
    TmrStatus status;
    QTime t;
    QString sStartTime;

    frequency = ui->leFrequency->text().toDouble();
    dutyCycle = ui->leDutyCycle->text().toDouble();
    initialDelay = ui->leInitialDelay->text().toDouble();
    pulseCount = ui->lePulseCount->text().toULongLong();
    idleState = static_cast<TmrIdleState>(ui->cmbIdleState->currentData(Qt::UserRole).toInt());
    options = static_cast<PulseOutOption>(ui->cmbOptions->currentData(Qt::UserRole).toInt());
    nameOfFunc = "ulTmrPulseOutStart";
    funcArgs = "(mDaqDeviceHandle, timerNum, frequency, "
               "dutyCycle, pulseCount,initialDelay, idleState, options)\n";
    foreach (QCheckBox *chkCtrSel, ctrCheckBoxes) {
        if (chkCtrSel->isChecked()) {
            timerNum = chkCtrSel->property("counterNum").toInt();
            sStartTime = t.currentTime().toString("hh:mm:ss.zzz") + "~";
            err = ulTmrPulseOutStart(mDaqDeviceHandle, timerNum, &frequency,
                                     &dutyCycle, pulseCount, &initialDelay, idleState, options);
            ui->lblFreqReturn->setText(QString("%1Hz").arg(frequency));
            ui->lblDutyReturn->setText(QString("%1%").arg(dutyCycle * 100));
            ui->lblDelayReturn->setText(QString("%1s").arg(initialDelay));
            argVals = QStringLiteral("(%1, %2, %3, %4, %5, %6, %7, %8)")
                    .arg(mDaqDeviceHandle)
                    .arg(timerNum)
                    .arg(frequency)
                    .arg(dutyCycle)
                    .arg(pulseCount)
                    .arg(initialDelay)
                    .arg(idleState)
                    .arg(options);
            ui->lblInfo->setText(nameOfFunc + argVals + QString(" [Error = %1]").arg(err));

            funcStr = nameOfFunc + funcArgs + "Arg vals: " + argVals;
            if(!err==ERR_NO_ERROR) {
                mMainWindow->setError(err, sStartTime + funcStr);
            } else {
                mMainWindow->addFunction(sStartTime + funcStr);
                ui->cmdStop->setEnabled(true);
                nameOfFunc = "ulTmrPulseOutStatus";
                funcArgs = "(mDaqDeviceHandle, timerNum, &status)\n";
                sStartTime = t.currentTime().toString("hh:mm:ss.zzz") + "~";
                err = ulTmrPulseOutStatus(mDaqDeviceHandle, timerNum, &status);
                argVals = QStringLiteral("(%1, %2, %3)")
                        .arg(mDaqDeviceHandle)
                        .arg(timerNum)
                        .arg(status);
                ui->lblInfo->setText(nameOfFunc + argVals + QString(" [Error = %1]").arg(err));

                funcStr = nameOfFunc + funcArgs + "Arg vals: " + argVals;
                if(!err==ERR_NO_ERROR) {
                    mMainWindow->setError(err, sStartTime + funcStr);
                } else {
                    mMainWindow->addFunction(sStartTime + funcStr);
                    if (status == TMRS_RUNNING)
                        tmrCheckStatus->start(300);
                }
            }
        }
    }
}

void CtrSubWidget::checkStatus()
{
    switch (mUtFunction) {
    case UL_C_INSCAN:
        checkScanStatus();
        break;
    case UL_TMR_OUT:
        checkTmrStatus();
        break;
    default:
        break;
    }
}

void CtrSubWidget::checkScanStatus()
{
    QString nameOfFunc, funcArgs, argVals, funcStr;
    ScanStatus status;
    struct TransferStatus xferStatus;
    unsigned long long currentTotalCount;
    unsigned long long currentScanCount;
    long long currentIndex;
    long long curBlock;
    QTime t;
    QString sStartTime;

    nameOfFunc = "ulCInScanStatus";
    funcArgs = "(mDaqDeviceHandle, &status, {&currentScanCount, &currentTotalCount, &currentIndex})\n";
    sStartTime = t.currentTime().toString("hh:mm:ss.zzz") + "~";
    err = ulCInScanStatus(mDaqDeviceHandle, &status, &xferStatus);
    currentIndex = xferStatus.currentIndex;
    currentScanCount = xferStatus.currentScanCount;
    currentTotalCount = xferStatus.currentTotalCount;
    argVals = QStringLiteral("(%1, %2, {%3, %4, %5})")
            .arg(mDaqDeviceHandle)
            .arg(status)
            .arg(currentScanCount)
            .arg(currentTotalCount)
            .arg(currentIndex);
    ui->lblStatus->setText(nameOfFunc + argVals + QString(" [Error = %1]").arg(err));
    mRunning = (status == SS_RUNNING);

    funcStr = nameOfFunc + funcArgs + "Arg vals: " + argVals;
    if ((!err==ERR_NO_ERROR) && mStatusTimerEnabled) {
        mStatusTimerEnabled = false;
        mMainWindow->setError(err, sStartTime + funcStr);
    } else if ((status == SS_RUNNING) && mStatusTimerEnabled) {
        curBlock = (((currentTotalCount - mPlotCount) + 1)-(mBlockSize * mChanCount));
        ui->lblStatus->setText(QString("RUNNING Total = %1 (%2 perChan), Index: %3")
                               .arg(currentTotalCount)
                               .arg(currentScanCount)
                               .arg(currentIndex));
        if (curBlock > 0) {
            long long blockAppend = (curBlock/mChanCount) - 1;
            if (mPlot) {
                plotScan(mPlotCount, mPlotIndex, mBlockSize + blockAppend);
            } else {
                printData(mPlotCount, mPlotIndex, mBlockSize + blockAppend);
            }
            mPlotIndex = currentIndex;
            mPlotCount = currentTotalCount;
        }
    } else {
        stopScan(currentScanCount, currentTotalCount, currentIndex);
    }
    ui->cmdStop->setEnabled(mGoTmrIsRunning | mRunning);
}

void CtrSubWidget::checkTmrStatus()
{
    QString nameOfFunc, funcArgs, argVals, funcStr;
    TmrStatus status;
    int timerNum, numTmrChans;
    QList<int> tmrsSelected;
    QTime t;
    QString sStartTime;

    tmrsSelected.clear();
    foreach (QCheckBox *chkTmrSel, ctrCheckBoxes) {
        if (chkTmrSel->isChecked()) {
            timerNum = chkTmrSel->property("counterNum").toInt();
            tmrsSelected.append(timerNum);
        }
    }
    numTmrChans = tmrsSelected.count();
    nameOfFunc = "ulTmrPulseOutStatus";
    funcArgs = "(mDaqDeviceHandle, timerNum, &status)\n";
    for (int i = 0; i < numTmrChans; i++) {
        timerNum = tmrsSelected[i];
        sStartTime = t.currentTime().toString("hh:mm:ss.zzz") + "~";
        err = ulTmrPulseOutStatus(mDaqDeviceHandle, timerNum, &status);
        argVals = QStringLiteral("(%1, %2, %3)")
                .arg(mDaqDeviceHandle)
                .arg(timerNum)
                .arg(status);
        ui->lblInfo->setText(nameOfFunc + argVals + QString(" [Error = %1]").arg(err));

        funcStr = nameOfFunc + funcArgs + "Arg vals: " + argVals;
        if(!err==ERR_NO_ERROR) {
            mStatusTimerEnabled = false;
            mMainWindow->setError(err, sStartTime + funcStr);
            return;
        } else {
            if (!(status == TMRS_RUNNING)) {
                mMainWindow->addFunction(sStartTime + funcStr);
                tmrCheckStatus->stop();
                ui->cmdStop->setEnabled(false);
            }
        }
    }
}

UlError CtrSubWidget::stopScan(long long perChan, long long curCount, long long curIndex)
{
    QString nameOfFunc, funcArgs, argVals, funcStr;
    long long finalBlockSize;
    QTime t;
    QString sStartTime;

    tmrCheckStatus->stop();
    nameOfFunc = "ulCInScanStop";
    funcArgs = "(mDaqDeviceHandle)\n";
    sStartTime = t.currentTime().toString("hh:mm:ss.zzz") + "~";
    err = ulCInScanStop(mDaqDeviceHandle);
    argVals = QStringLiteral("(%1)")
            .arg(mDaqDeviceHandle);

    funcStr = nameOfFunc + funcArgs + "Arg vals: " + argVals;
    if (!err==ERR_NO_ERROR) {
        mStatusTimerEnabled = false;
        mMainWindow->setError(err, sStartTime + funcStr);
        return err;
    } else {
        mMainWindow->addFunction(sStartTime + funcStr);
    }
    ui->lblInfo->setText(QStringLiteral("IDLE Count = %1 (%2 perChan), index: %3")
                           .arg(curCount)
                           .arg(perChan)
                           .arg(curIndex));
    mRunning = false;
    if (!(mChanCount == 0)) {
        finalBlockSize = (curCount - mPlotCount) / mChanCount;
        if (finalBlockSize > 1) {
            if (mPlot) {
                plotScan(mPlotCount, mPlotIndex, finalBlockSize);
            } else {
                printData(mPlotCount, mPlotIndex, finalBlockSize);
            }
        }
    }
    mPlotCount = curCount;
    mFinalCount = curCount;
    return err;
}

void CtrSubWidget::runTimerStop()
{
    QString nameOfFunc, funcArgs, argVals, funcStr;
    int timerNum;
    QTime t;
    QString sStartTime;

    tmrCheckStatus->stop();
    nameOfFunc = "ulTmrPulseOutStop";
    funcArgs = "(mDaqDeviceHandle, timerNum)\n";
    foreach (QCheckBox *chkCtrSel, ctrCheckBoxes) {
        if (chkCtrSel->isChecked()) {
            timerNum = chkCtrSel->property("counterNum").toInt();
            sStartTime = t.currentTime().toString("hh:mm:ss.zzz") + "~";
            err = ulTmrPulseOutStop(mDaqDeviceHandle, timerNum);
            argVals = QStringLiteral("(%1, %2)")
                    .arg(mDaqDeviceHandle)
                    .arg(timerNum);
            ui->lblInfo->setText(nameOfFunc + argVals + QString(" [Error = %1]").arg(err));

            funcStr = nameOfFunc + funcArgs + "Arg vals: " + argVals;
            if(!err==ERR_NO_ERROR) {
                mMainWindow->setError(err, sStartTime + funcStr);
            } else {
                mMainWindow->addFunction(sStartTime + funcStr);
                ui->cmdStop->setEnabled(false);
            }
        }
    }
    checkTmrStatus();
}

void CtrSubWidget::stopGoTimer()
{
    mStatusTimerEnabled = false;
    mUseTimer = false;
    switch (mUtFunction) {
    case UL_C_INSCAN:

        break;
    case UL_TMR_OUT:
        runTimerStop();
        break;
    default:
        break;
    }
}

void CtrSubWidget::swStopScan()
{
    //this comes from escape key on main through ChildWindow
    if (mUtFunction == UL_TMR_OUT)
        runTimerStop();
    else
        userScanStop();
}

void CtrSubWidget::plotScan(unsigned long long currentCount, long long currentIndex, int blockSize)
{
    xValues.resize(blockSize);
    double *xData = xValues.data();
    yChans.resize(mChanCount);
    for (int chan=0; chan<mChanCount; chan++)
        yChans[chan].resize(blockSize);

    int curScan, plotData, curChanCount;
    int sampleNum = 0;
    int increment = 0;
    long long totalSamples;

    totalSamples = mChanCount * ui->leNumSamples->text().toLong();

    for (int y = 0; y < blockSize; y++) {
        curScan = currentIndex + increment;
        if (!(curScan < totalSamples)) {
            currentIndex = 0;
            curScan = 0;
            increment = 0;
        }
        xData[y] = currentCount + sampleNum;
        for (int chan = 0; chan < mChanCount; chan++) {
            yChans[chan][y] = buffer[curScan + chan];
            sampleNum++;
       }
        increment +=mChanCount;
    }

    curChanCount = 1;
    if (mPlotChan == -1)
        curChanCount = mChanCount;

    for (int plotNum=0; plotNum<curChanCount; plotNum++) {
        plotData = mPlotChan;
        if (mPlotChan == -1)
            plotData = plotNum;

        if (mPlotList[plotNum])
            ui->plotCtr->graph(plotNum)->setData(xValues, yChans[plotData]);
    }
    updatePlot();
}

void CtrSubWidget::printData(unsigned long long currentCount, long long currentIndex, int blockSize)
{
    QString dataText, str, val;
    QTextCursor curCursor;
    unsigned long long curSample;
    int curScan, samplesToPrint, sampleLimit;
    int sampleNum = 0;
    int increment = 0;
    long long samplePerChanel = mChanCount * ui->leNumSamples->text().toLongLong();;

    curCursor = QTextCursor(ui->teShowValues->textCursor());
    ui->teShowValues->clear();
    dataText = "<style> th, td { padding-right: 10px;}</style><tr>";
    sampleLimit = mRunning? 100 : 5000;
    samplesToPrint = blockSize < sampleLimit? blockSize : sampleLimit;
    for (int y = 0; y < samplesToPrint; y++) {
        curScan = currentIndex + increment;
        if (!(curScan < samplePerChanel)) {
            currentIndex = 0;
            curScan = 0;
            sampleNum = 0;
        }
        //ui->teShowValues->append(QString("%1: \t")
        //                         .arg(currentCount + increment));
        dataText.append("<td>" + str.setNum(currentCount + increment) + "</td>");
        for (int chan = 0; chan < mChanCount; chan++) {
            curSample = buffer[curScan + chan];
            //curCursor.movePosition(QTextCursor::End);
            //ui->teShowValues->textCursor().setPosition(curCursor.position());
            //ui->teShowValues->insertPlainText(QString("%1\t").arg(curSample));
            dataText.append("<td>" + str.setNum(curSample) + "</td>");
        }
        dataText.append("</tr><tr>");
        sampleNum = sampleNum + 1;
        increment +=mChanCount;
    }
    dataText.append("</td></tr>");
    ui->teShowValues->setHtml(dataText);
    if (samplesToPrint < blockSize)
        ui->teShowValues->append("...");
}

void CtrSubWidget::updatePlot()
{
    bool autoScale;
    double rangeBuf;
    double rangeUpper, rangeLower;
    int ctlIndex;

    for (int plotNum=0; plotNum<mChanCount; plotNum++) {
        ctlIndex = plotNum % 8;
        rbPlotSel[ctlIndex]->setVisible(true);
        if (!mPlotList[ctlIndex])
            ui->plotCtr->graph(plotNum)->clearData();
    }
    for (int i = mChanCount; i<8; i++)
        rbPlotSel[i]->setVisible(false);
    autoScale = ui->rbAutoScale->isChecked();
    if(autoScale)
        ui->plotCtr->rescaleAxes();
    else {
        long fsCount = qPow(2, mCtrResolution);
        rangeBuf = fsCount / 10;
        rangeUpper = fsCount;
        rangeLower = 0;
        ui->plotCtr->xAxis->rescale();
        ui->plotCtr->yAxis->setRangeLower(rangeLower - rangeBuf);
        ui->plotCtr->yAxis->setRangeUpper(rangeUpper + rangeBuf);
    }
    ui->plotCtr->replot();
}

void CtrSubWidget::replot()
{
    updatePlot();
}

void CtrSubWidget::plotSelect()
{
    for (int i = 0; i<8; i++)
        mPlotList[i] = rbPlotSel[i]->isChecked();

    if (!mRunning)
        plotScan(mPlotCount, mPlotIndex, mBlockSize);
}
