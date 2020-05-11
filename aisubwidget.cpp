#include "aisubwidget.h"
#include "ui_aisubwidget.h"
#include <qfont.h>
#include "childwindow.h"
#include "unitest.h"
//#include "ul.h"
//#include "plotwindow.h"
//#include "testutilities.h"

AiSubWidget::AiSubWidget(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::AiSubWidget)
{
    ui->setupUi(this);

    buffer = NULL;
    tmrCheckStatus = new QTimer(this);
    mAutoStop  = true;
    mUseGetStatus = true;
    mUseWait = false;
    mInitPlot = true;
    //int fontSize;
    QFont font;

    mFontSize = 8;
    font.setPointSize(10);

#ifdef Q_OS_MAC
    mFontSize = 12;
    font.setPointSize(12);
    this->setFont(font);
#endif

    ui->teShowValues->setFont(QFont ("Courier", mFontSize));
    ui->lblRateReturned->setFont(QFont ("Courier", mFontSize));
    ui->frame->setFont(QFont ("Ubuntu", mFontSize));
    ui->fraScan->setFont(QFont ("Ubuntu", mFontSize));
    ui->teShowValues->setStyleSheet("QTextEdit { background-color : white; color : blue; }" );
    ui->lblRateReturned->setStyleSheet("QLabel { background-color : white; color : blue; }" );
    ui->lblStatus->setStyleSheet("QLabel { color : blue; }" );
    ui->lblInfo->setStyleSheet("QLabel { color : blue; }" );
    connect(tmrCheckStatus, SIGNAL(timeout()), this, SLOT(checkStatus()));
    connect(ui->cmdGo, SIGNAL(clicked(bool)), this, SLOT(onClickCmdGo()));
    connect(ui->cmdStop, SIGNAL(clicked(bool)), this, SLOT(onClickCmdStop()));

    connect(ui->AiPlot->xAxis, SIGNAL(rangeChanged(QCPRange)),
            ui->AiPlot->xAxis2, SLOT(setRange(QCPRange)));
    connect(ui->AiPlot->yAxis, SIGNAL(rangeChanged(QCPRange)),
            ui->AiPlot->yAxis2, SLOT(setRange(QCPRange)));
    connect(ui->rbAutoScale, SIGNAL(clicked(bool)), this, SLOT(replot()));
    connect(ui->rbFullScale, SIGNAL(clicked(bool)), this, SLOT(replot()));
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
    /*connect(ui->rbPlot0, SIGNAL(clicked(bool)), this, SLOT(plotSelect()));
    connect(ui->rbPlot1, SIGNAL(clicked(bool)), this, SLOT(plotSelect()));
    connect(ui->rbPlot2, SIGNAL(clicked(bool)), this, SLOT(plotSelect()));
    connect(ui->rbPlot3, SIGNAL(clicked(bool)), this, SLOT(plotSelect()));
    connect(ui->rbPlot4, SIGNAL(clicked(bool)), this, SLOT(plotSelect()));
    connect(ui->rbPlot5, SIGNAL(clicked(bool)), this, SLOT(plotSelect()));
    connect(ui->rbPlot6, SIGNAL(clicked(bool)), this, SLOT(plotSelect()));
    connect(ui->rbPlot7, SIGNAL(clicked(bool)), this, SLOT(plotSelect()));*/
    //connect(ui->actionAuto_scale, SIGNAL(triggered()), this, SLOT(setPlotAutoScale()));
    connect(this, SIGNAL(dataReady(unsigned long long, unsigned long long, int)), this,
            SLOT(showData(unsigned long long, unsigned long long, int)));

    ui->cmdStop->setVisible(false);
    mOneSampPerForTotalSamps = false;
    mTotalRead = 0;
    mPlot = false;
    mPlotChan = -1;
    mPlotCount = 0;
    mEventType = DE_NONE;
    mGoTmrIsRunning = false;
    mRunning = false;
    mPrintResolution = 5;
    ui->cmdStop->setEnabled(false);
    setupPlot(ui->AiPlot, 1);
    ui->AiPlot->replot();
    mMainWindow = getMainWindow();
    for (int i = 0; i < 8; i++)
        mPlotList[i] = true;
}

AiSubWidget::~AiSubWidget()
{
    delete ui;
}

void AiSubWidget::keyPressEvent(QKeyEvent *event)
{
    int keyCode = event->key();
    if (keyCode == Qt::Key_F6)
        if (!mPlot)
            updateData();
    if (keyCode == Qt::Key_Escape) {
        onClickCmdStop();
    }
    if ((keyCode == Qt::Key_Plus)  && (QApplication::keyboardModifiers() & Qt::AltModifier)) {
        mPrintResolution += 1;
        ui->lblInfo->setText(QString("Text resolution %1").arg(mPrintResolution));
    }
    if ((keyCode == Qt::Key_Minus)  && (QApplication::keyboardModifiers() & Qt::AltModifier)) {
        mPrintResolution -= 1;
        if (mPrintResolution < 0)
            mPrintResolution = 0;
        ui->lblInfo->setText(QString("Text resolution %1").arg(mPrintResolution));
    }
    if ((keyCode == Qt::Key_X) && (QApplication::keyboardModifiers() & Qt::AltModifier)) {
        mShowHex = !mShowHex;
        ui->lblStatus->setText(QString("Print hex %1").arg(mShowHex));
    }
    if ((keyCode == Qt::Key_Period)  && (QApplication::keyboardModifiers() & Qt::AltModifier)) {
        if(mDevName.startsWith("DT9837"))
            mCalcPeriod = true;
        else
            mCalcTime = true;
        ui->lblInfo->setText(QString("Enabled pulse/period calculation"));
    }
    if ((keyCode == Qt::Key_Slash)  && (QApplication::keyboardModifiers() & Qt::AltModifier)) {
        mCalcTime = false;
        mCalcPeriod = false;
        ui->lblInfo->setText(QString("Disabled pulse/period calculation"));
    }
#ifdef Q_OS_MAC
    if (keyCode == Qt::Key_F5) {
        this->setWindowState(Qt::WindowMaximized);
        this->setWindowState(Qt::WindowNoState);
    }
#endif
}

MainWindow *AiSubWidget::getMainWindow()
{
    foreach (QWidget *w, QApplication::topLevelWidgets())
        if (QMainWindow* mainWin = qobject_cast<QMainWindow*>(w))
            return qobject_cast<MainWindow *>(mainWin);
    return nullptr;
}

void AiSubWidget::updateParameters()
{
    ChildWindow *parentWindow;
    bool showStop;
    QString funcName;

    //QString devUID = mDevUID;
    parentWindow = qobject_cast<ChildWindow *>(this->parent());
    mDaqDeviceHandle = parentWindow->devHandle();
    mDevName = parentWindow->devName();
    mDevUID = parentWindow->devUID();

    showStop = false;
    mUseGetStatus = parentWindow->statusEnabled();
    mUseWait = parentWindow->waitEnabled();
    mWaitTime = parentWindow->waitTime();

    mInputMode = parentWindow->inputMode();
    mAiFlags = parentWindow->aiFlags();
    mTiFlags = parentWindow->tiFlags();
    mTiArrayFlags = parentWindow->tiArrayFlags();
    mDaqiFlags = parentWindow->daqiFlags();
    mTriggerType = parentWindow->triggerType();
    mTrigChannel = parentWindow->trigChannel();
    mTrigLevel = parentWindow->trigLevel();
    mTrigVariance = parentWindow->trigVariance();
    mRetrigCount = parentWindow->retrigCount();
    mScanOptions = parentWindow->scanOptions();
    QString optNames = getOptionNames(mScanOptions);
    if ((mUtFunction == UL_AINSCAN) | (mUtFunction == UL_DAQ_INSCAN)) {
        funcName = "UlDaqInScan(";
        if (mUtFunction == UL_AINSCAN)
                funcName = "UlAinScan(";
        ui->lblInfo->setText(funcName + optNames + ")");
        showStop = true;
    }
    mUseTimer = parentWindow->tmrEnabled();
    mOneSampPerForTotalSamps = parentWindow->tmrSampPerInterval();
    mStopOnStart = parentWindow->tmrStopOnStart();
    mAutoStop = parentWindow->stopBGEnabled();
    mGoTmrIsRunning = parentWindow->tmrRunning();
    if (mUseTimer | showStop) {
        ui->cmdStop->setVisible(true);
        ui->cmdStop->setEnabled(mGoTmrIsRunning);
    }

    ui->leBlockSize->setEnabled(mUseGetStatus);
    //if (devUID != mDevUID)
   //     initDeviceParams();
    this->setWindowTitle(mFuncName + ": " + mDevName + QString(" [%1]").arg(mDaqDeviceHandle));
}

void AiSubWidget::initDeviceParams()
{
    QString nameOfFunc, funcArgs, argVals, funcStr;
    AiInfoItem infoItem;
    long long infoValue;
    QTime t;
    QString sStartTime;

    nameOfFunc = "ulAIGetInfo";
    funcArgs = "(mDaqDeviceHandle, infoItem, 0, &infoValue)\n";

    infoItem = AI_INFO_RESOLUTION;
    sStartTime = t.currentTime().toString("hh:mm:ss.zzz") + "~";
    err = ulAIGetInfo(mDaqDeviceHandle, infoItem, 0, &infoValue);
    argVals = QString("(%1, %2, %3, %4)")
            .arg(mDaqDeviceHandle)
            .arg(infoItem)
            .arg(0)
            .arg(infoValue);

    funcStr = nameOfFunc + funcArgs + "Arg vals: " + argVals;
    if (err == ERR_NO_ERROR) {
        mMainWindow->addFunction(sStartTime + funcStr);
        mAiResolution = infoValue;
    } else {
        if (err == ERR_BAD_DEV_TYPE)
            return;
        mMainWindow->setError(err, sStartTime + funcStr);
    }
    ui->cmdGo->setFocus();
}

void AiSubWidget::groupChanged(int newGroup)
{
    mCurGroup = newGroup;
    //setUiForGroup();
}

void AiSubWidget::functionChanged(int utFunction)
{
    mUtFunction = utFunction;
    this->setUiForFunction();
}

void AiSubWidget::setupPlot(QCustomPlot *dataPlot, int chanCount)
{
    QColor penColor;
    QPalette brushColor;

    //brushColor = QPalette::background();
    int chanCycle;
    int curChanCount;
    dataPlot->clearGraphs();
    dataPlot->setBackground(brushColor.window());
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

void AiSubWidget::showPlotWindow(bool showIt)
{
    QFrame::Shape frameShape;

    mPlot = showIt;
    int curIndex = 0;
    if ((mUtFunction == UL_AINSCAN) | (mUtFunction == UL_DAQ_INSCAN))
        mInitPlot = mPlot;
    frameShape = QFrame::Box;

    if (showIt) {
        curIndex = 1;
        frameShape = QFrame::NoFrame;
    } else {
        mTextIndex = 0;
        updateData();
    }

    ui->stackedWidget->setFrameShape(frameShape);
    ui->stackedWidget->setCurrentIndex(curIndex);
}

void AiSubWidget::showQueueConfig()
{
    queueSetup = new QueueDialog(this);
    if (mChanList.count()) {
        queueSetup->setChanList(mChanList);
        queueSetup->setChanTypeList(mChanTypeList);
        queueSetup->setModeList(mModeList);
        queueSetup->setRangeList(mRangeList);
        queueSetup->setNumQueueElements(mChanList.count());
    }
    connect(queueSetup, SIGNAL(accepted()), this, SLOT(queueDialogResponse()));
    queueSetup->exec();
}

void AiSubWidget::queueDialogResponse()
{
    mChanList = queueSetup->chanList();
    mChanTypeList = queueSetup->chanTypeList();
    mModeList = queueSetup->modeList();
    mRangeList = queueSetup->rangeList();

    disconnect(queueSetup);
    delete queueSetup;
    if (mUtFunction == UL_AINSCAN)
        setupQueue();
    if (mUtFunction == UL_DAQ_INSCAN) {
        unsigned int numElements = mChanList.count();
        ui->spnLowChan->setValue(0);
        ui->spnHighChan->setValue(numElements - 1);
    }
}

void AiSubWidget::setupQueue()
{
    QString nameOfFunc, funcArgs, argVals, funcStr;
    unsigned int numElements = mChanList.count();
    QTime t;
    QString sStartTime;

    struct AiQueueElement queue[numElements];
    foreach (int key, mChanList.keys()) {
        queue[key].channel = mChanList.value(key);
        queue[key].inputMode = mModeList.value(key);
        queue[key].range = mRangeList.value(key);
    }

    nameOfFunc = "ulAInLoadQueue";
    funcArgs = "(mDaqDeviceHandle, {channel, mode, range}, numElements)\n";

    sStartTime = t.currentTime().toString("hh:mm:ss.zzz") + "~";
    err = ulAInLoadQueue(mDaqDeviceHandle, queue, numElements);
    argVals = QStringLiteral("(%1, {%2, %3, %4}, %5)")
            .arg(mDaqDeviceHandle)
            .arg(queue[0].channel)
            .arg(queue[0].inputMode)
            .arg(queue[0].range)
            .arg(numElements);
    ui->lblInfo->setText(nameOfFunc + argVals + QString(" [Error = %1]").arg(err));

    funcStr = nameOfFunc + funcArgs + "Arg vals: " + argVals;
    if (err != ERR_NO_ERROR) {
        mMainWindow->setError(err, sStartTime + funcStr);
        return;
    } else {
        mMainWindow->addFunction(sStartTime + funcStr);
    }
    int chanAdjust = 0;
    if (numElements)
        chanAdjust = 1;
    ui->spnLowChan->setValue(0);
    ui->spnHighChan->setValue(numElements - chanAdjust);
}

void AiSubWidget::setUiForFunction()
{
    ChildWindow *parentWindow;
    parentWindow = qobject_cast<ChildWindow *>(this->parent());
    mRange = parentWindow->getCurrentRange();
    mScale = parentWindow->getCurrentScale();
    bool scanVisible;

    mChanList.clear();
    mRangeList.clear();
    mChanTypeList.clear();
    mPlot = false;
    scanVisible = false;
    switch (mUtFunction) {
    case UL_AIN:
        mFuncName = "ulAIn";
        ui->leNumSamples->setText("10");
        mInitPlot = true;
        break;
    case UL_AINSCAN:
        mFuncName = "ulAInScan";
        scanVisible = true;
        if (mInitPlot) {
            ui->leRate->setText("1000");
            ui->leNumSamples->setText("1000");
            ui->leBlockSize->setText("1000");
            mPlot = true;
        }
        break;
    case UL_DAQ_INSCAN:
        mFuncName = "ulDaqInScan";
        scanVisible = true;
        if (mInitPlot) {
            ui->leRate->setText("1000");
            ui->leNumSamples->setText("1000");
            ui->leBlockSize->setText("1000");
            mPlot = true;
        }
        //set default values for chanDescriptor
        mChanList.insert(0, 0);
        mRangeList.insert(0, mRange);
        mChanTypeList.insert(0, DAQI_ANALOG_DIFF);
        break;
    case UL_TIN:
        mFuncName = "ulTIn";
        ui->leNumSamples->setText("10");
        break;
    case UL_TINARRAY:
        mFuncName = "ulTInArray";
        ui->leNumSamples->setText("10");
        break;
    default:
        break;
    }
    ui->fraScan->setVisible(scanVisible);
    ui->cmdStop->setEnabled(false);
    showPlotWindow(mPlot);
    parentWindow->setShowPlot(mPlot);
    this->setWindowTitle(mFuncName + ": " + mDevName + QString(" [%1]").arg(mDaqDeviceHandle));
    ui->cmdGo->setFocus();
}

void AiSubWidget::updateText(QString infoText)
{
    ui->lblInfo->setText(infoText);
}

void AiSubWidget::onClickCmdGo()
{
    ChildWindow *parentWindow;
    parentWindow = qobject_cast<ChildWindow *>(this->parent());
    bool tmrIsEnabled;

    mPlotIndex = 0;
    mPlotCount = 0;
    mTotalRead = 0;
    tmrIsEnabled = parentWindow->tmrEnabled();
    mUseTimer = tmrIsEnabled;
    ui->teShowValues->clear();
    ui->lblRateReturned->clear();
    ui->lblStatus->clear();
    ui->lblInfo->clear();
    runSelectedFunc();
#ifdef Q_OS_MAC
    this->setWindowState(Qt::WindowMaximized);
    this->setWindowState(Qt::WindowNoState);
#endif
}

void AiSubWidget::runSelectedFunc()
{
    ChildWindow *parentWindow;
    QFont goFont = ui->cmdGo->font();
    bool makeBold, tmrIsEnabled, tmrIsRunning;
    bool showStop;

    mPlotIndex = 0;
    mPlotCount = 0;
    parentWindow = qobject_cast<ChildWindow *>(this->parent());
    mRange = parentWindow->getCurrentRange();
    mScale = parentWindow->getCurrentScale();
    //mRange = (Range)curRange;
    switch (mUtFunction) {
    case UL_AIN:
        runAInFunc();
        break;
    case UL_AINSCAN:
        mTriggerType = parentWindow->triggerType();
        if (mTriggerType != TRIG_NONE) {
            mTrigChannel = parentWindow->trigChannel();
            mTrigLevel = parentWindow->trigLevel();
            mTrigVariance = parentWindow->trigVariance();
            mRetrigCount = parentWindow->retrigCount();
            qApp->processEvents();
            runSetTriggerFunc();
        }
        runAInScanFunc();
        break;
    case UL_DAQ_INSCAN:
        mChanCount = mChanList.count();
        if (mChanCount) {
            for (int i = 0; i < mChanCount; i++) {
                chanDescriptors[i].channel = mChanList.value(i);
                chanDescriptors[i].range = mRangeList.value(i);
                chanDescriptors[i].type = mChanTypeList.value(i);
            }
        }
        mTriggerType = parentWindow->triggerType();
        if (mTriggerType != TRIG_NONE) {
            mTrigChannel = parentWindow->trigChannel();
            mTrigChanType = (DaqInChanType)parentWindow->trigChanType();
            mTrigLevel = parentWindow->trigLevel();
            mTrigVariance = parentWindow->trigVariance();
            mRetrigCount = parentWindow->retrigCount();
            qApp->processEvents();
            runSetTriggerFunc();
        }
        runDaqInScanFunc();
        break;
    case UL_TIN:
        runTInFunc();
        break;
    case UL_TINARRAY:
        runTInArray();
        break;
    default:
        break;
    }

    tmrIsEnabled = parentWindow->tmrEnabled();
    tmrIsRunning = parentWindow->tmrRunning();

    showStop = ((mUtFunction == UL_AINSCAN) | (mUtFunction == UL_DAQ_INSCAN));
    if (!tmrIsEnabled) {
        if (tmrIsRunning)
            parentWindow->setTmrRunning(false);
        ui->cmdStop->setVisible(showStop);
        mUseTimer = false;
        goFont.setBold(false);
        ui->cmdGo->setFont(goFont);
    } else {
        if (mUseTimer) {
            if (!tmrIsRunning) {
                parentWindow->setTmrRunning(true);
                ui->cmdStop->setVisible(true);
                ui->cmdStop->setEnabled(true);
            }
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
}

void AiSubWidget::runSetTriggerFunc()
{
    QString nameOfFunc, funcArgs, argVals, funcStr;
    QTime t;
    QString sStartTime, chanDescString, str;

    if (mUtFunction == UL_DAQ_INSCAN) {
        DaqInChanDescriptor trigChanDescriptor;
        //int trigKey = mChanList.key(mTrigChannel, 0);
        trigChanDescriptor.channel = mTrigChannel;
        trigChanDescriptor.range = mTrigRange;
        trigChanDescriptor.type = mTrigChanType;
        nameOfFunc = "ulDaqInSetTrigger";
        funcArgs = "(mDaqDeviceHandle, trigType, {trigChan, trigRange, trigChanType}, level, variance, trigCount)\n";
        sStartTime = t.currentTime().toString("hh:mm:ss.zzz") + "~";
        err = ulDaqInSetTrigger(mDaqDeviceHandle, mTriggerType, trigChanDescriptor, mTrigLevel, mTrigVariance, mRetrigCount);
        chanDescString = QString("{%1, %2, %3}")
                .arg(mTrigChannel)
                .arg(mTrigRange)
                .arg(mTrigChanType);
    } else {
        nameOfFunc = "ulAInSetTrigger";
        funcArgs = "(mDaqDeviceHandle, trigType, trigChan, level, variance, trigCount)\n";
        sStartTime = t.currentTime().toString("hh:mm:ss.zzz") + "~";
        err = ulAInSetTrigger(mDaqDeviceHandle, mTriggerType, mTrigChannel, mTrigLevel, mTrigVariance, mRetrigCount);
        chanDescString = str.setNum(mTrigChannel);
    }
    argVals = QStringLiteral("(%1, %2, %3, %4, %5, %6)")
            .arg(mDaqDeviceHandle)
            .arg(mTriggerType)
            .arg(chanDescString)
            .arg(mTrigLevel)
            .arg(mTrigVariance)
            .arg(mRetrigCount);
    ui->lblInfo->setText(nameOfFunc + argVals + QString(" [Error = %1]").arg(err));

    funcStr = nameOfFunc + funcArgs + "Arg vals: " + argVals;
    if (err != ERR_NO_ERROR) {
        mMainWindow->setError(err, sStartTime + funcStr);
    } else {
        mMainWindow->addFunction(sStartTime + funcStr);
    }
}

void AiSubWidget::runEventSetup()
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
    if (getEndOutScanEvent(mDaqDeviceHandle, eventValue)) {
        //if this exists, enable or disable according to eventValue
        if (eventValue) {
            disableAll = false;
            if (!(mEventType & DE_ON_END_OF_OUTPUT_SCAN))
                enableList = DE_ON_END_OF_OUTPUT_SCAN;
        } else {
            disableList = DE_ON_END_OF_OUTPUT_SCAN;
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
            if (mEventParams != eventParam) {
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
    if (getOutScanErrorEvent(mDaqDeviceHandle, eventValue)) {
        //if this exists, enable or disable according to eventValue
        if (eventValue) {
            disableAll = false;
            if (!(mEventType & DE_ON_OUTPUT_SCAN_ERROR))
                enableList |= DE_ON_OUTPUT_SCAN_ERROR;
        } else {
            disableList |= DE_ON_OUTPUT_SCAN_ERROR;
        }
    }
    if (disableAll)
        disableList = (int)ALL_EVENT_TYPES;

    eventsEnabled = (DaqEventType)enableList;
    eventsDisabled = (DaqEventType)disableList;

    if (eventsEnabled != DE_NONE)
        runEventEnable(eventsEnabled, eventParam);
    if (eventsDisabled != DE_NONE)
        runEventDisable(eventsDisabled);
}

void AiSubWidget::runEventEnable(DaqEventType eventType, unsigned long long eventParam)
{
    QString nameOfFunc, funcArgs, argVals, funcStr;
    QTime t;
    QString sStartTime;

    DaqEventCallback eventCallbackFunction = &AiSubWidget::eventCallback;

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

    int newEventType = 0;
    funcStr = nameOfFunc + funcArgs + "Arg vals: " + argVals;
    if (err != ERR_NO_ERROR) {
        mMainWindow->setError(err, sStartTime + funcStr);
    } else {
        mMainWindow->addFunction(sStartTime + funcStr);
        newEventType = mEventType | eventType;
        mEventType = DaqEventType(newEventType);
        mEventParams = eventParam;
    }
}

void AiSubWidget::runEventDisable(DaqEventType eventType)
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
    if (err != ERR_NO_ERROR) {
        mMainWindow->setError(err, sStartTime + funcStr);
    } else {
        mMainWindow->addFunction(sStartTime + funcStr);
        int mask = eventType ^ (int)ALL_EVENT_TYPES;
        DaqEventType newEvent = (DaqEventType)(mask & mEventType);
        mEventType = newEvent;
        removeEventsByHandle(mDaqDeviceHandle, eventType);
    }
}

void AiSubWidget::eventCallback(DaqDeviceHandle devHandle, DaqEventType eventType, unsigned long long eventData, void* userData)
{
    AiSubWidget *aiChild = (AiSubWidget *)userData;
    if (devHandle == 0)
        eventType = DE_ON_INPUT_SCAN_ERROR;
    aiChild->callbackHandler(eventType, eventData);
}

void AiSubWidget::callbackHandler(DaqEventType eventType, unsigned long long eventData)
{
    //called from eventCallback
    int finalBlockSize;
    QString nameOfFunc, funcArgs, argVals, funcStr;
    QTime t;
    QString sStartTime;

    finalBlockSize = 0;
    nameOfFunc = "eventCallback";
    funcArgs = "(devHandle, eventType, eventData, *userData)\n";
    sStartTime = t.currentTime().toString("hh:mm:ss.zzz") + "~";
    argVals = QStringLiteral("(%1, %2, %3, %4)")
            .arg(mDaqDeviceHandle)
            .arg(eventType)
            .arg(eventData)
            .arg(this->windowTitle());

    funcStr = nameOfFunc + funcArgs + "Arg vals: " + argVals;
    mMainWindow->addFunction(sStartTime + funcStr);

    switch (eventType) {
    case DE_ON_DATA_AVAILABLE:
        ui->lblStatus->setText(QString("Data Available: %1, TotalCount: %2, Index: %3")
                               .arg(eventData)
                               .arg(mPlotCount)
                               .arg(mPlotIndex));
        //emit dataReady(mPlotCount, mPlotIndex, mEventParams);
        finalBlockSize = eventData - (mPlotCount / mChanCount);
        emit dataReady(mPlotCount, mPlotIndex, finalBlockSize);
        break;
    case DE_ON_INPUT_SCAN_ERROR:
        ui->cmdStop->setEnabled(false);
        ui->lblStatus->setText(QString("Error event: %1").arg(eventData));
        break;
    case DE_ON_END_OF_INPUT_SCAN:
        ui->cmdStop->setEnabled(false);
        ui->lblStatus->setText(QString("Scan end event: %1 samples acquired").arg(eventData));
        finalBlockSize = eventData - (mPlotCount / mChanCount);
        if (finalBlockSize > 1)
            emit dataReady(mPlotCount, mPlotIndex, finalBlockSize);
        break;
    default:
        break;
    }
    //mPlotCount = eventData * mChanCount;
    mPlotCount += (finalBlockSize * mChanCount);
    mPlotIndex = mPlotCount % (mSamplesPerChan * mChanCount);
}

void AiSubWidget::showData(unsigned long long curTotalCount, unsigned long long curIndex, int blockSize)
{
    if (mPlot) {
        plotScan(curTotalCount, curIndex, blockSize);
    } else {
        printData(curTotalCount, curIndex, blockSize);
    }
}

void AiSubWidget::runAInFunc()
{
    QString dataText, str, val;
    int aInChan, aInLastChan;   //, numAinChans
    int samplesToRead, curIndex;    //numSamples,
    double data, curSample;
    QString nameOfFunc, funcArgs, argVals, funcStr;
    QString showSign = "+";
    int afterDecimal, bufIndex; //totalZ,
    QTime t;
    QString errDesc = "";
    QString errNumStr;
    QString sStartTime;

    aInChan = ui->spnLowChan->value();
    aInLastChan = ui->spnHighChan->value();
    //numAinChans = (aInLastChan - aInChan) + 1;
    //numSamples = ui->leNumSamples->text().toLong();
    mChanCount = (aInLastChan - aInChan) + 1;
    mSamplesPerChan = ui->leNumSamples->text().toLong();

    samplesToRead = mSamplesPerChan;
    curIndex = 0;
    if(mOneSampPerForTotalSamps) {
        samplesToRead = 1;
        curIndex = mTotalRead * mChanCount;
    }

    if((!mOneSampPerForTotalSamps) | (mTotalRead == 0)) {
        if (buffer) {
            delete[] buffer;
            buffer = NULL;
            mPlotIndex = 0;
            mPlotCount = 0;
            mTotalRead = 0;
        }

        long long bufSize = mChanCount * mSamplesPerChan;
        mBufSize = bufSize;
        buffer = new double[bufSize];
        memset(buffer, 0, mBufSize * sizeof(*buffer));
        if(mPlot)
            setupPlot(ui->AiPlot, mChanCount);
    }

    /*QVector<double> dataVal(numAinChans);
    QVector<QVector<double>>  dataArray(numSamples);
    for (int sample=0; sample<numSamples; sample++)
        dataArray[sample].resize(numAinChans);*/

    nameOfFunc = "ulAIn";
    funcArgs = "(mDaqDeviceHandle, aInChan, inputMode, range, flags, &data)\n";

    for (long sampleNum = 0; sampleNum < samplesToRead; sampleNum++) {
        //curIndex = 0;
        for (int curChan = aInChan; curChan <= aInLastChan; curChan ++) {
            sStartTime = t.currentTime().toString("hh:mm:ss.zzz") + "~";
            data = -110011;
            err = ulAIn(mDaqDeviceHandle, curChan, mInputMode, mRange, mAiFlags, &data);
            argVals = QStringLiteral("(%1, %2, %3, %4, %5, %6)")
                    .arg(mDaqDeviceHandle)
                    .arg(aInChan)
                    .arg(mInputMode)
                    .arg(mRange)
                    .arg(mAiFlags)
                    .arg(data);
            ui->lblInfo->setText(nameOfFunc + argVals + QString(" [Error = %1]").arg(err));
            buffer[curIndex] = data;
            //dataVal[curIndex] = data;
            //dataArray[sampleNum][curIndex] = dataVal[curIndex];

            funcStr = nameOfFunc + funcArgs + "Arg vals: " + argVals;
            if (err != ERR_NO_ERROR) {
                errNumStr = QString("[Error %1: ").arg(err);
                switch (err) {
                case ERR_OPEN_CONNECTION:
                    errDesc = errNumStr + "Open thermocouple]";
                    break;
                default:
                    break;
                }
                if (errDesc.length()) {
                    mMainWindow->addFunction(sStartTime + funcStr + errDesc);
                } else {
                    mUseTimer = false;
                    mMainWindow->setError(err, sStartTime + funcStr);
                    return;
                }
            } else {
                mMainWindow->addFunction(sStartTime + funcStr);
            }
            curIndex++;
        }
        mTotalRead += 1;
    }

    if(mPlot)
        plotScan(0, 0, mTotalRead);
    else {
        afterDecimal = mPrintResolution;
        if (mAiFlags & AIN_FF_NOSCALEDATA) {
                afterDecimal = 0;
                showSign = "";
        }
        funcStr = nameOfFunc + argVals;
        curIndex = 0;
        bufIndex = 0;
        ui->teShowValues->clear();
        dataText = "<style> th, td { padding-right: 10px;}</style><tr>";
        for (long thisSample = 0; thisSample < mSamplesPerChan; thisSample++) {
            dataText.append("<td>" + str.setNum(thisSample * mChanCount) + "</td>");
            for (curIndex = 0; curIndex < mChanCount; curIndex++) {
                curSample = buffer[bufIndex];
                val = QString("%1%2").arg((curSample < 0) ? "" : showSign).arg
                        (curSample, 1, 'f', afterDecimal, '0');
                dataText.append("<td>" + val + "</td>");
                bufIndex++;
            }
            dataText.append("</tr><tr>");
        }
        dataText.append("</td></tr>");
        ui->teShowValues->setHtml(dataText);
    }

    if(mOneSampPerForTotalSamps) {
        if(mTotalRead == mSamplesPerChan) {
            mUseTimer = false;
            mPlotIndex = 0;
            mPlotCount = 0;
            mTotalRead = 0;
        }
    }
}

void AiSubWidget::runTInFunc()
{
    QString dataText, str, val;
    int tInChan, tInLastChan;
    int samplesToRead, curIndex;
    double data, curSample;
    QString nameOfFunc, funcArgs, argVals, funcStr;
    QString errNumStr;
    QString errDesc = "";
    QString showSign = "+";
    int afterDecimal, totalZ, bufIndex;
    QTime t;
    QString sStartTime;

    //mTiFlags = TIN_FF_DEFAULT; //currently the only value
    tInChan = ui->spnLowChan->value();
    tInLastChan = ui->spnHighChan->value();
    mChanCount = (tInLastChan - tInChan) + 1;
    mSamplesPerChan = ui->leNumSamples->text().toLong();

    samplesToRead = mSamplesPerChan;
    curIndex = 0;
    if(mOneSampPerForTotalSamps) {
        samplesToRead = 1;
        curIndex = mTotalRead * mChanCount;
    }

    if((!mOneSampPerForTotalSamps) | (mTotalRead == 0)) {
        if (buffer) {
            delete[] buffer;
            buffer = NULL;
        }

        long long bufSize = mChanCount * mSamplesPerChan;
        mBufSize = bufSize;
        buffer = new double[bufSize];
        memset(buffer, 0, mBufSize * sizeof(*buffer));
        if(mPlot)
            setupPlot(ui->AiPlot, mChanCount);
    }

    /*QVector<double> dataVal(numtInChans);
    QVector<QVector<double>>  dataArray(numSamples);
    for (int sample=0; sample<numSamples; sample++)
        dataArray[sample].resize(numtInChans);*/

    nameOfFunc = "ulTIn";
    funcArgs = "(mDaqDeviceHandle, channel, scale, flags, &data)\n";

    for (long sampleNum = 0; sampleNum < samplesToRead; sampleNum++) {
        //curIndex = 0;
        for (int curChan = tInChan; curChan <= tInLastChan; curChan ++) {
            sStartTime = t.currentTime().toString("hh:mm:ss.zzz") + "~";
            data = -110011;
            err = ulTIn(mDaqDeviceHandle, curChan, mScale, mTiFlags, &data);
            argVals = QStringLiteral("(%1, %2, %3, %4, %5)")
                    .arg(mDaqDeviceHandle)
                    .arg(curChan)
                    .arg(mScale)
                    .arg(mTiFlags)
                    .arg(data);
            ui->lblInfo->setText(nameOfFunc + argVals + QString(" [Error = %1]").arg(err));
            buffer[curIndex] = data;
            //dataArray[sampleNum][curIndex] = dataVal[curIndex];

            funcStr = nameOfFunc + funcArgs + "Arg vals: " + argVals;
            if (err != ERR_NO_ERROR) {
                errNumStr = QString("[Error %1: ").arg(err);
                switch (err) {
                case ERR_OPEN_CONNECTION:
                    errDesc = errNumStr + "Open thermocouple]";
                    break;
                default:
                    break;
                }
                if (errDesc.length()) {
                    mMainWindow->addFunction(sStartTime + funcStr + errDesc);
                } else {
                    mUseTimer = false;
                    mMainWindow->setError(err, sStartTime + funcStr);
                    return;
                }
            } else {
                mMainWindow->addFunction(sStartTime + funcStr);
            }
            curIndex++;
        }
        mTotalRead += 1;
    }

    if(mPlot)
        plotScan(0, 0, mTotalRead);
    else {
        afterDecimal = 5;
        totalZ = afterDecimal;
        funcStr = nameOfFunc + argVals;
        curIndex = 0;
        bufIndex = 0;
        ui->teShowValues->clear();
        dataText = "<style> th, td { padding-right: 10px;}</style><tr>";
        for (long thisSample = 0; thisSample < mSamplesPerChan; thisSample++) {
            dataText.append("<td>" + str.setNum(thisSample * mChanCount) + "</td>");
            for (curIndex = 0; curIndex < mChanCount; curIndex++) {
                curSample = buffer[bufIndex];
                totalZ = afterDecimal;
                if(curSample < -500)
                    totalZ = 1;
                val = QString("%1%2").arg((curSample < 0) ? "" : showSign).arg
                        (curSample, 2, 'f', totalZ, '0');
                dataText.append("<td>" + val + "</td>");
                bufIndex++;
            }
            dataText.append("</tr><tr>");
        }
        dataText.append("</td></tr>");
        ui->teShowValues->setHtml(dataText);
    }

    if(mOneSampPerForTotalSamps) {
        if(mTotalRead == mSamplesPerChan) {
            mUseTimer = false;
        }
    }
}

void AiSubWidget::runTInArray()
{
    QString dataText, str, val;
    int lowChan, highChan;
    int samplesToRead, curIndex;
    double curSample;
    double *data;
    QString errNumStr;
    QString nameOfFunc, funcArgs, argVals, funcStr;
    QString showSign = "+";
    int afterDecimal, totalZ, bufIndex;
    QTime t;
    QString sStartTime;

    //mTiArrFlags = TINARRAY_FF_DEFAULT; //currently the only value
    lowChan = ui->spnLowChan->value();
    highChan = ui->spnHighChan->value();
    mChanCount = (highChan - lowChan) + 1;
    mSamplesPerChan = ui->leNumSamples->text().toLong();
    data = new double[mChanCount];

    samplesToRead = mSamplesPerChan;
    curIndex = 0;
    if(mOneSampPerForTotalSamps) {
        samplesToRead = 1;
        curIndex = mTotalRead * mChanCount;
    }

    if((!mOneSampPerForTotalSamps) | (mTotalRead == 0)) {
        if (buffer) {
            delete[] buffer;
            buffer = NULL;
        }

        long long bufSize = mChanCount * mSamplesPerChan;
        mBufSize = bufSize;
        buffer = new double[bufSize];
        memset(buffer, 0, mBufSize * sizeof(*buffer));
        if(mPlot)
            setupPlot(ui->AiPlot, mChanCount);
    }

    nameOfFunc = "ulTInArray";
    funcArgs = "(mDaqDeviceHandle, lowChan, highChan, scale, flags, &data)\n";

    for (long sampleNum = 0; sampleNum < samplesToRead; sampleNum++) {
        sStartTime = t.currentTime().toString("hh:mm:ss.zzz") + "~";
        err = ulTInArray(mDaqDeviceHandle, lowChan, highChan, mScale, mTiArrayFlags, data);
        argVals = QStringLiteral("(%1, %2, %3, %4, %5, %6)")
                .arg(mDaqDeviceHandle)
                .arg(lowChan)
                .arg(highChan)
                .arg(mScale)
                .arg(mTiArrayFlags)
                .arg(data[0]);

        QString errDesc = "";
        ui->lblInfo->setText(nameOfFunc + argVals + QString(" [Error = %1]").arg(err));
        funcStr = nameOfFunc + funcArgs + "Arg vals: " + argVals;
        if (err != ERR_NO_ERROR) {
            errNumStr = QString("[Error %1: ").arg(err);
            switch (err) {
            case ERR_OPEN_CONNECTION:
                errDesc = errNumStr + "Open thermocouple]";
                break;
            default:
                break;
            }
            if (errDesc.length()) {
                mMainWindow->addFunction(sStartTime + funcStr + errDesc);
            } else {
                mUseTimer = false;
                mMainWindow->setError(err, sStartTime + funcStr);
                return;
            }
        } else {
            mMainWindow->addFunction(sStartTime + funcStr);
        }

        for (int curChan = 0; curChan < mChanCount; curChan ++) {
            buffer[curIndex] = data[curChan];
            curIndex++;
        }

        mTotalRead += 1;
    }

    if(mPlot)
        plotScan(0, 0, mTotalRead);
    else {
        afterDecimal = 5;
        totalZ = afterDecimal;
        funcStr = nameOfFunc + argVals;
        curIndex = 0;
        bufIndex = 0;
        ui->teShowValues->clear();
        dataText = "<style> th, td { padding-right: 10px;}</style><tr>";
        for (long thisSample = 0; thisSample < mSamplesPerChan; thisSample++) {
            dataText.append("<td>" + str.setNum(thisSample * mChanCount) + "</td>");
            for (curIndex = 0; curIndex < mChanCount; curIndex++) {
                curSample = buffer[bufIndex];
                totalZ = afterDecimal;
                if(curSample < -500)
                    totalZ = 1;
                val = QString("%1%2").arg((curSample < 0) ? "" : showSign).arg
                        (curSample, 2, 'f', totalZ, '0');
                dataText.append("<td>" + val + "</td>");
                bufIndex++;
            }
            dataText.append("</tr><tr>");
        }
        dataText.append("</td></tr>");
        ui->teShowValues->setHtml(dataText);
    }

    if(mTotalRead == mSamplesPerChan) {
        mUseTimer = false;
    }
}

void AiSubWidget::runAInScanFunc()
{
    int lowChan, highChan;
    QString nameOfFunc, funcArgs, argVals, funcStr;
    QTime t;
    QString sStartTime, blockStyle;
    QString fsString, warnColor;

    fsString = "font-size: 8pt; ";
    if (mFontSize == 12)
        fsString = "font-size: 12pt; ";
    warnColor = "background-color:white; ";
    lowChan = ui->spnLowChan->value();
    highChan = ui->spnHighChan->value();
    mBlockSize = ui->leBlockSize->text().toLongLong();
    mChanCount = (highChan - lowChan) + 1;
    if(mChanCount < 1) mChanCount = 1;
    mSamplesPerChan = ui->leNumSamples->text().toLong();
    double rate = ui->leRate->text().toDouble();

    if(mSamplesPerChan < mBlockSize * 1.8) {
        if(mScanOptions & SO_CONTINUOUS)
            warnColor = "background-color:yellow; ";
        if(mSamplesPerChan < mBlockSize)
            warnColor += "color:red; ";
    }
    blockStyle = "QLineEdit {" + warnColor + fsString + "}";
    ui->leBlockSize->setStyleSheet(blockStyle);

    long long bufSize = mChanCount * mSamplesPerChan;
    if(mPlot)
        setupPlot(ui->AiPlot, mChanCount);

    mFunctionFlag = (AInScanFlag)mAiFlags;
    if (mStopOnStart && mAutoStop) {
        nameOfFunc = "ulAInScanStop";
        funcArgs = "(mDaqDeviceHandle)";
        sStartTime = t.currentTime().toString("hh:mm:ss.zzz") + "~";
        err = ulAInScanStop(mDaqDeviceHandle);
        argVals = QString("(%1)")
                .arg(mDaqDeviceHandle);

        funcStr = nameOfFunc + funcArgs + "Arg vals: " + argVals;
        if (err != ERR_NO_ERROR) {
            mMainWindow->setError(err, sStartTime + funcStr);
        } else {
            mMainWindow->addFunction(sStartTime + funcStr);
        }
    }

    if (buffer) {
        delete[] buffer;
        buffer = NULL;
    }

    mBufSize = bufSize;
    buffer = new double[bufSize];
    memset(buffer, 0, mBufSize * sizeof(*buffer));

    nameOfFunc = "ulAInScan";
    funcArgs = "(mDaqDeviceHandle, lowChan, highChan, inputMode, "
               "range, mSamplesPerChan, &rate, mScanOptions, flags, &data)\n";

    sStartTime = t.currentTime().toString("hh:mm:ss.zzz") + "~";
    err = ulAInScan(mDaqDeviceHandle, lowChan, highChan, mInputMode,
                        mRange, mSamplesPerChan, &rate, mScanOptions, mFunctionFlag, buffer);
    argVals = QString("(%1, %2, %3, %4, %5, %6, %7, %8, %9, %10)")
            .arg(mDaqDeviceHandle)
            .arg(lowChan)
            .arg(highChan)
            .arg(mInputMode)
            .arg(mRange)
            .arg(mSamplesPerChan)
            .arg(rate)
            .arg(mScanOptions)
            .arg(mFunctionFlag)
            .arg(buffer[0]);
    ui->lblInfo->setText(nameOfFunc + argVals + QString(" [Error = %1]").arg(err));

    funcStr = nameOfFunc + funcArgs + "Arg vals: " + argVals;
    if (err != ERR_NO_ERROR) {
        if (mAutoStop)
            ulAInScanStop(mDaqDeviceHandle);
        mStatusTimerEnabled = false;
        mMainWindow->setError(err, sStartTime + funcStr);
    } else {
        mMainWindow->addFunction(sStartTime + funcStr);
        ui->lblRateReturned->setText(QString("%1").arg(rate, 1, 'f', mPrintResolution, '0'));
        if (mUseWait) {
            qApp->processEvents();
            WaitType waitType = WAIT_UNTIL_DONE;
            long long waitParam = 0;
            nameOfFunc = "ulAInScanWait";
            funcArgs = "(mDaqDeviceHandle, waitType, waitParam, timeout)\n";
            sStartTime = t.currentTime().toString("hh:mm:ss.zzz") + "~";
            err = ulAInScanWait(mDaqDeviceHandle, waitType, waitParam, mWaitTime);
            argVals = QString("(%1, %2, %3, %4)")
                    .arg(mDaqDeviceHandle)
                    .arg(waitType)
                    .arg(waitParam)
                    .arg(mWaitTime);

            funcStr = nameOfFunc + funcArgs + "Arg vals: " + argVals;
            if (err != ERR_NO_ERROR) {
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

void AiSubWidget::runDaqInScanFunc()
{
    QTime t;
    QString sStartTime, blockStyle;
    double rate;
    QString nameOfFunc, funcArgs, argVals, funcStr;
    QString fsString, warnColor;

    fsString = "font-size: 8pt; ";
    if (mFontSize == 12)
        fsString = "font-size: 12pt; ";
    warnColor = "background-color:white; ";
    ui->spnLowChan->setValue(0);
    ui->spnHighChan->setValue(mChanCount - 1);
    mBlockSize = ui->leBlockSize->text().toLongLong();
    if(mChanCount < 1) mChanCount = 1;
    mSamplesPerChan = ui->leNumSamples->text().toLong();
    rate = ui->leRate->text().toDouble();

    if(mSamplesPerChan < mBlockSize * 1.8) {
        if(mScanOptions & SO_CONTINUOUS)
            warnColor = "background-color:yellow; ";
        if(mSamplesPerChan < mBlockSize)
            warnColor += "color:red; ";
    }
    blockStyle = "QLineEdit {" + warnColor + fsString + "}";
    ui->leBlockSize->setStyleSheet(blockStyle);

    long long bufSize = mChanCount * mSamplesPerChan;
    if(mPlot)
        setupPlot(ui->AiPlot, mChanCount);

    if (mStopOnStart && mAutoStop) {
        nameOfFunc = "ulDaqInScanStop";
        funcArgs = "(mDaqDeviceHandle)";
        sStartTime = t.currentTime().toString("hh:mm:ss.zzz") + "~";
        err = ulDaqInScanStop(mDaqDeviceHandle);
        argVals = QString("(%1)")
                .arg(mDaqDeviceHandle);

        funcStr = nameOfFunc + funcArgs + "Arg vals: " + argVals;
        if (err != ERR_NO_ERROR) {
            mMainWindow->setError(err, sStartTime + funcStr);
        } else {
            mMainWindow->addFunction(sStartTime + funcStr);
        }
    }

    if (buffer) {
        delete[] buffer;
        buffer = NULL;
    }

    mBufSize = bufSize;
    buffer = new double[bufSize];
    memset(buffer, 0, mBufSize * sizeof(*buffer));

    nameOfFunc = "ulDaqInScan";
    funcArgs = "(mDaqDeviceHandle, chanDescriptors, numChans, "
               "mSamplesPerChan, rate, options, flags, data)\n";

    sStartTime = t.currentTime().toString("hh:mm:ss.zzz") + "~";
    err = ulDaqInScan(mDaqDeviceHandle, chanDescriptors, mChanCount,
                      mSamplesPerChan, &rate, mScanOptions, mDaqiFlags, buffer);
    argVals = QString("(%1, {%2, %3, %4}, %5, %6, %7, %8, %9, %10)")
            .arg(mDaqDeviceHandle)
            .arg(chanDescriptors[0].channel)
            .arg(chanDescriptors[0].range)
            .arg(chanDescriptors[0].type)
            .arg(mChanCount)
            .arg(mSamplesPerChan)
            .arg(rate)
            .arg(mScanOptions)
            .arg(mDaqiFlags)
            .arg(buffer[0]);
    ui->lblInfo->setText(nameOfFunc + argVals + QString(" [Error = %1]").arg(err));

    funcStr = nameOfFunc + funcArgs + "Arg vals: " + argVals;
    if (err != ERR_NO_ERROR) {
        mMainWindow->setError(err, sStartTime + funcStr);
    } else {
        mMainWindow->addFunction(sStartTime + funcStr);
        ui->lblRateReturned->setText(QString("%1").arg(rate, 1, 'f', mPrintResolution, '0'));
        if (mUseWait) {
            qApp->processEvents();
            WaitType waitType = WAIT_UNTIL_DONE;
            long long waitParam = 0;
            nameOfFunc = "ulDaqInScanWait";
            funcArgs = "(mDaqDeviceHandle, waitType, waitParam, timeout)\n";

            sStartTime = t.currentTime().toString("hh:mm:ss.zzz") + "~";
            err = ulDaqInScanWait(mDaqDeviceHandle, waitType, waitParam, mWaitTime);
            argVals = QString("(%1, %2, %3, %4)")
                    .arg(mDaqDeviceHandle)
                    .arg(waitType)
                    .arg(waitParam)
                    .arg(mWaitTime);

            funcStr = nameOfFunc + funcArgs + "Arg vals: " + argVals;
            if (err != ERR_NO_ERROR) {
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

void AiSubWidget::onClickCmdStop()
{
    QString nameOfFunc, funcArgs, argVals, funcStr;
    QTime t;
    QString sStartTime;

    mUseTimer = false;
    if ((mUtFunction == UL_AINSCAN) | (mUtFunction == UL_DAQ_INSCAN)) {
        ScanStatus status;
        struct TransferStatus xferStatus;
        unsigned long long currentScanCount = 0;
        unsigned long long currentTotalCount = 0;
        long long currentIndex = 0;

        if(mUseGetStatus) {
            funcArgs = "(mDaqDeviceHandle, &status, &currentScanCount, &currentTotalCount, &currentIndex)\n";
            mStatusTimerEnabled = false;
            if (mUtFunction == UL_AINSCAN) {
                nameOfFunc = "ulAInScanStatus";
                sStartTime = t.currentTime().toString("hh:mm:ss.zzz") + "~";
                err = ulAInScanStatus(mDaqDeviceHandle, &status, &xferStatus);
            } else {
                nameOfFunc = "ulDaqInScanStatus";
                sStartTime = t.currentTime().toString("hh:mm:ss.zzz") + "~";
                err = ulDaqInScanStatus(mDaqDeviceHandle, &status, &xferStatus);
            }
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
            if (err != ERR_NO_ERROR) {
                mMainWindow->setError(err, sStartTime + funcStr);
            } else {
                mMainWindow->addFunction(sStartTime + funcStr);
            }
        }
        err = stopScan(currentScanCount, currentTotalCount, currentIndex);
        ui->cmdStop->setEnabled(mGoTmrIsRunning | mRunning);
    }
}

void AiSubWidget::checkStatus()
{
    QString nameOfFunc, funcArgs, argVals, funcStr;
    ScanStatus status;
    QTime t;
    QString sStartTime;
    struct TransferStatus xferStatus;
    unsigned long long currentTotalCount;
    unsigned long long currentScanCount;
    long long currentIndex;
    long long curBlock;

    funcArgs = "(mDaqDeviceHandle, &status, &currentScanCount, &currentTotalCount, &currentIndex)\n";
    if (mUtFunction == UL_DAQ_INSCAN) {
        nameOfFunc = "ulDaqInScanStatus";
        sStartTime = t.currentTime().toString("hh:mm:ss.zzz") + "~";
        err = ulDaqInScanStatus(mDaqDeviceHandle, &status, &xferStatus);
    } else {
        nameOfFunc = "ulAInScanStatus";
        sStartTime = t.currentTime().toString("hh:mm:ss.zzz") + "~";
        err = ulAInScanStatus(mDaqDeviceHandle, &status, &xferStatus);
    }
    currentIndex = xferStatus.currentIndex;
    currentScanCount = xferStatus.currentScanCount;
    currentTotalCount = xferStatus.currentTotalCount;
    argVals = QStringLiteral("(%1, %2, %3, %4, %5)")
            .arg(mDaqDeviceHandle)
            .arg(status)
            .arg(currentScanCount)
            .arg(currentTotalCount)
            .arg(currentIndex);
    ui->lblStatus->setText(nameOfFunc + argVals + QString(" [Error = %1]").arg(err));
    mRunning = (status == SS_RUNNING);

    funcStr = nameOfFunc + funcArgs + "Arg vals: " + argVals;
    if ((err != ERR_NO_ERROR) && mStatusTimerEnabled) {
        mStatusTimerEnabled = false;
        mMainWindow->setError(err, sStartTime + funcStr);
    } else if ((status == SS_RUNNING) && mStatusTimerEnabled) {
        //to do - add switch to log status calls
        //mMainWindow->addFunction(sStartTime + funcStr);
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

            mPlotIndex = currentIndex + mChanCount;
            mPlotCount = currentTotalCount;
        }
    } else {
        stopScan(currentScanCount, currentTotalCount, currentIndex);
    }
    ui->cmdStop->setEnabled(mGoTmrIsRunning | mRunning);
}

UlError AiSubWidget::stopScan(long long perChan, long long curCount, long long curIndex)
{
    QString nameOfFunc, funcArgs, argVals, funcStr;
    long long finalBlockSize;
    QTime t;
    QString sStartTime;

    tmrCheckStatus->stop();
    if(mAutoStop) {
        nameOfFunc = "ulAInScanStop";
        funcArgs = "(mDaqDeviceHandle)\n";
        if (mUtFunction == UL_DAQ_INSCAN) {
            nameOfFunc = "ulDaqInScanStop";
            sStartTime = t.currentTime().toString("hh:mm:ss.zzz") + "~";
            err = ulDaqInScanStop(mDaqDeviceHandle);
        } else {
            nameOfFunc = "ulAInScanStop";
            sStartTime = t.currentTime().toString("hh:mm:ss.zzz") + "~";
            err = ulAInScanStop(mDaqDeviceHandle);
        }
        argVals = QStringLiteral("(%1)")
                .arg(mDaqDeviceHandle);

        funcStr = nameOfFunc + funcArgs + "Arg vals: " + argVals;
        if (err != ERR_NO_ERROR) {
            mStatusTimerEnabled = false;
            mMainWindow->setError(err, sStartTime + funcStr);
            return err;
        } else {
            mMainWindow->addFunction(sStartTime + funcStr);
        }
    }
    if(mUseGetStatus) {
        ui->lblStatus->setText(QStringLiteral("IDLE Count = %1 (%2 perChan), index: %3")
                               .arg(curCount)
                               .arg(perChan)
                               .arg(curIndex));
        mRunning = false;
        if (mChanCount != 0) {
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
    } else {
        mRunning = false;
        ui->cmdStop->setEnabled(false);
    }
    return err;
}

void AiSubWidget::swStopScan()
{
    //this comes from escape key on main through ChildWindow
    onClickCmdStop();
}

void AiSubWidget::updateData()
{
    QString dataText, str, val;
    bool floatValue, hasFloat, hasInt;
    bool isCounter;

    floatValue = false;
    int increment = mTextIndex;
    int samplesToPrint;
    double curSample, ctrRate0, ctrRate;
    double tickFactor, countComp;

    countComp = 0;
    tickFactor = 1;
    ctrRate0 = 1;
    ctrRate = 1;
    isCounter = false;
    floatValue = false;
    hasFloat = false;
    hasInt = false;
    if (mCalcPeriod) {
        ctrRate0 = (double)1/12000000;
        ctrRate = (double)1/48000000;
        countComp = 1;
    }

    if (!buffer)
        return;
    if(mBlockSize == 0)
        return;
    if (mUtFunction == UL_DAQ_INSCAN) {
        hasInt = (mDaqiFlags & DAQINSCAN_FF_NOSCALEDATA);
        floatValue = !hasInt;
        for (int tList = 0; tList < mChanTypeList.count(); tList++) {
            if ((mChanTypeList[tList] == DAQI_ANALOG_DIFF)
                    |(mChanTypeList[tList] == DAQI_ANALOG_SE)
                    |(mChanTypeList[tList] == DAQI_DAC)) hasFloat = true;
            if ((mChanTypeList[tList] == DAQI_CTR16)
                    |(mChanTypeList[tList] == DAQI_CTR32)
                    |(mChanTypeList[tList] == DAQI_CTR48)
                    |(mChanTypeList[tList] == DAQI_DIGITAL))
                hasInt = true;
        }
        mMixedFloatInt = (hasFloat & hasInt);
    }
    if (mUtFunction == UL_AINSCAN) {
        floatValue = (!(mAiFlags & AINSCAN_FF_NOSCALEDATA));
        hasInt = true;
    }
    //print only 1000
    samplesToPrint = (mSamplesPerChan < 1000? mSamplesPerChan : 1000) / mChanCount;
    if ((samplesToPrint + mTextIndex) > (mSamplesPerChan * mChanCount))
        samplesToPrint = ((mSamplesPerChan * mChanCount) - mTextIndex) / mChanCount;
    ui->teShowValues->clear();
    dataText = "<style> th, td { padding-right: 10px;}</style><tr>";
    for (int y = 0; y < samplesToPrint; y++) {
        dataText.append("<td>" + str.setNum(increment) + "</td>");
        for (int chan = 0; chan < mChanCount; chan++) {
            curSample = buffer[increment + chan];
            if (hasInt) {
                floatValue = false;
                if ((mChanTypeList[chan] == DAQI_ANALOG_DIFF)
                        |(mChanTypeList[chan] == DAQI_ANALOG_SE)
                        |(mChanTypeList[chan] == DAQI_DAC)) floatValue = true;
            }
            if (floatValue) {
                val = QString("%1%2").arg((curSample < 0) ? "" : "+")
                        .arg(curSample, 1, 'f', mPrintResolution, '0');
            } else {
                isCounter = (mChanTypeList[chan] == DAQI_CTR32);
                if (mCalcPeriod & isCounter) {
                    tickFactor = ctrRate;
                    if (mChanList[chan] == 0)
                        tickFactor = ctrRate0;
                }
                if(mShowHex)
                    val = QString("0x%1").arg((long)curSample, mHexResolution, 16, QLatin1Char('0'));
                else {
                    double calcSample = tickFactor * (curSample - countComp);
                    val = QString("%1").arg(calcSample, 1, 'g', mPrintResolution, '0');
                }
            }
            dataText.append("<td>" + val + "</td>");
        }
        dataText.append("</tr><tr>");
        increment += mChanCount;
        mTextIndex = increment;
    }
    dataText.append("</td></tr>");
    ui->teShowValues->setHtml(dataText);
    if (mTextIndex >= (mSamplesPerChan * mChanCount))
        mTextIndex = 0;
    else
        ui->teShowValues->append("... (F6)");
}

void AiSubWidget::printData(unsigned long long currentCount, long long currentIndex, int blockSize)
{
    QString dataText, str, val;
    double curSample, ctrRate0, ctrRate;
    double tickFactor, countComp;
    int curScan, samplesToPrint, sampleLimit;
    int sampleNum = 0;
    int increment = 0;
    bool floatValue, hasFloat, hasInt;
    bool printBlocks, isCounter;
    long long samplePerChanel = mChanCount * ui->leNumSamples->text().toLongLong();

    mTextIndex = 0;
    countComp = 0;
    tickFactor = 1;
    ctrRate0 = 1;
    ctrRate = 1;
    isCounter = false;
    floatValue = false;
    hasFloat = false;
    hasInt = false;
    printBlocks = (mRunning && (mScanOptions & SO_CONTINUOUS));
    if (mCalcPeriod) {
        ctrRate0 = (double)1/12000000;
        ctrRate = (double)1/48000000;
        countComp = 1;
    }
    mMixedFloatInt = false;
    if (mUtFunction == UL_DAQ_INSCAN) {
        hasInt = (mDaqiFlags & DAQINSCAN_FF_NOSCALEDATA);
        floatValue = !hasInt;
        for (int tList = 0; tList < mChanTypeList.count(); tList++) {
            if ((mChanTypeList[tList] == DAQI_ANALOG_DIFF)
                    |(mChanTypeList[tList] == DAQI_ANALOG_SE)
                    |(mChanTypeList[tList] == DAQI_DAC)) hasFloat = true;
            if ((mChanTypeList[tList] == DAQI_CTR16)
                    |(mChanTypeList[tList] == DAQI_CTR32)
                    |(mChanTypeList[tList] == DAQI_CTR48)
                    |(mChanTypeList[tList] == DAQI_DIGITAL))
                hasInt = true;
        }
        mMixedFloatInt = (hasFloat & hasInt);
    }
    if (mUtFunction == UL_AINSCAN) {
        floatValue = (!(mAiFlags & AINSCAN_FF_NOSCALEDATA));
        hasInt = true;
    }

    ui->teShowValues->clear();
    dataText = "<style> th, td { padding-right: 10px;}</style><tr>";
    sampleLimit = printBlocks? 100 : 1000 / mChanCount;
    samplesToPrint = blockSize < sampleLimit? blockSize : sampleLimit;
    for (int y = 0; y < samplesToPrint; y++) {
        curScan = currentIndex + increment;
        if (curScan >= samplePerChanel) {
            currentIndex = 0;
            curScan = 0;
            sampleNum = 0;
        }
        dataText.append("<td>" + str.setNum(currentCount + increment) + "</td>");
        for (int chan = 0; chan < mChanCount; chan++) {
            curSample = buffer[curScan + chan];
            if (hasInt) {
                floatValue = false;
                if ((mChanTypeList[chan] == DAQI_ANALOG_DIFF)
                        |(mChanTypeList[chan] == DAQI_ANALOG_SE)
                        |(mChanTypeList[chan] == DAQI_DAC)) floatValue = true;
            }
            if (floatValue) {
                val = QString("%1%2").arg((curSample < 0) ? "" : "+")
                        .arg(curSample, 1, 'f', mPrintResolution, '0');
            } else {
                isCounter = (mChanTypeList[chan] == DAQI_CTR32);
                if (mCalcPeriod & isCounter) {
                    tickFactor = ctrRate;
                    if (mChanList[chan] == 0)
                        tickFactor = ctrRate0;
                }
                if(mShowHex)
                    val = QString("0x%1").arg((long)curSample, mHexResolution, 16, QLatin1Char('0'));
                else {
                    double calcSample = tickFactor * (curSample - countComp);
                    val = QString("%1").arg(calcSample, 1, 'g', mPrintResolution, '0');
                }
            }
            dataText.append("<td>" + val + "</td>");
        }
        dataText.append("</tr><tr>");
        sampleNum = sampleNum + 1;
        increment +=mChanCount;
    }
    dataText.append("</td></tr>");
    ui->teShowValues->setHtml(dataText);
    if (samplesToPrint < blockSize) {
        ui->teShowValues->append("... (F6)");
        mTextIndex = increment;
    }
}

void AiSubWidget::plotScan(unsigned long long currentCount, long long currentIndex, int blockSize)
{
    xValues.resize(blockSize);
    double *xData = xValues.data();
    yChans.resize(mChanCount);
    for (int chan=0; chan<mChanCount; chan++)
        yChans[chan].resize(blockSize);

    int curScan, plotData, curChanCount;
    int listIndex;
    int sampleNum = 0;
    int increment = 0;
    long long totalSamples;

    totalSamples = mChanCount * ui->leNumSamples->text().toLong();

    for (int y = 0; y < blockSize; y++) {
        curScan = currentIndex + increment;
        if (curScan >= totalSamples) {
            currentIndex = 0;
            curScan = 0;
            increment = 0;
        }
        xData[y] = currentCount + sampleNum;
        for (int chan = 0; chan < mChanCount; chan++) {
            yChans[chan][y] = buffer[curScan + chan];
            sampleNum++;
            /*if ((chan == mEvalChan) && mEvalData) {
                if (priorSamp != -20) {
                    sampDiff = fabs(buffer[curScan + chan] - priorSamp);
                }
                priorSamp = buffer[curScan + chan];
                if (sampDiff > mDeltaValue) {
                    if(mRunning) stopScan(currentCount, currentIndex);
                    mEvalData = false;
                    deltaFail = true;
                    dfSample = x[y] + chan;
                }
            }*/
       }
        increment +=mChanCount;
    }

    curChanCount = 1;
    if (mPlotChan == -1)
        curChanCount = mChanCount;

    for (int plotNum=0; plotNum<curChanCount; plotNum++) {
        plotData = mPlotChan;
        listIndex = plotNum % 8;
        if (mPlotChan == -1)
            plotData = plotNum;

        if (mPlotList[listIndex])
            ui->AiPlot->graph(plotNum)->setData(xValues, yChans[plotData]);
    }
    updatePlot();
}

void AiSubWidget::updatePlot()
{
    bool setTCRange = false;
    bool autoScale, bipolar;
    double rangeBuf;
    double rangeUpper, rangeLower;
    int ctlIndex;

    for (int plotNum=0; plotNum<mChanCount; plotNum++) {
        ctlIndex = plotNum % 8;
        rbPlotSel[ctlIndex]->setVisible(true);
        if (!mPlotList[ctlIndex])
            ui->AiPlot->graph(plotNum)->clearData();
    }
    for (int i = mChanCount; i<8; i++)
        rbPlotSel[i]->setVisible(false);
    autoScale = ui->rbAutoScale->isChecked();
    if(autoScale) {
        ui->AiPlot->rescaleAxes();
        double center = ui->AiPlot->yAxis->range().center();
        ui->AiPlot->yAxis->scaleRange(1.2, center);
    } else {
        if (mRange == BIPPT078VOLTS) {
            AiConfigItem configItem = AI_CFG_CHAN_TYPE;
            unsigned int index = 0;
            long long configValue;
            UlError err = ulAIGetConfig(mDaqDeviceHandle,
                                        configItem,  index, &configValue);
            if ((err == ERR_NO_ERROR) && (configValue == 2))
                setTCRange = true;
        }
        if (mUtFunction == UL_TIN)
            setTCRange = true;

        if (setTCRange) {
            rangeBuf = 0;
            rangeUpper = 50;
            rangeLower = 10;
        } else {
            if (mAiFlags && AIN_FF_NOSCALEDATA) {
                long fsCount = qPow(2, mAiResolution);
                rangeBuf = fsCount / 10;
                rangeUpper = fsCount;
                rangeLower = 0;
            } else {
                bipolar = mRange < 100;
                double rangeVolts = getRangeVolts(mRange);
                rangeBuf = rangeVolts / 10;
                rangeUpper = bipolar? rangeVolts / 2 : rangeVolts;
                rangeLower = bipolar? rangeUpper * -1 : 0;
            }
        }
        ui->AiPlot->xAxis->rescale();
        ui->AiPlot->yAxis->setRangeLower(rangeLower - rangeBuf);
        ui->AiPlot->yAxis->setRangeUpper(rangeUpper + rangeBuf);
    }
    ui->AiPlot->replot();
}

void AiSubWidget::replot()
{
    updatePlot();
}

void AiSubWidget::plotSelect()
{
    for (int i = 0; i<8; i++)
        mPlotList[i] = rbPlotSel[i]->isChecked();

    if (!mRunning)
        plotScan(mPlotCount, mPlotIndex, mBlockSize);
}

//stub slots for childwindow signals

void AiSubWidget::showDataGen()
{
    return;
}

