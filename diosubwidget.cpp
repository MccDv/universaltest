#include "diosubwidget.h"
#include "ui_diosubwidget.h"
#include <qfont.h>
#include "childwindow.h"
#include "testutilities.h"
#include "unitest.h"
#include "uldaq.h"
#include "errordialog.h"


DioSubWidget::DioSubWidget(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::DioSubWidget)
{
    ui->setupUi(this);
    int fontSize;
    QFont font;

    tmrCheckStatus = new QTimer(this);
    mUseGetStatus = true;
    mUseWait = false;
    fontSize = 8;
    font.setPointSize(10);

#ifdef Q_OS_MAC
    fontSize = 12;
    font.setPointSize(12);
    this->setFont(font);
#endif

    ui->teShowValues->setFont(QFont ("Courier", fontSize));
    //ui->lblRateReturned->setFont(QFont ("Ubuntu", fontSize));
    ui->fraInfoStat->setFont(QFont ("Courier", fontSize));
    ui->teShowValues->setStyleSheet("QTextEdit { background-color : white; color : blue; }" );
    ui->lblStatus->setStyleSheet("QLabel { color : blue; } ");
    ui->lblInfo->setStyleSheet("QLabel { color : blue; } ");
    ui->lblRateReturned->setStyleSheet("QLabel { background-color : white; color : blue; }" );

    connect(tmrCheckStatus, SIGNAL(timeout()), this, SLOT(checkStatus()));
    connect(ui->cmdToggle, SIGNAL(clicked(bool)), this, SLOT(togglePortSelection()));
    connect(ui->cmdConfigIn, SIGNAL(clicked(bool)), this, SLOT(configureInputs()));
    connect(ui->cmdConfigOut, SIGNAL(clicked(bool)), this, SLOT(configureOutputs()));
    connect(ui->cmdGo, SIGNAL(clicked(bool)), this, SLOT(onClickCmdGo()));
    connect(ui->cmdStop, SIGNAL(clicked(bool)), this, SLOT(stopGoTimer()));
    //connect(ui->teShowValues, SIGNAL(textChanged()), this, SLOT(setFuncFocus()));
    connect(ui->plotDigitalData->xAxis, SIGNAL(rangeChanged(QCPRange)),
            ui->plotDigitalData->xAxis2, SLOT(setRange(QCPRange)));
    connect(ui->plotDigitalData->yAxis, SIGNAL(rangeChanged(QCPRange)),
            ui->plotDigitalData->yAxis2, SLOT(setRange(QCPRange)));
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
    connect(this, SIGNAL(dataReady(unsigned long long, unsigned long long, int)), this,
            SLOT(showData(unsigned long long, unsigned long long, int)));

    //ui->cmdStop->setVisible(false);
    buffer = NULL;
    mPlot = false;
    mPlotChan = -1;
    mChanCount = 1;
    mEventType = DE_NONE;
    ui->cmdStop->setEnabled(false);
    setupPlot(ui->plotDigitalData, 1);
    ui->plotDigitalData->replot();
    ui->cmdStop->setVisible(false);

    portCheckBoxes.clear();
    portCheckBoxes = ui->fraPorts->findChildren<QCheckBox*>();
    createBitBoxes();
    mMainWindow = getMainWindow();
    for (int i = 0; i < 8; i++)
        mPlotList[i] = true;
}

DioSubWidget::~DioSubWidget()
{
    delete ui;
    if (buffer) {
        delete[] buffer;
        buffer = NULL;
    }
}

MainWindow *DioSubWidget::getMainWindow()
{
    foreach (QWidget *w, QApplication::topLevelWidgets())
        if (QMainWindow* mainWin = qobject_cast<QMainWindow*>(w))
            return qobject_cast<MainWindow *>(mainWin);
    return nullptr;
}

void DioSubWidget::updateParameters()
{
    ChildWindow *parentWindow;
    bool showStop = false;
    bool enableIt = false;

    parentWindow = qobject_cast<ChildWindow *>(this->parent());
    mDaqDeviceHandle = parentWindow->devHandle();
    mDevName = parentWindow->devName();
    mDevUID = parentWindow->devUID();

    mUseGetStatus = parentWindow->statusEnabled();
    mUseWait = parentWindow->waitEnabled();
    mWaitTime = parentWindow->waitTime();

    mTriggerType = parentWindow->triggerType();
    mTrigChannel = parentWindow->trigChannel();
    mTrigLevel = parentWindow->trigLevel();
    mTrigVariance = parentWindow->trigVariance();
    mRetrigCount = parentWindow->retrigCount();
    mScanOptions = parentWindow->scanOptions();
    QString optNames = getOptionNames(mScanOptions);
    if (mUtFunction == UL_D_OUTSCAN) {
        ui->lblInfo->setText("UlDOutScan(" + optNames + ")");
        showStop = true;
    }
    if (mUtFunction == UL_D_INSCAN) {
        ui->lblInfo->setText("UlDInScan(" + optNames + ")");
        showStop = true;
    }

    mUseTimer = parentWindow->tmrEnabled();
    bool tmrIsRunning = parentWindow->tmrRunning();
    if (mUseTimer) {
        showStop = true;
        enableIt = (tmrIsRunning | (mUtFunction == UL_D_CONFIG_BIT));
        ui->cmdStop->setEnabled(enableIt);
    }

    mStopOnStart = parentWindow->tmrStopOnStart();
    mGoTmrIsRunning = parentWindow->tmrRunning();
    if (mUseTimer) {
        enableIt = (tmrIsRunning | (mUtFunction == UL_D_CONFIG_BIT));
        ui->cmdStop->setEnabled(enableIt);
    }

    ui->cmdStop->setVisible(showStop | (mUtFunction == UL_D_CONFIG_BIT));
    ui->cmdStop->setEnabled(mUtFunction == UL_D_CONFIG_BIT);
    ui->leBlockSize->setEnabled(mUseGetStatus);
    this->setWindowTitle(mFuncName + ": " + mDevName + QString(" [%1]").arg(mDaqDeviceHandle));
}

void DioSubWidget::groupChanged(int newGroup)
{
    mCurGroup = newGroup;
}

void DioSubWidget::functionChanged(int utFunction)
{
    mUtFunction = utFunction;
    portList.clear();
    foreach (QCheckBox *chkPort, portCheckBoxes) {
        if (chkPort->isChecked()) {
            int portNum = chkPort->property("portNum").toInt();
            DigitalPortType portType = (DigitalPortType)portNum;
            portList.append(portType);
        }
    }

    disableCheckboxInput(false);
    this->setUiForFunction();
}

void DioSubWidget::setUiForFunction()
{
    bool scanVisible, portsVisible, stopVisible;
    bool scanParamsVisible, setNumberVisible, asyncVisible;
    int stackIndex;
    ChildWindow *parentWindow;

    parentWindow = qobject_cast<ChildWindow *>(this->parent());
    parentWindow->setTmrEnabled(false);

    portsVisible = false;
    asyncVisible = false;
    scanVisible = false;
    scanParamsVisible = false;
    setNumberVisible = false;
    stackIndex = 0;
    stopVisible = false;
    mPlot = false;
    QString goText = "Read";
    QString stopText = "Stop";
    QString sampToolTip = "Value read";
    QString startSample = "7";
    QString rateVal = "1000";
    QString blockText = "1000";
    int defPort;
    switch (mUtFunction) {
    case UL_D_CONFIG_PORT:
        mFuncName = "ulDConfigPort";
        portsVisible = true;
        asyncVisible = true;
        setNumberVisible = true;
        if (mCurGroup == FUNC_GROUP_DOUT) {
            sampToolTip = "Value to write";
            goText = "Write";
        }
        break;
    case UL_D_CONFIG_BIT:
        mFuncName = "ulDConfigBit";
        stackIndex = 1;
        asyncVisible = true;
        stopVisible = true;
        sampToolTip = "Bit and Port to read/write (bit, port format)";
        stopText = "Write";
        setNumberVisible = true;
        foreach (DigitalPortType pt, validPorts) {
            setDefaultBits(pt);
        }
        defPort = (int)validPorts[0];
        startSample = QString("0, %1").arg(defPort);
        break;
    case UL_D_IN:
        mFuncName = "ulDIn";
        asyncVisible = true;
        startSample = "10";
        sampToolTip = "Samples per channel";
        setNumberVisible = true;
        break;
    case UL_D_OUT:
        portsVisible = true;
        mFuncName = "ulDOut";
        goText = "Write";
        asyncVisible = true;
        setNumberVisible = true;
        break;
    case UL_D_BIT_IN:
        mFuncName = "ulDBitIn";
        asyncVisible = true;
        stackIndex = 1;
        break;
    case UL_D_BIT_OUT:
        mFuncName = "ulDBitOut";
        asyncVisible = true;
        goText = "Read";
        stackIndex = 1;
        break;
    case UL_D_INSCAN:
        mFuncName = "ulDInScan";
        asyncVisible = true;
        goText = "Go";
        sampToolTip = "Samples per channel";
        startSample = "1000";
        scanVisible = true;
        mPlot = true;
        setNumberVisible = true;
        mPlot = true;
        stackIndex = 2;
        break;
    case UL_D_OUTSCAN:
        mFuncName = "ulDOutScan";
        asyncVisible = true;
        scanVisible = true;
        setNumberVisible = true;
        goText = "Go";
        sampToolTip = "Samples per channel";
        startSample = "1000";
        mPlot = true;
        stackIndex = 2;
        break;
    case UL_D_INARRAY:
        mFuncName = "ulDInArray";
        asyncVisible = true;
        startSample = "10";
        sampToolTip = "Samples per channel";
        setNumberVisible = true;
        break;
    case UL_D_OUTARRAY:
        portsVisible = true;
        mFuncName = "ulDOutArray";
        goText = "Write";
        asyncVisible = true;
        setNumberVisible = true;
        break;
    default:
        break;
    }
    ui->stackedWidget->setVisible(!portsVisible);
    ui->stackedWidget->setCurrentIndex(stackIndex);
    ui->fraScan->setVisible(scanVisible);
    ui->fraAsync->setVisible(asyncVisible);
    ui->fraPorts->setVisible(portsVisible);
    ui->spnLowChan->setVisible(scanParamsVisible);
    ui->spnHighChan->setVisible(scanParamsVisible);
    ui->leNumSamples->setVisible(setNumberVisible);
    ui->leRate->setText(rateVal);
    ui->leNumSamples->setToolTip(sampToolTip);
    ui->leNumSamples->setText(startSample);
    ui->leBlockSize->setText(blockText);
    ui->cmdGo->setText(goText);
    ui->cmdStop->setVisible(stopVisible);
    ui->cmdStop->setText(stopText);
    parentWindow->adjustSize();
    updateControlDefaults(false);
    if (mPlot)
        showPlotWindow(mPlot);
    ui->cmdGo->setFocus();
    this->setWindowTitle(mFuncName + ": " + mDevName + QString(" [%1]").arg(mDaqDeviceHandle));
}

void DioSubWidget::updateText(QString infoText)
{
    ui->lblInfo->setText(infoText);
}

void DioSubWidget::setupPlot(QCustomPlot *dataPlot, int chanCount)
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

void DioSubWidget::showPlotWindow(bool showIt)
{
    QFrame::Shape frameShape;

    if ((mUtFunction == UL_D_OUTSCAN) | (mUtFunction == UL_D_INSCAN))
        frameShape = QFrame::Box;
    mPlot = showIt;
    int curIndex = 0;
    if (showIt) {
        curIndex = 2;
        frameShape = QFrame::NoFrame;
    }
    ui->stackedWidget->setFrameShape(frameShape);
    ui->stackedWidget->setCurrentIndex(curIndex);
}

void DioSubWidget::showDataGen()
{
    dataSelectDlg = new DataSelectDialog(this);
    if (mWaves.count()) {
        dataSelectDlg->setAmplitude(mAmplitude);
        dataSelectDlg->setNumCycles(mCycles);
        dataSelectDlg->setWaveType(mWaves);
        dataSelectDlg->setOffset(mOffset);
        dataSelectDlg->setResolution(mResolution);
        dataSelectDlg->setFsRange(mFSRange);
        dataSelectDlg->setDataScale(mDataScale);
        dataSelectDlg->setBipolar(mBipolar);
    }
    connect(dataSelectDlg, SIGNAL(accepted()), this, SLOT(dataDialogResponse()));
    dataSelectDlg->exec();
}

void DioSubWidget::dataDialogResponse()
{
    mWaves = dataSelectDlg->waveType();
    mCycles = dataSelectDlg->numCycles();
    mAmplitude = dataSelectDlg->amplitude();
    mOffset = dataSelectDlg->offset();
    mResolution = dataSelectDlg->resolution();
    mFSRange = dataSelectDlg->fsRange();
    mDataScale = dataSelectDlg->dataScale();
    mBipolar = dataSelectDlg->bipolar();

    disconnect(dataSelectDlg);
    delete dataSelectDlg;
    unsigned int numElements = mWaves.count();
    ui->spnLowChan->setValue(0);
    ui->spnHighChan->setValue(numElements - 1);
}

void DioSubWidget::initDeviceParams()
{
    DioInfoItem infoItem;
    DigitalPortType portType;
    long long infoValue, portValue, bitCount;
    QString nameOfFunc, funcArgs, argVals, funcStr;
    QTime t;
    QString sStartTime;

    qApp->processEvents();
    for (int i = 0; i < numBitCheckboxes; i++)
        chkBit[i]->setCheckState(Qt::PartiallyChecked);
    portList.clear();
    foreach (QCheckBox *chkPort, portCheckBoxes)
        chkPort->setChecked(false);

    validPorts.clear();
    portBits.clear();

    nameOfFunc = "ulDIOGetInfo";
    funcArgs = "(mDaqDeviceHandle, infoItem, 0, &infoValue)\n";
    infoItem = DIO_INFO_NUM_PORTS;
    sStartTime = t.currentTime().toString("hh:mm:ss.zzz") + "~";
    err = ulDIOGetInfo(mDaqDeviceHandle, infoItem, 0, &infoValue);
    argVals = QStringLiteral("(%1, %2, %3, %4)")
            .arg(mDaqDeviceHandle)
            .arg(infoItem)
            .arg(0)
            .arg(infoValue);

    funcStr = nameOfFunc + funcArgs + "Arg vals: " + argVals;
    if (err != ERR_NO_ERROR) {
        if (err == ERR_BAD_DEV_TYPE)
            return;
        mMainWindow->setError(err, sStartTime + funcStr);
    } else {
        mMainWindow->addFunction(sStartTime + funcStr);
        for (int portIndex = 0; portIndex < infoValue; portIndex++) {
            nameOfFunc = "ulDIOGetInfo";
            funcArgs = "(mDaqDeviceHandle, infoItem, portIndex, &portValue)\n";
            infoItem = DIO_INFO_PORT_TYPE;
            sStartTime = t.currentTime().toString("hh:mm:ss.zzz") + "~";
            err = ulDIOGetInfo(mDaqDeviceHandle, infoItem, portIndex, &portValue);
            argVals = QStringLiteral("(%1, %2, %3, %4)")
                    .arg(mDaqDeviceHandle)
                    .arg(infoItem)
                    .arg(portIndex)
                    .arg(portValue);

            funcStr = nameOfFunc + funcArgs + "Arg vals: " + argVals;
            if (err != ERR_NO_ERROR) {
                mMainWindow->setError(err, sStartTime + funcStr);
            } else {
                mMainWindow->addFunction(sStartTime + funcStr);
                portType = (DigitalPortType)portValue;
                validPorts.append(portType);
                nameOfFunc = "ulDIOGetInfo";
                funcArgs = "(mDaqDeviceHandle, infoItem, portIndex, &bitCount)\n";
                infoItem = DIO_INFO_NUM_BITS;
                sStartTime = t.currentTime().toString("hh:mm:ss.zzz") + "~";
                err = ulDIOGetInfo(mDaqDeviceHandle, infoItem, portIndex, &bitCount);
                argVals = QStringLiteral("(%1, %2, %3, %4)")
                        .arg(mDaqDeviceHandle)
                        .arg(infoItem)
                        .arg(portIndex)
                        .arg(bitCount);

                funcStr = nameOfFunc + funcArgs + "Arg vals: " + argVals;
                if (err != ERR_NO_ERROR) {
                    mMainWindow->setError(err, sStartTime + funcStr);
                } else {
                    mMainWindow->addFunction(sStartTime + funcStr);
                    portBits[portType] = bitCount;
                }
            }
        }
        mDioResolution = portBits.value(portType);
        updateControlDefaults(true);
    }
    portList = validPorts;
    ui->cmdGo->setFocus();
}

void DioSubWidget::updateControlDefaults(bool setAllValidPorts)
{
    DigitalPortType portType;

    foreach (portType, validPorts) {
        if (setAllValidPorts) {
            switch (portType) {
            case AUXPORT0:
                ui->chkAux0->setChecked(true);
                break;
            case AUXPORT1:
                ui->chkAux1->setChecked(true);
                break;
            case AUXPORT2:
                ui->chkAux2->setChecked(true);
                break;
            case FIRSTPORTA:
                ui->chk1stA->setChecked(true);
                break;
            case FIRSTPORTB:
                ui->chk1stB->setChecked(true);
                break;
            case FIRSTPORTCL:
                ui->chk1stCL->setChecked(true);
                break;
            case FIRSTPORTCH:
                ui->chk1stCH->setChecked(true);
                break;
            case SECONDPORTA:
                ui->chk2ndA->setChecked(true);
                break;
            case SECONDPORTB:
                ui->chk2ndB->setChecked(true);
                break;
            case SECONDPORTCL:
                ui->chk2ndCL->setChecked(true);
                break;
            case SECONDPORTCH:
                ui->chk2ndCH->setChecked(true);
                break;
            case THIRDPORTA:
                ui->chk3rdA->setChecked(true);
                break;
            case THIRDPORTB:
                ui->chk3rdB->setChecked(true);
                break;
            case THIRDPORTCL:
                ui->chk3rdCL->setChecked(true);
                break;
            case THIRDPORTCH:
                ui->chk3rdCH->setChecked(true);
                break;
            case FOURTHPORTA:
                ui->chk4thA->setChecked(true);
                break;
            case FOURTHPORTB:
                ui->chk4thB->setChecked(true);
                break;
            case FOURTHPORTCL:
                ui->chk4thCL->setChecked(true);
                break;
            case FOURTHPORTCH:
                ui->chk4thCH->setChecked(true);
                break;
            default:
                break;
            }
        }
        setDefaultBits(portType);
    }

}

void DioSubWidget::togglePortSelection()
{
    bool chkState, getState;

    chkState = false;
    foreach (QCheckBox *chkPort, portCheckBoxes) {
        if (!getState) {
            chkState = !chkPort->isChecked();
            getState = true;
        }
        chkPort->setChecked(chkState);
    }
}

void DioSubWidget::configureInputs()
{
    DigitalPortType portType;
    DigitalDirection direction;
    QString nameOfFunc, funcArgs, argVals, funcStr;
    QTime t;
    QString sStartTime;

    direction = DD_INPUT;
    nameOfFunc = "ulDConfigPort";
    funcArgs = "(mDaqDeviceHandle, portType, direction)\n";
    foreach (QCheckBox *chkPort, portCheckBoxes) {
        if (chkPort->isChecked()) {
            int portNum = chkPort->property("portNum").toInt();
            portType = (DigitalPortType)portNum;
            sStartTime = t.currentTime().toString("hh:mm:ss.zzz") + "~";
            err = ulDConfigPort(mDaqDeviceHandle, portType, direction);
            argVals = QStringLiteral("(%1, %2, %3)")
                    .arg(mDaqDeviceHandle)
                    .arg(portType)
                    .arg(direction);

            funcStr = nameOfFunc + funcArgs + "Arg vals: " + argVals;
            if (err != ERR_NO_ERROR) {
                mMainWindow->setError(err, sStartTime + funcStr);
            } else {
                mMainWindow->addFunction(sStartTime + funcStr);
                funcStr = nameOfFunc + argVals;
                ui->lblInfo->setText(funcStr);
            }
        }
    }
}

void DioSubWidget::configureOutputs()
{
    DigitalPortType portType;
    DigitalDirection direction;
    QString nameOfFunc, funcArgs, argVals, funcStr;
    QTime t;
    QString sStartTime;

    direction = DD_OUTPUT;
    nameOfFunc = "ulDConfigPort";
    funcArgs = "(mDaqDeviceHandle, portType, direction)\n";
    foreach (QCheckBox *chkPort, portCheckBoxes) {
        if (chkPort->isChecked()) {
            int portNum = chkPort->property("portNum").toInt();
            portType = (DigitalPortType)portNum;
            sStartTime = t.currentTime().toString("hh:mm:ss.zzz") + "~";
            err = ulDConfigPort(mDaqDeviceHandle, portType, direction);
            argVals = QStringLiteral("(%1, %2, %3)")
                    .arg(mDaqDeviceHandle)
                    .arg(portType)
                    .arg(direction);

            funcStr = nameOfFunc + funcArgs + "Arg vals: " + argVals;
            if (err != ERR_NO_ERROR) {
                mMainWindow->setError(err, sStartTime + funcStr);
            } else {
                mMainWindow->addFunction(sStartTime + funcStr);
                funcStr = nameOfFunc + argVals;
                ui->lblInfo->setText(funcStr);
            }
        }
    }
}

void DioSubWidget::onClickCmdGo()
{
    ChildWindow *parentWindow;
    parentWindow = qobject_cast<ChildWindow *>(this->parent());
    bool tmrIsEnabled;

    mPlotIndex = 0;
    mPlotCount = 0;
    if (mChanCount < 1)
        mChanCount = 1;
    mTotalSamples = ui->leNumSamples->text().toLong();

    //if (mUtFunction == UL_D_OUTSCAN)
    //    getDataValues();
    tmrIsEnabled = parentWindow->tmrEnabled();
    mUseTimer = tmrIsEnabled;
    ui->lblStatus->clear();

    if (mUtFunction == UL_D_CONFIG_PORT) {
        if (mCurGroup == FUNC_GROUP_DIN)
            runDInFunc();
        else
            runDOutFunc();
        return;
    }
    runSelectedFunc();
}

void DioSubWidget::getDataValues()
{
    QTextCursor curCursor;
    DataManager *genData = new DataManager();
    QVector<double> xValues;
    QVector<double> chanValues;
    QVector<QVector<double>> yChans;
    int defaultCycles = 1;
    int cycles;
    double offset, amplitude;
    double defaultOffset, defaultAmplitude;
    long curSample;
    DMgr::WaveType waveType;

    //set default wave parameters - half scale, sine wave
    //set default wave parameters - half scale, sine wave
    //mDioResolution = 32;
    defaultAmplitude = (qPow(2, mDioResolution) / 2) - 2;
    defaultOffset = defaultAmplitude / 2;

    //setup the buffer
    if (buffer) {
        delete[] buffer;
        buffer = NULL;
    }

    DMgr::WaveType defaultWave = DMgr::sineWave;
    int dataSetSize = mTotalSamples * mChanCount;
    mBufSize = dataSetSize;
    buffer = new unsigned long long[dataSetSize];

    //setup the plot data
    xValues.resize(mTotalSamples);
    chanValues.resize(mTotalSamples);
    yChans.resize(mChanCount);
    for (int chan=0; chan<mChanCount; chan++)
        yChans[chan].resize(mTotalSamples);

    //get the data - populated in plot vectors
    for (int chan = 0; chan < mChanCount; chan++) {
        offset = mOffset.value(chan, defaultOffset);
        amplitude = mAmplitude.value(chan, defaultAmplitude);
        waveType = (DMgr::WaveType)mWaves.value(chan, defaultWave);
        cycles = mCycles.value(chan, defaultCycles);
        genData->getChannelData(offset, amplitude, waveType,
                           cycles, chanValues, true);
        yChans[chan] = chanValues;
    }

    //transfer the data to the buffer from the plot vectors
    curSample = 0;
    for (long sample = 0; sample < mTotalSamples; sample++) {
        for (int chan = 0; chan < mChanCount; chan++) {
            buffer[curSample] = yChans[chan][sample];
            curSample +=1;
        }
    }

    if (mPlot) {
        int increment = 0;
        setupPlot(ui->plotDigitalData, mChanCount);
        for (int y = 0; y < mTotalSamples; y++) {
            xValues[y] = y;
            for (int chan = 0; chan < mChanCount; chan++) {
                yChans[chan][y] = buffer[chan + increment];
           }
           increment += mChanCount;
        }

        int curChanCount = 1;
        if (mPlotChan == -1)
            curChanCount = mChanCount;

        for (int plotNum=0; plotNum<curChanCount; plotNum++) {
            int plotData = mPlotChan;
            if (mPlotChan == -1)
                plotData = plotNum;

            ui->plotDigitalData->graph(plotNum)->setData(xValues, yChans[plotData]);
        }
        updatePlot();
    } else {
        int increment = 0;
        double curSample;
        curCursor = QTextCursor(ui->teShowValues->textCursor());
        ui->teShowValues->clear();
        for (int y = 0; y < mTotalSamples; y++) {
            ui->teShowValues->append
                    (QString("%1\t")
                     .arg(increment));
            for (int chan = 0; chan < mChanCount; chan++) {
                curSample = buffer[increment + chan];
                curCursor.movePosition(QTextCursor::End);
                ui->teShowValues->textCursor().setPosition(curCursor.position());
                ui->teShowValues->insertPlainText(QString("%1\t").arg(curSample));
            }
            increment +=mChanCount;
        }
    }
    delete genData;
}

void DioSubWidget::bitToggled(int bitNumber)
{
    DigitalPortType portType;
    int curBit;

    bool setValue = chkBit[bitNumber]->isChecked();
    unsigned int bitVal = 0;
    DigitalDirection direction = DD_INPUT;
    if (setValue) {
        bitVal = 1;
        direction = DD_OUTPUT;
    }
    mapGridToPortBit(bitNumber, portType, curBit);
    /*curBit = bitNumber;
    if (bitNumber < 16)
        tempPort = AUXPORT;
    else {
        tempPort = FIRSTPORTA;
       curBit = bitNumber - 16;
    }
    portType = parsePortFromBitIndex(tempPort, *pCurBit);*/
    switch (mUtFunction) {
    case UL_D_CONFIG_BIT:
        runDConfigBit(portType, curBit, direction);
        break;
    case UL_D_IN:
        break;
    case UL_D_OUT:
        runDOutFunc();
        break;
    case UL_D_BIT_IN:
        break;
    case UL_D_BIT_OUT:
        runDBitOutFunc(portType, curBit, bitVal);
        break;
    default:
        ui->lblInfo->setText(QString("Port %1, bit %2 clicked as bit value %3")
                               .arg(portType)
                               .arg(curBit)
                               .arg(bitVal));
        break;
    }
}

void DioSubWidget::runSelectedFunc()
{
    ChildWindow *parentWindow;
    QFont goFont = ui->cmdGo->font();
    bool makeBold, tmrIsEnabled, tmrIsRunning;
    bool isBipolar;
    bool showStop = false;
    int chanScale;
    double defaultRange, offset;

    parentWindow = qobject_cast<ChildWindow *>(this->parent());

    mChanCount = portList.count();
    if (mUtFunction == UL_D_OUTSCAN) {
        if (mChanCount) {
            for (int i = 0; i < mChanCount; i++) {
                int numWaves = mWaves.count() - 1;
                if (i > numWaves) {
                    //if data hasn't been defined set default data
                    chanScale = DMgr::counts;
                    defaultRange = (pow(2, mDioResolution)) - 2;
                    offset = defaultRange / 2;
                    isBipolar = false;
                    mWaves.insert(i, DMgr::sineWave);
                    mCycles.insert(i, 1);
                    mAmplitude.insert(i, defaultRange);
                    mOffset.insert(i, offset);
                    mDataScale.insert(i, chanScale);
                    mBipolar.insert(i, isBipolar);
                }
            }
        }
    }

    switch (mUtFunction) {
    case UL_D_CONFIG_PORT:
        //runDOutFunc();
        break;
    case UL_D_CONFIG_BIT:
        showStop = true;
        readSingleBit();
        break;
    case UL_D_IN:
        runDInFunc();
        break;
    case UL_D_OUT:
        runDOutFunc();
        break;
    case UL_D_BIT_IN:
        disableCheckboxInput(true);
        runDBitInFunc();
        break;
    case UL_D_BIT_OUT:
        runDBitInFunc();
        break;
    case UL_D_INSCAN:
        showStop = true;
        mTriggerType = parentWindow->triggerType();
        if (mTriggerType != TRIG_NONE) {
            mTrigChannel = parentWindow->trigChannel();
            mTrigLevel = parentWindow->trigLevel();
            mTrigVariance = parentWindow->trigVariance();
            mRetrigCount = parentWindow->retrigCount();
            qApp->processEvents();
            runSetTriggerFunc();
        }
        runDInScanFunc();
        break;
    case UL_D_OUTSCAN:
        showStop = true;
        getDataValues();
        mTriggerType = parentWindow->triggerType();
        if (mTriggerType != TRIG_NONE) {
            mTrigChannel = parentWindow->trigChannel();
            mTrigLevel = parentWindow->trigLevel();
            mTrigVariance = parentWindow->trigVariance();
            mRetrigCount = parentWindow->retrigCount();
            qApp->processEvents();
            runSetTriggerFunc();
        }
        runDOutScanFunc();
        break;
    case UL_D_INARRAY:
        runDInArray();
        break;
    case UL_D_OUTARRAY:
        runDOutArray();
        break;
    default:
        break;
    }

    tmrIsEnabled = parentWindow->tmrEnabled();
    tmrIsRunning = parentWindow->tmrRunning();
    if ((!tmrIsEnabled) & (!showStop)) {
        if (tmrIsRunning)
            parentWindow->setTmrRunning(false);
        showStop = false;
        mUseTimer = false;
        goFont.setBold(false);
        ui->cmdGo->setFont(goFont);
    } else {
        if (mUtFunction == UL_D_CONFIG_BIT) {
            showStop = true;
        } else if (mUseTimer) {
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
    ui->cmdStop->setVisible(tmrIsEnabled | showStop);
}

void DioSubWidget::runSetTriggerFunc()
{
    QString nameOfFunc, funcArgs, argVals, funcStr;
    QTime t;
    QString sStartTime;

    funcArgs = "(mDaqDeviceHandle, trigType, trigChan, level, variance, trigCount)\n";
    if (mUtFunction == UL_D_INSCAN) {
        nameOfFunc = "ulDInSetTrigger";
        sStartTime = t.currentTime().toString("hh:mm:ss.zzz") + "~";
        err = ulDInSetTrigger(mDaqDeviceHandle, mTriggerType, mTrigChannel, mTrigLevel, mTrigVariance, mRetrigCount);
    } else {
        nameOfFunc = "ulDOutSetTrigger";
        sStartTime = t.currentTime().toString("hh:mm:ss.zzz") + "~";
        err = ulDOutSetTrigger(mDaqDeviceHandle, mTriggerType, mTrigChannel, mTrigLevel, mTrigVariance, mRetrigCount);
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
    if (err != ERR_NO_ERROR) {
        mMainWindow->setError(err, sStartTime + funcStr);
    } else {
        mMainWindow->addFunction(sStartTime + funcStr);
    }
}

void DioSubWidget::runDConfigBit(DigitalPortType portType, int bitNum, DigitalDirection direction)
{
    QString nameOfFunc, funcArgs, argVals, funcStr;
    QTime t;
    QString sStartTime;

    nameOfFunc = "ulDConfigBit";
    funcArgs = "(mDaqDeviceHandle, portType, bitNum, direction)\n";
    sStartTime = t.currentTime().toString("hh:mm:ss.zzz") + "~";
    err = ulDConfigBit(mDaqDeviceHandle, portType, bitNum, direction);
    argVals = QStringLiteral("(%1, %2, %3, %4)")
            .arg(mDaqDeviceHandle)
            .arg(portType)
            .arg(bitNum)
            .arg(direction);
    ui->lblInfo->setText(nameOfFunc + argVals + QString(" [Error = %1]").arg(err));

    funcStr = nameOfFunc + funcArgs + "Arg vals: " + argVals;
    if (err != ERR_NO_ERROR) {
        mMainWindow->setError(err, sStartTime + funcStr);
        return;
    } else {
        mMainWindow->addFunction(sStartTime + funcStr);
    }
}

long long DioSubWidget::getIOConfigMask(int portIndex)
{
    QTime t;
    QString sStartTime;
    DioConfigItem configItem;
    QString nameOfFunc, funcArgs, argVals, funcStr;
    long long configValue;

    configItem = DIO_CFG_PORT_DIRECTION_MASK;
    nameOfFunc = "ulDIOGetConfig";
    funcArgs = "(mDaqDeviceHandle, configItem, portIndex, &configValue)\n";
    sStartTime = t.currentTime().toString("hh:mm:ss.zzz") + "~";
    err = ulDIOGetConfig(mDaqDeviceHandle, configItem, portIndex, &configValue);
    argVals = QStringLiteral("(%1, %2, %3, %4)")
            .arg(mDaqDeviceHandle)
            .arg(configItem)
            .arg(portIndex)
            .arg(configValue);

    funcStr = nameOfFunc + funcArgs + "Arg vals: " + argVals;
    if (err != ERR_NO_ERROR) {
        mMainWindow->setError(err, sStartTime + funcStr);
        return 0;
    } else {
        mMainWindow->addFunction(sStartTime + funcStr);
    }
    return configValue;
}

void DioSubWidget::runEventSetup()
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
            if (mCurGroup == FUNC_GROUP_DIN) {
                if (!(mEventType & DE_ON_END_OF_INPUT_SCAN))
                    enableList = DE_ON_END_OF_INPUT_SCAN;
            }
        } else {
            disableList = DE_ON_END_OF_INPUT_SCAN;
        }
    }
    if (getEndOutScanEvent(mDaqDeviceHandle, eventValue)) {
        //if this exists, enable or disable according to eventValue
        if (eventValue) {
            disableAll = false;
            if (mCurGroup == FUNC_GROUP_DOUT) {
                if (!(mEventType & DE_ON_END_OF_OUTPUT_SCAN))
                    enableList = DE_ON_END_OF_OUTPUT_SCAN;
            }
        } else {
            disableList = DE_ON_END_OF_OUTPUT_SCAN;
        }
    }
    if (getDataAvailableEvent(mDaqDeviceHandle, eventValue)) {
        //if this exists, enable or disable according to eventValue
        if (eventValue) {
            disableAll = false;
            if (mCurGroup == FUNC_GROUP_DIN) {
                if (!(mEventType & DE_ON_DATA_AVAILABLE)) {
                    reEnable = false;
                    enableList |= DE_ON_DATA_AVAILABLE;
                }
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
            if (mCurGroup == FUNC_GROUP_DIN) {
                if (!(mEventType & DE_ON_INPUT_SCAN_ERROR))
                    enableList |= DE_ON_INPUT_SCAN_ERROR;
            }
        } else {
            disableList |= DE_ON_INPUT_SCAN_ERROR;
        }
    }
    if (getOutScanErrorEvent(mDaqDeviceHandle, eventValue)) {
        //if this exists, enable or disable according to eventValue
        if (eventValue) {
            disableAll = false;
            if (mCurGroup == FUNC_GROUP_DOUT) {
                if (!(mEventType & DE_ON_OUTPUT_SCAN_ERROR))
                    enableList |= DE_ON_OUTPUT_SCAN_ERROR;
            }
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

void DioSubWidget::runEventEnable(DaqEventType eventType, unsigned long long eventParam)
{
    QString nameOfFunc, funcArgs, argVals, funcStr;
    QTime t;
    QString sStartTime;

    DaqEventCallback eventCallbackFunction = &DioSubWidget::eventCallback;

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

void DioSubWidget::runEventDisable(DaqEventType eventType)
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

void DioSubWidget::eventCallback(DaqDeviceHandle devHandle, DaqEventType eventType, unsigned long long eventData, void* userData)
{
    if (devHandle == 0)
        eventType = DE_ON_INPUT_SCAN_ERROR;
    DioSubWidget *dioChild = (DioSubWidget *)userData;
    dioChild->callbackHandler(eventType, eventData);
}

void DioSubWidget::callbackHandler(DaqEventType eventType, unsigned long long eventData)
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
        ui->lblStatus->setText(QString("Scan end event: %1 samples acquired").arg(eventData));
        finalBlockSize = eventData - (mPlotCount / mChanCount);
        if (finalBlockSize > 1)
            emit dataReady(mPlotCount, mPlotIndex, finalBlockSize);
        break;
    case DE_ON_OUTPUT_SCAN_ERROR:
        ui->cmdStop->setEnabled(false);
        ui->lblStatus->setText(QString("Error event: %1").arg(eventData));
        break;
    case DE_ON_END_OF_OUTPUT_SCAN:
        ui->cmdStop->setEnabled(false);
        ui->lblStatus->setText(QString("Scan end event: %1 samples generated").arg(eventData));
        break;
    default:
        break;
    }
    mPlotCount = eventData * mChanCount;
    mPlotIndex = mPlotCount % (mSamplesPerChan * mChanCount);
}

void DioSubWidget::showData(unsigned long long curTotalCount, unsigned long long curIndex, int blockSize)
{
    if (mPlot) {
        plotScan(curTotalCount, curIndex, blockSize);
    } else {
        printData(curTotalCount, curIndex, blockSize);
    }
}

void DioSubWidget::runDInFunc()
{
    QString dataText, str;
    QString nameOfFunc, funcArgs, argVals, funcStr;
    unsigned long long data;
    int numDigPorts, numSamples, curIndex;
    QTime t;
    QString sStartTime;
    QList<DigitalPortType> portsSelected;
    DigitalPortType portType;

    portsSelected.clear();
    foreach (QCheckBox *chkPort, portCheckBoxes) {
        if (chkPort->isChecked()) {
            int portNum = chkPort->property("portNum").toInt();
            portType = (DigitalPortType)portNum;
            portsSelected.append(portType);
        }
    }
    numDigPorts = portsSelected.count();
    if (mUtFunction == UL_D_CONFIG_PORT) {
        numSamples = 1;
    } else {
        numSamples = ui->leNumSamples->text().toInt();
    }
    QVector<unsigned long long> dataVal(numDigPorts);
    QVector<QVector<unsigned long long>>  dataArray(numSamples);
    for (int sample=0; sample<numSamples; sample++)
        dataArray[sample].resize(numDigPorts);

    nameOfFunc = "ulDIn";
    funcArgs = "(mDaqDeviceHandle, portType, data)\n";
    for (int sampleNum = 0; sampleNum < numSamples; sampleNum++) {
        curIndex = 0;
        foreach (DigitalPortType portType, portsSelected) {
            sStartTime = t.currentTime().toString("hh:mm:ss.zzz") + "~";
            data = 110011;
            err = ulDIn(mDaqDeviceHandle, portType, &data);
            dataVal[curIndex] = data;
            dataArray[sampleNum][curIndex] = dataVal[curIndex];
            curIndex++;
            argVals = QStringLiteral("(%1, %2, %3)")
                    .arg(mDaqDeviceHandle)
                    .arg(portType)
                    .arg(data);
            ui->lblInfo->setText(nameOfFunc + argVals + QString(" [Error = %1]").arg(err));

            funcStr = nameOfFunc + funcArgs + "Arg vals: " + argVals;
            if (err != ERR_NO_ERROR) {
                mMainWindow->setError(err, sStartTime + funcStr);
                return;
            } else {
                mMainWindow->addFunction(sStartTime + funcStr);
            }
        }
    }

    QString temp;
    QListIterator<DigitalPortType> i(portsSelected);
    if (mUtFunction == UL_D_CONFIG_PORT) {
        QString portValues = "";
        curIndex = 0;
        while (i.hasNext()) {
            data = dataArray[0][curIndex];
            portValues.append(QString("%1, ").arg(data));
            i.next();
            curIndex++;
        }
        int loc = portValues.lastIndexOf(",");
        ui->leNumSamples->setText(portValues.left(loc));
    } else {
        ui->teShowValues->clear();
        dataText = "<style> th, td { padding-right: 10px;}</style><tr>";
        for (int sampleNum = 0; sampleNum < numSamples; sampleNum++) {
            curIndex = 0;
            dataText.append("<td>" + str.setNum(sampleNum * numDigPorts) + "</td>");
            foreach (int portNum, portList) {
                data = dataArray[sampleNum][curIndex];
                dataText.append("<td>P" + str.setNum(portNum) + ": " + temp.setNum(data) + "</td>");
                curIndex++;
            }
            dataText.append("</tr><tr>");
        }
        dataText.append("</td></tr>");
        ui->teShowValues->setHtml(dataText);
    }
}

void DioSubWidget::runDInArray()
{
    QString dataText, str;
    QString nameOfFunc, funcArgs, argVals, funcStr;
    unsigned long long *data;
    unsigned long long dataVal;
    int numDigPorts, numSamples, curIndex;
    int arraySize;
    QTime t;
    QString sStartTime;
    QList<DigitalPortType> portsSelected;
    DigitalPortType portTypeLow, portTypeHigh;

    portTypeLow = (DigitalPortType)0;
    portTypeHigh = (DigitalPortType)0;
    portsSelected.clear();
    foreach (QCheckBox *chkPort, portCheckBoxes) {
        if (chkPort->isChecked()) {
            int portNum = chkPort->property("portNum").toInt();
            if(portTypeLow == 0)
                portTypeLow = (DigitalPortType)portNum;
            portTypeHigh = (DigitalPortType)portNum;
        }
    }
    numDigPorts = (portTypeHigh - portTypeLow) + 1;
    if (mUtFunction == UL_D_CONFIG_PORT) {
        numSamples = 1;
    } else {
        numSamples = ui->leNumSamples->text().toInt();
    }
    arraySize = numSamples * numDigPorts;
    data = new unsigned long long[arraySize];
    //QVector<unsigned long long> dataVal(numDigPorts);
    QVector<QVector<unsigned long long>>  dataArray(numSamples);
    for (int sample=0; sample<numSamples; sample++)
        dataArray[sample].resize(numDigPorts);

    nameOfFunc = "ulDInArray";
    funcArgs = "(mDaqDeviceHandle, portTypeLow, portTypeHigh, data[])\n";
    for (int sampleNum = 0; sampleNum < numSamples; sampleNum++) {
        sStartTime = t.currentTime().toString("hh:mm:ss.zzz") + "~";
        err = ulDInArray(mDaqDeviceHandle, portTypeLow, portTypeHigh, data);
        for(curIndex = 0; curIndex < numDigPorts; curIndex++) {
            dataArray[sampleNum][curIndex] = data[curIndex];
        }
        argVals = QStringLiteral("(%1, %2, %3, %4)")
                .arg(mDaqDeviceHandle)
                .arg(portTypeLow)
                .arg(portTypeHigh)
                .arg(data[0]);
        ui->lblInfo->setText(nameOfFunc + argVals + QString(" [Error = %1]").arg(err));

        funcStr = nameOfFunc + funcArgs + "Arg vals: " + argVals;
        if (err != ERR_NO_ERROR) {
            mMainWindow->setError(err, sStartTime + funcStr);
            return;
        } else {
            mMainWindow->addFunction(sStartTime + funcStr);
        }
    }

    QString temp;
    QListIterator<DigitalPortType> i(portsSelected);
    if (mUtFunction == UL_D_CONFIG_PORT) {
        QString portValues = "";
        curIndex = 0;
        while (i.hasNext()) {
            dataVal = dataArray[0][curIndex];
            portValues.append(QString("%1, ").arg(dataVal));
            i.next();
            curIndex++;
        }
        int loc = portValues.lastIndexOf(",");
        ui->leNumSamples->setText(portValues.left(loc));
    } else {
        ui->teShowValues->clear();
        dataText = "<style> th, td { padding-right: 10px;}</style><tr>";
        for (int sampleNum = 0; sampleNum < numSamples; sampleNum++) {
            dataText.append("<td>" + str.setNum(sampleNum * numDigPorts) + "</td>");
            for(curIndex = 0; curIndex < numDigPorts; curIndex++) {
                dataVal = dataArray[sampleNum][curIndex];
                dataText.append("<td>P" + str.setNum(portTypeLow + curIndex) + ": " + temp.setNum(dataVal) + "</td>");
            }
            dataText.append("</tr><tr>");
        }
        dataText.append("</td></tr>");
        ui->teShowValues->setHtml(dataText);
    }
}

void DioSubWidget::runDBitInFunc()
{
    QString nameOfFunc, funcArgs, argVals, funcStr;
    DigitalPortType portType;
    unsigned int bitValue;
    int bitNum, gridIndex;
    QTime t;
    QString sStartTime;

    disableCheckboxInput(true);
    nameOfFunc = "ulDBitIn";
    funcArgs = "(mDaqDeviceHandle, portType, bitNum, data)\n";
    for (int bitCheckBox = 0; bitCheckBox < numBitCheckboxes; bitCheckBox++) {
        if (chkBit[bitCheckBox]->checkState() != Qt::PartiallyChecked) {
            gridIndex = bitCheckBox;
            mapGridToPortBit(gridIndex, portType, bitNum);
            sStartTime = t.currentTime().toString("hh:mm:ss.zzz") + "~";
            err = ulDBitIn(mDaqDeviceHandle, portType, bitNum, &bitValue);
            argVals = QStringLiteral("(%1, %2, %3, %4)")
                    .arg(mDaqDeviceHandle)
                    .arg(portType)
                    .arg(bitNum)
                    .arg(bitValue);
            ui->lblInfo->setText(nameOfFunc + argVals + QString(" [Error = %1]").arg(err));

            funcStr = nameOfFunc + funcArgs + "Arg vals: " + argVals;
            if (err != ERR_NO_ERROR) {
                mMainWindow->setError(err, sStartTime + funcStr);
                disableCheckboxInput(false);
                return;
            } else {
                mMainWindow->addFunction(sStartTime + funcStr);
            }
            bool setCheckValue = (bitValue != 0);
            chkBit[bitCheckBox]->setChecked(setCheckValue);
        }
    }
    if (!ui->cmdStop->isVisible())
        disableCheckboxInput(false);
}

void DioSubWidget::runDOutFunc()
{
    DigitalPortType portType;
    QString nameOfFunc, funcArgs, argVals, funcStr;
    unsigned long long data;
    QTime t;
    QString sStartTime;

    data = ui->leNumSamples->text().toULongLong();
    nameOfFunc = "ulDOut";
    funcArgs = "(mDaqDeviceHandle, portType, data)\n";

    foreach (QCheckBox *chkPort, portCheckBoxes) {
        if (chkPort->isChecked()) {
            int portNum = chkPort->property("portNum").toInt();
            portType = (DigitalPortType)portNum;
            sStartTime = t.currentTime().toString("hh:mm:ss.zzz") + "~";
            err = ulDOut(mDaqDeviceHandle, portType, data);
            argVals = QStringLiteral("(%1, %2, %3)")
                    .arg(mDaqDeviceHandle)
                    .arg(portType)
                    .arg(data);

            funcStr = nameOfFunc + funcArgs + "Arg vals: " + argVals;
            if (err != ERR_NO_ERROR) {
                mMainWindow->setError(err, sStartTime + funcStr);
            } else {
                mMainWindow->addFunction(sStartTime + funcStr);
                funcStr = nameOfFunc + argVals;
                ui->lblInfo->setText(funcStr);
            }
        }
    }
}

void DioSubWidget::runDOutArray()
{
    DigitalPortType portTypeLow, portTypeHigh;
    QString nameOfFunc, funcArgs, argVals, funcStr;
    unsigned long long *data;
    unsigned long long dataVal;
    int numDigPorts;
    QTime t;
    QString sStartTime;

    nameOfFunc = "ulDOutArray";
    funcArgs = "(mDaqDeviceHandle, portTypeLow, portTypeHigh, data)\n";

    portTypeLow = (DigitalPortType)0;
    portTypeHigh = (DigitalPortType)0;
    foreach (QCheckBox *chkPort, portCheckBoxes) {
        if (chkPort->isChecked()) {
            int portNum = chkPort->property("portNum").toInt();
            if(portTypeLow == 0)
                portTypeLow = (DigitalPortType)portNum;
            portTypeHigh = (DigitalPortType)portNum;
        }
    }
    numDigPorts = (portTypeHigh - portTypeLow) + 1;
    dataVal = ui->leNumSamples->text().toULongLong();
    data = new unsigned long long[numDigPorts];
    for (int port = 0; port < numDigPorts; port++)
        data[port] = dataVal;

    sStartTime = t.currentTime().toString("hh:mm:ss.zzz") + "~";
    err = ulDOutArray(mDaqDeviceHandle, portTypeLow, portTypeHigh, data);
    argVals = QStringLiteral("(%1, %2, %3, %4)")
            .arg(mDaqDeviceHandle)
            .arg(portTypeLow)
            .arg(portTypeHigh)
            .arg(data[0]);

    funcStr = nameOfFunc + funcArgs + "Arg vals: " + argVals;
    if (err != ERR_NO_ERROR) {
        mMainWindow->setError(err, sStartTime + funcStr);
    } else {
        mMainWindow->addFunction(sStartTime + funcStr);
        funcStr = nameOfFunc + argVals;
        ui->lblInfo->setText(funcStr);
    }
}

void DioSubWidget::runDBitOutFunc(DigitalPortType portType, int bitNum, unsigned int bitValue)
{
    QString nameOfFunc, funcArgs, argVals, funcStr;
    QTime t;
    QString sStartTime;

    nameOfFunc = "ulDBitOut";
    funcArgs = "(mDaqDeviceHandle, portType, bitNum, bitValue)\n";
    sStartTime = t.currentTime().toString("hh:mm:ss.zzz") + "~";
    err = ulDBitOut(mDaqDeviceHandle, portType, bitNum, bitValue);
    argVals = QStringLiteral("(%1, %2, %3, %4)")
            .arg(mDaqDeviceHandle)
            .arg(portType)
            .arg(bitNum)
            .arg(bitValue);
    ui->lblInfo->setText(nameOfFunc + argVals + QString(" [Error = %1]").arg(err));

    funcStr = nameOfFunc + funcArgs + "Arg vals: " + argVals;
    if (err != ERR_NO_ERROR) {
        mMainWindow->setError(err, sStartTime + funcStr);
        return;
    } else {
        mMainWindow->addFunction(sStartTime + funcStr);
    }
}

void DioSubWidget::runDInScanFunc()
{
    DigitalPortType lowPort, highPort;
    DigitalPortType portType;
    int samplesPerPort, numDigPorts;
    QString nameOfFunc, funcArgs, argVals, funcStr;
    QList<int> portsSelected;
    QTime t;
    QString sStartTime;

    portsSelected.clear();
    foreach (QCheckBox *chkPort, portCheckBoxes) {
        if (chkPort->isChecked()) {
            int portNum = chkPort->property("portNum").toInt();
            portType = (DigitalPortType)portNum;
            portsSelected.append(portType);
        }
    }
    numDigPorts = portsSelected.count();
    if (numDigPorts < 1) {
        return;
    } else {
        lowPort = (DigitalPortType)portsSelected[0];
        highPort = (DigitalPortType)portsSelected[numDigPorts - 1];
    }

    mChanCount = numDigPorts;
    samplesPerPort = ui->leNumSamples->text().toLong();
    double rate = ui->leRate->text().toDouble();
    mBlockSize = ui->leBlockSize->text().toLongLong();

    mSamplesPerChan = samplesPerPort;
    long long bufSize = numDigPorts * samplesPerPort;
    if(mPlot)
        setupPlot(ui->plotDigitalData, mChanCount);

    //only default flag (0) exists currently
    mDInScanFlag = DINSCAN_FF_DEFAULT;
    if (mStopOnStart) {
        nameOfFunc = "ulDInScanStop";
        funcArgs = "(mDaqDeviceHandle)\n";
        sStartTime = t.currentTime().toString("hh:mm:ss.zzz") + "~";
        err = ulDInScanStop(mDaqDeviceHandle);
        argVals = QStringLiteral("(%1)")
                .arg(mDaqDeviceHandle);

        funcStr = nameOfFunc + funcArgs + "Arg vals: " + argVals;
        if (err != ERR_NO_ERROR) {
            mMainWindow->setError(err, sStartTime + funcStr);
            return;
        } else {
            mMainWindow->addFunction(sStartTime + funcStr);
        }
    }

    if (buffer) {
        delete[] buffer;
        buffer = NULL;
    }

    mBufSize = bufSize;
    buffer = new unsigned long long[mBufSize];
    memset(buffer, 0, mBufSize * sizeof(*buffer));
    nameOfFunc = "ulDInScan";
    funcArgs = "(mDaqDeviceHandle, lowPort, highPort, samplesPerPort, "
               "&rate, mScanOptions, mDInScanFlag, buffer)\n";

    sStartTime = t.currentTime().toString("hh:mm:ss.zzz") + "~";
    err = ulDInScan(mDaqDeviceHandle, lowPort, highPort,
                        mTotalSamples, &rate, mScanOptions, mDInScanFlag, buffer);
    argVals = QString("(%1, %2, %3, %4, %5, %6, %7, buffer)")
            .arg(mDaqDeviceHandle)
            .arg(lowPort)
            .arg(highPort)
            .arg(mTotalSamples)
            .arg(rate)
            .arg(mScanOptions)
            .arg(mDInScanFlag);
    ui->lblInfo->setText(nameOfFunc + argVals + QString(" [Error = %1]").arg(err));

    funcStr = nameOfFunc + funcArgs + "Arg vals: " + argVals;
    if (err != ERR_NO_ERROR) {
        mStatusTimerEnabled = false;
        mMainWindow->setError(err, sStartTime + funcStr);
    } else {
        mMainWindow->addFunction(sStartTime + funcStr);
        ui->lblRateReturned->setText(QString("%1").arg(rate, 1, 'f', 4, '0'));
        if (mUseWait) {
            qApp->processEvents();
            WaitType waitType = WAIT_UNTIL_DONE;
            long long waitParam = 0;
            nameOfFunc = "ulDInScanWait";
            funcArgs = "(mDaqDeviceHandle, waitType, waitParam, timeout)\n";
            sStartTime = t.currentTime().toString("hh:mm:ss.zzz") + "~";
            err = ulDInScanWait(mDaqDeviceHandle, waitType, waitParam, mWaitTime);
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

void DioSubWidget::runDOutScanFunc()
{
    DigitalPortType lowPort, highPort;
    DigitalPortType portType;
    int numDigPorts;
    QString nameOfFunc, funcArgs, argVals, funcStr;
    QList<int> portsSelected;
    QTime t;
    QString sStartTime;

    portsSelected.clear();
    foreach (QCheckBox *chkPort, portCheckBoxes) {
        if (chkPort->isChecked()) {
            int portNum = chkPort->property("portNum").toInt();
            portType = (DigitalPortType)portNum;
            portsSelected.append(portType);
        }
    }
    numDigPorts = portsSelected.count();
    if (numDigPorts < 1) {
        return;
    } else {
        lowPort = (DigitalPortType)portsSelected[0];
        highPort = (DigitalPortType)portsSelected[numDigPorts - 1];
    }

    mDOutScanFlag = DOUTSCAN_FF_DEFAULT;    //currently only flag
    double rate = ui->leRate->text().toDouble();
    mBlockSize = ui->leBlockSize->text().toLongLong();
    nameOfFunc = "ulDOutScan";
    funcArgs = "(mDaqDeviceHandle, lowPort, highPort, samplesPerPort, "
               "&rate, mScanOptions, mDInScanFlag, buffer)\n";

    sStartTime = t.currentTime().toString("hh:mm:ss.zzz") + "~";
    err = ulDOutScan(mDaqDeviceHandle, lowPort, highPort,
                        mTotalSamples, &rate, mScanOptions, mDOutScanFlag, buffer);
    argVals = QString("(%1, %2, %3, %4, %5, %6, %7, buffer)")
            .arg(mDaqDeviceHandle)
            .arg(lowPort)
            .arg(highPort)
            .arg(mTotalSamples)
            .arg(rate)
            .arg(mScanOptions)
            .arg(mDInScanFlag);
    ui->lblInfo->setText(nameOfFunc + argVals + QString(" [Error = %1]").arg(err));

    funcStr = nameOfFunc + funcArgs + "Arg vals: " + argVals;
    if (err != ERR_NO_ERROR) {
        mStatusTimerEnabled = false;
        mMainWindow->setError(err, sStartTime + funcStr);
    } else {
        mMainWindow->addFunction(sStartTime + funcStr);
        ui->lblRateReturned->setText(QString("%1").arg(rate, 1, 'f', 4, '0'));
        if (mUseWait) {
            qApp->processEvents();
            WaitType waitType = WAIT_UNTIL_DONE;
            long long waitParam = 0;
            nameOfFunc = "ulDOutScanWait";
            funcArgs = "(mDaqDeviceHandle, waitType, waitParam, timeout)\n";
            sStartTime = t.currentTime().toString("hh:mm:ss.zzz") + "~";
            err = ulDOutScanWait(mDaqDeviceHandle, waitType, waitParam, mWaitTime);
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
        }
    }
}

void DioSubWidget::checkStatus()
{
    ScanStatus status;
    struct TransferStatus xferStatus;
    QString nameOfFunc, funcArgs, argVals, funcStr;
    unsigned long long currentScanCount;
    unsigned long long currentTotalCount;
    long long currentIndex;
    long long curBlock;
    QTime t;
    QString sStartTime;

    funcArgs = "(mDaqDeviceHandle, &status, {&currentScanCount, &currentTotalCount, &currentIndex})\n";
    if (mUtFunction == UL_D_INSCAN) {
        nameOfFunc = "ulDInScanStatus";
        sStartTime = t.currentTime().toString("hh:mm:ss.zzz") + "~";
        err = ulDInScanStatus(mDaqDeviceHandle, &status, &xferStatus);
    } else {
        nameOfFunc = "ulDOutScanStatus";
        sStartTime = t.currentTime().toString("hh:mm:ss.zzz") + "~";
        err = ulDOutScanStatus(mDaqDeviceHandle, &status, &xferStatus);
    }
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
    if ((err != ERR_NO_ERROR) && mStatusTimerEnabled) {
        mStatusTimerEnabled = false;
        mMainWindow->setError(err, sStartTime + funcStr);
    } else if ((status == SS_RUNNING) && mStatusTimerEnabled) {
        curBlock = (((currentTotalCount - mPlotCount) + 1)-(mBlockSize * mChanCount));
        ui->lblStatus->setText(QString("RUNNING Total = %1 (%2 perChan), Index: %3")
                               .arg(currentTotalCount)
                               .arg(currentScanCount)
                               .arg(currentIndex));
        if (mUtFunction == UL_D_INSCAN) {
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
        }
    } else {
        stopScan(currentScanCount, currentTotalCount, currentIndex);
    }
    ui->cmdStop->setEnabled(mGoTmrIsRunning | mRunning);
}

UlError DioSubWidget::stopScan(long long perChan, long long curCount, long long curIndex)
{
    QString nameOfFunc, funcArgs, argVals, funcStr;
    long long finalBlockSize;
    QTime t;
    QString sStartTime;

    funcArgs = "(mDaqDeviceHandle)";
    tmrCheckStatus->stop();
    if (mUtFunction == UL_D_INSCAN) {
        nameOfFunc = "ulDInScanStop";
        sStartTime = t.currentTime().toString("hh:mm:ss.zzz") + "~";
        err = ulDInScanStop(mDaqDeviceHandle);
    } else {
        nameOfFunc = "ulDOutScanStop";
        sStartTime = t.currentTime().toString("hh:mm:ss.zzz") + "~";
        err = ulDOutScanStop(mDaqDeviceHandle);
    }
    argVals = QStringLiteral("(%1)")
            .arg(mDaqDeviceHandle);
    ui->lblInfo->setText(nameOfFunc + argVals + QString(" [Error = %1]").arg(err));

    funcStr = nameOfFunc + funcArgs + "Arg vals: " + argVals;
    if (err != ERR_NO_ERROR) {
        mStatusTimerEnabled = false;
        mMainWindow->setError(err, sStartTime + funcStr);
        return err;
    } else {
        mMainWindow->addFunction(sStartTime + funcStr);
    }
    ui->lblStatus->setText(QStringLiteral("IDLE Count = %1 (%2 perChan), index: %3")
                           .arg(curCount)
                           .arg(perChan)
                           .arg(curIndex));
    mRunning = false;
    if (mUtFunction == UL_D_INSCAN) {
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
    }
    mPlotCount = curCount;
    mFinalCount = curCount;
    return err;
}

void DioSubWidget::onClickCmdStop()
{
    QTime t;
    QString sStartTime;

    QFont goFont = ui->cmdGo->font();
    goFont.setBold(false);
    ui->cmdGo->setFont(goFont);
    mUseTimer = false;
    if ((mUtFunction == UL_D_OUTSCAN)
        | (mUtFunction == UL_D_INSCAN)) {
        ScanStatus status;
        struct TransferStatus xferStatus;
        unsigned long long currentTotalCount;
        unsigned long long currentScanCount;
        long long currentIndex;

        mStatusTimerEnabled = false;
        QString nameOfFunc, funcArgs, argVals, funcStr;

        funcArgs = "(mDaqDeviceHandle, &status, {&currentScanCount, &currentTotalCount, &currentIndex})\n";
        if (mUtFunction == UL_D_INSCAN) {
            nameOfFunc = "ulDInScanStatus";
            sStartTime = t.currentTime().toString("hh:mm:ss.zzz") + "~";
            err = ulDInScanStatus(mDaqDeviceHandle, &status, &xferStatus);
        } else {
            nameOfFunc = "ulDOutScanStatus";
            sStartTime = t.currentTime().toString("hh:mm:ss.zzz") + "~";
            err = ulDOutScanStatus(mDaqDeviceHandle, &status, &xferStatus);
        }
        currentIndex = xferStatus.currentIndex;
        currentScanCount = xferStatus.currentScanCount;
        currentTotalCount = xferStatus.currentTotalCount;
        argVals = QStringLiteral("(%1, %2, {%3, %4, %5})")
                .arg(mDaqDeviceHandle)
                .arg(status)
                .arg(currentScanCount)
                .arg(currentTotalCount)
                .arg(currentIndex);
        mRunning = (status == SS_RUNNING);
        ui->lblStatus->setText(QStringLiteral("Idle at %1, (%2 perPort), %3")
                               .arg(currentTotalCount)
                               .arg(currentScanCount)
                               .arg(currentIndex));

        funcStr = nameOfFunc + funcArgs + "Arg vals: " + argVals;
        if (err != ERR_NO_ERROR) {
            mMainWindow->setError(err, sStartTime + funcStr);
        } else {
            // to do - add ability to enable / disable logging this
            //mMainWindow->addFunction(sStartTime + funcStr);
        }
        err = stopScan(currentScanCount, currentTotalCount, currentIndex);
        ui->cmdStop->setEnabled(mGoTmrIsRunning | mRunning);
    }
    if (mCancelDScan) {
        mCancelDScan = false;
        ui->cmdStop->setEnabled(false);
    }
    ui->cmdStop->setEnabled(false);
    disableCheckboxInput(false);
}

void DioSubWidget::swStopScan()
{
    //this comes from escape key on main through ChildWindow
    onClickCmdStop();
}

void DioSubWidget::plotScan(unsigned long long currentCount, long long currentIndex, int blockSize)
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
        if (mPlotChan == -1)
            plotData = plotNum;

        if (mPlotList[plotNum])
            ui->plotDigitalData->graph(plotNum)->setData(xValues, yChans[plotData]);
    }
    updatePlot();
}

void DioSubWidget::printData(unsigned long long currentCount, long long currentIndex, int blockSize)
{
    QTextCursor curCursor;
    QString dataText, str;
    double curSample;
    int curScan, samplesToPrint, sampleLimit;
    int sampleNum = 0;
    int increment = 0;
    long long samplePerChanel = mChanCount * ui->leNumSamples->text().toLongLong();;

    curCursor = QTextCursor(ui->teShowValues->textCursor());
    ui->teShowValues->clear();
    dataText = "<style> th, td { padding-right: 10px;}</style><tr>";
    sampleLimit = mRunning? 100 : 5000;
    samplesToPrint = blockSize < sampleLimit? blockSize : sampleLimit;
    for (int y = 0; y < blockSize; y++) {
        curScan = currentIndex + increment;
        if (curScan >= samplePerChanel) {
            currentIndex = 0;
            curScan = 0;
            sampleNum = 0;
        }
        //ui->teShowValues->append
        //        (QString("%1: \t")
        //         .arg(currentCount + increment));
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

void DioSubWidget::stopGoTimer()
{
    QString nameOfFunc, funcArgs, argVals, funcStr;
    QString bitPort, bit, port;
    int bitNum;
    DigitalPortType portNum;
    static unsigned int bitValue = 0;
    QTime t;
    QString sStartTime;

    mUseTimer = false;
    if ((mUtFunction == UL_D_OUTSCAN)
        | (mUtFunction == UL_D_INSCAN)) {
        onClickCmdStop();
        return;
    }
    if (mUtFunction == UL_D_CONFIG_BIT) {
        bitValue = !bitValue;
        bitPort = ui->leNumSamples->text();
        int here = bitPort.indexOf(",");
        bit = bitPort.left(here);
        bitNum = bit.toInt();
        port = bitPort.mid(here + 1);
        portNum = DigitalPortType(port.toInt());

        nameOfFunc = "ulDBitOut";
        funcArgs = "(mDaqDeviceHandle, portType, bitNum, data)\n";
        sStartTime = t.currentTime().toString("hh:mm:ss.zzz") + "~";
        err = ulDBitOut(mDaqDeviceHandle, portNum, bitNum, bitValue);
        argVals = QStringLiteral("(%1, %2, %3, %4)")
                .arg(mDaqDeviceHandle)
                .arg(portNum)
                .arg(bitNum)
                .arg(bitValue);
        ui->lblInfo->setText(nameOfFunc + argVals + QString(" [Error = %1]").arg(err));

        funcStr = nameOfFunc + funcArgs + "Arg vals: " + argVals;
        if (err != ERR_NO_ERROR) {
            mMainWindow->setError(err, sStartTime + funcStr);
        } else {
            mMainWindow->addFunction(sStartTime + funcStr);
        }
    }
}

void DioSubWidget::readSingleBit()
{
    QString nameOfFunc, funcArgs, argVals, funcStr;
    QString bitPort, bit, port;
    int bitNum;
    unsigned int bitValue;
    DigitalPortType portNum;
    QTime t;
    QString sStartTime;

    bitPort = ui->leNumSamples->text();
    int here = bitPort.indexOf(",");
    bit = bitPort.left(here);
    bitNum = bit.toInt();
    port = bitPort.mid(here + 1);
    portNum = DigitalPortType(port.toInt());

    nameOfFunc = "ulDBitIn";
    funcArgs = "(mDaqDeviceHandle, portType, bitNum, data)\n";
    sStartTime = t.currentTime().toString("hh:mm:ss.zzz") + "~";
    err = ulDBitIn(mDaqDeviceHandle, portNum, bitNum, &bitValue);
    argVals = QStringLiteral("(%1, %2, %3, %4)")
            .arg(mDaqDeviceHandle)
            .arg(portNum)
            .arg(bitNum)
            .arg(bitValue);
    ui->lblInfo->setText(nameOfFunc + argVals + QString(" [Error = %1]").arg(err));

    funcStr = nameOfFunc + funcArgs + "Arg vals: " + argVals;
    if (err != ERR_NO_ERROR) {
        mMainWindow->setError(err, sStartTime + funcStr);
    } else {
        mMainWindow->addFunction(sStartTime + funcStr);
    }
}

void DioSubWidget::updatePlot()
{
    bool autoScale;
    double rangeBuf;
    double rangeUpper, rangeLower;
    int ctlIndex;

    for (int plotNum=0; plotNum<mChanCount; plotNum++) {
        ctlIndex = plotNum % 8;
        rbPlotSel[ctlIndex]->setVisible(true);
        if (!mPlotList[ctlIndex])
            ui->plotDigitalData->graph(plotNum)->clearData();
    }
    for (int i = mChanCount; i<8; i++)
        rbPlotSel[i]->setVisible(false);
    autoScale = ui->rbAutoScale->isChecked();
    if (autoScale)
        ui->plotDigitalData->rescaleAxes();
    else {
        //add manual scale based on counts
        rangeBuf = pow(2, mDioResolution) / 10;
        rangeUpper = pow(2, mDioResolution);
        rangeLower = 0;

        ui->plotDigitalData->xAxis->rescale();
        ui->plotDigitalData->yAxis->setRangeLower(rangeLower - rangeBuf);
        ui->plotDigitalData->yAxis->setRangeUpper(rangeUpper + rangeBuf);
    }
    ui->plotDigitalData->replot();
}

void DioSubWidget::replot()
{
    updatePlot();
}

void DioSubWidget::plotSelect()
{
    for (int i = 0; i<8; i++)
        mPlotList[i] = rbPlotSel[i]->isChecked();

    if (!mRunning)
        plotScan(mPlotCount, mPlotIndex, mBlockSize);
}

void DioSubWidget::createBitBoxes()
{
    int pos, offset, portIndex, bitIndex;
    QString portBit, str;
    //int numBits = 64;
    //QCheckBox *chkBit[numBits]={};
    QSignalMapper *mapBits = new QSignalMapper(ui->stackedWidget->widget(1));
    QGridLayout *bitLayout = new QGridLayout(ui->stackedWidget->widget(1));
    QColor blue( 100, 180, 255 );

    connect(mapBits, SIGNAL(mapped(int)), this, SLOT(bitToggled(int)));
    bitLayout->setSpacing(1);
    bitLayout->setMargin(4);
    bitLayout->setSizeConstraint(QLayout::SetFixedSize);
    bitLayout->setColumnMinimumWidth(3, 40);

    //spacing between auxport and firsport set by col width

    pos = 0;
    offset = 0;
    portIndex = 0;
    bitIndex = 0;
    QList<QString> portNames = QList<QString>()
            << "1A" << "1B" << "1CL" << "1CH"
            << "2A" << "2B" << "2CL" << "2CH"
            << "3A" << "3B" << "3CL" << "3CH"
            << "4A" << "4B" << "4CL" << "4CH"
            << "3A" << "3B" << "3CL" << "3CH"
            << "4A" << "4B" << "4CL" << "4CH";
    for (int i=0; i<numBitCheckboxes; i++) {
        chkBit[i] = new QCheckBox(ui->stackedWidget->widget(1));
        //QPalette p = chkBit[i]->palette();
        //p.setColor( QPalette::Active, QPalette::Base, blue );
        //chkBit[i]->setPalette(p);
        chkBit[i]->setTristate(true);
        chkBit[i]->setCheckState(Qt::PartiallyChecked);
        mapBits->setMapping(chkBit[i], i);
        connect(chkBit[i], SIGNAL(clicked(bool)), mapBits, SLOT(map()));
        chkBit[i]->setMaximumHeight(16);
        chkBit[i]->setMaximumWidth(20);
        chkBit[i]->setSizePolicy(QSizePolicy::Minimum, QSizePolicy::Fixed);
        if (i < 16)
            portBit = QString("Aux1 bit %1").arg(i);
        else if (i < 32)
            portBit = QString("Aux2 bit %1").arg(i - 16);
        else {
            if  (i - 32) {
                if (!((i - 32 - offset) % 20)) {
                    pos += 4;
                    bitIndex = 0;
                    portIndex += 1;
                } else if (!((i - 32) % 24)) {
                    pos += 4;
                    offset += 4;
                    bitIndex = 0;
                    portIndex += 1;
                }
            }
            if ((i > 32) && (bitIndex > 0))
                if (!(i % 8)) {
                    bitIndex = 0;
                    portIndex += 1;
                }
            portBit = portNames[portIndex] + str.setNum(bitIndex);
            bitIndex += 1;
        }
        chkBit[i]->setToolTip(portBit);
        bitLayout->addWidget(chkBit[i], pos % 8, pos / 8);
        pos++;
    }
    ui->stackedWidget->widget(1)->setLayout(bitLayout);
}

void DioSubWidget::disableCheckboxInput(bool disableMouse)
{
    if (disableMouse)
        for (int i = 0; i < numBitCheckboxes; i++) {
            chkBit[i]->setAttribute(Qt::WA_TransparentForMouseEvents);
            //chkBit[i]->setAttribute(Qt::NoFocus);
        }
    else
        for (int i = 0; i < numBitCheckboxes; i++) {
            chkBit[i]->setAttribute(Qt::WA_TransparentForMouseEvents, false);
            //chkBit[i]->setAttribute(Qt::StrongFocus);
        }
}

void DioSubWidget::setDefaultBits(DigitalPortType portType)
{
    int portBit, portIndex;
    long long configMask;
    bool checkTrue;

    portIndex = validPorts.indexOf(portType);
    configMask = getIOConfigMask(portIndex);
    int numBits = portBits.value(portType);
    int portOffset = getGridOffsetFromPort(portType);
    checkTrue = false;
    for (int i = 0; i < numBits; i++) {
        portBit = i + portOffset;
        //if (portType < FIRSTPORTA)
        //    portBit = i;
        bool triState = true;
        if ((mUtFunction == UL_D_CONFIG_BIT) |
            (mUtFunction == UL_D_BIT_OUT)) {
            triState = false;
            if (mUtFunction == UL_D_CONFIG_BIT)
                checkTrue = ((int)pow(2, i) & (int)configMask);
        }
        chkBit[portBit]->setTristate(triState);
        chkBit[portBit]->setChecked(checkTrue);
    }
}

DigitalPortType DioSubWidget::parsePortFromBitIndex(DigitalPortType tempPort, int &bitNum)
{
    int testBit;
    int totalBits = 0;
    DigitalPortType portType;

    testBit = bitNum;
    foreach (portType, validPorts) {
        int bitsInPort = portBits.value(portType);
        if ((bitsInPort + totalBits) >= testBit) {
            bitNum = testBit - totalBits;
            return portType;
        }
        totalBits += bitsInPort;
    }
    return tempPort;
}

int DioSubWidget::getGridOffsetFromPort(DigitalPortType portType)
{
    switch (portType) {
    case AUXPORT0:
        return 0;
        break;
    case AUXPORT1:
        return 16;
    case AUXPORT2:
        //not implemented - used by net prods
        return 16;
    case FIRSTPORTA:
        return 32;
    case FIRSTPORTB:
        return 40;
    case FIRSTPORTCL:
        return 48;
    case FIRSTPORTCH:
        return 52;
    case SECONDPORTA:
        return 56;
    case SECONDPORTB:
        return 64;
    case SECONDPORTCL:
        return 72;
    case SECONDPORTCH:
        return 76;
    case THIRDPORTA:
        return 80;
    case THIRDPORTB:
        return 88;
    case THIRDPORTCL:
        return 96;
    case THIRDPORTCH:
        return 100;
    case FOURTHPORTA:
        return 104;
    case FOURTHPORTB:
        return 112;
    case FOURTHPORTCL:
        return 120;
    case FOURTHPORTCH:
        return 124;
    default:
        return 0;
        break;
    }
}

void DioSubWidget::mapGridToPortBit(int gridIndex, DigitalPortType &portType, int &bitInPort)
{
    if (gridIndex < 16) {
        portType = AUXPORT0;
        bitInPort = gridIndex;
    } else if (gridIndex < 32) {
        portType = AUXPORT1;
        bitInPort = gridIndex - 16;
    } else if (gridIndex < 40) {
        portType = FIRSTPORTA;
        bitInPort = gridIndex - 32;
    } else if (gridIndex < 48) {
        portType = FIRSTPORTB;
        bitInPort = gridIndex - 40;
    } else if (gridIndex < 52) {
        portType = FIRSTPORTCL;
        bitInPort = gridIndex - 48;
    } else if (gridIndex < 56) {
        portType = FIRSTPORTC;
        if (validPorts.contains(FIRSTPORTCH)) {
            portType = FIRSTPORTCH;
            bitInPort = gridIndex - 52;
        } else {
            bitInPort = gridIndex - 48;
        }
    } else if (gridIndex < 64) {
        portType = SECONDPORTA;
        bitInPort = gridIndex - 56;
    } else if (gridIndex < 72) {
        portType = SECONDPORTB;
        bitInPort = gridIndex - 64;
    } else if (gridIndex < 76) {
        portType = SECONDPORTCL;
        bitInPort = gridIndex - 72;
    } else if (gridIndex < 80) {
        portType = SECONDPORTCH;
        bitInPort = gridIndex - 76;
    } else if (gridIndex < 88) {
        portType = THIRDPORTA;
        bitInPort = gridIndex - 80;
    } else if (gridIndex < 96) {
        portType = THIRDPORTB;
        bitInPort = gridIndex - 88;
    } else if (gridIndex < 100) {
        portType = THIRDPORTCL;
        bitInPort = gridIndex - 96;
    } else if (gridIndex < 104) {
        portType = THIRDPORTCH;
        bitInPort = gridIndex - 100;
    } else if (gridIndex < 112) {
        portType = FOURTHPORTA;
        bitInPort = gridIndex - 104;
    } else if (gridIndex < 120) {
        portType = FOURTHPORTB;
        bitInPort = gridIndex - 112;
    } else if (gridIndex < 124) {
        portType = FOURTHPORTCL;
        bitInPort = gridIndex - 120;
    } else if (gridIndex < 128) {
        portType = FOURTHPORTCH;
        bitInPort = gridIndex - 124;
    }
}

//stub slots for childwindow signals

void DioSubWidget::showQueueConfig()
{
    return;
}
