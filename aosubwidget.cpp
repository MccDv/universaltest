#include "aosubwidget.h"
#include "ui_aosubwidget.h"
#include <qfont.h>
#include "childwindow.h"
#include "unitest.h"
#include "uldaq.h"
#include "testutilities.h"
#include "../Test/errordialog.h"

AoSubWidget::AoSubWidget(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::AoSubWidget)
{
    ui->setupUi(this);

    tmrCheckStatus = new QTimer(this);
    mUseGetStatus = true;
    mUseWait = false;
    ui->lblRateReturned->setFont(QFont ("Courier", 8));
    ui->lblRateReturned->setStyleSheet("QLabel { background-color : white; color : blue; }" );
    ui->teShowValues->setStyleSheet("QTextEdit { background-color : white; color : blue; }" );
    ui->lblStatus->setStyleSheet("QLabel { color : blue; }" );
    ui->lblInfo->setStyleSheet("QLabel { color : blue; }" );
    connect(tmrCheckStatus, SIGNAL(timeout()), this, SLOT(checkStatus()));
    connect(ui->hSldAoutVal, SIGNAL(valueChanged(int)), this, SLOT(updateValueBox()));
    connect(ui->cmdGo, SIGNAL(clicked(bool)), this, SLOT(onClickCmdGo()));
    connect(ui->cmdStop, SIGNAL(clicked(bool)), this, SLOT(onClickCmdStop()));
    connect(ui->plotOutData->xAxis, SIGNAL(rangeChanged(QCPRange)),
            ui->plotOutData->xAxis2, SLOT(setRange(QCPRange)));
    connect(ui->plotOutData->yAxis, SIGNAL(rangeChanged(QCPRange)),
            ui->plotOutData->yAxis2, SLOT(setRange(QCPRange)));
    //connect(this, SIGNAL(dataReady(unsigned long long, unsigned long long)), this,
    //        SLOT(showData(unsigned long long, unsigned long long)));
    //connect(ui->actionAuto_scale, SIGNAL(triggered()), this, SLOT(setPlotAutoScale()));

    ui->cmdStop->setVisible(false);
    buffer = NULL;
    mPlot = false;
    mPlotChan = -1;
    mEventType = DE_NONE;
    mCancelAOut = false;
    setupPlot(ui->plotOutData, 1);
    ui->plotOutData->replot();
    mMainWindow = getMainWindow();
}

AoSubWidget::~AoSubWidget()
{
    delete ui;
    if (buffer) {
        delete[] buffer;
        buffer = NULL;
    }
}

MainWindow *AoSubWidget::getMainWindow()
{
    foreach (QWidget *w, QApplication::topLevelWidgets())
        if (QMainWindow* mainWin = qobject_cast<QMainWindow*>(w))
            return qobject_cast<MainWindow *>(mainWin);
    return nullptr;
}

void AoSubWidget::updateParameters()
{
    ChildWindow *parentWindow;
    bool showStop = false;
    QString funcName;

    //following used to determine device changes
    //between initDevice calls
    QString devID = mDevUID;
    DaqOutScanFlag daqOutFlag = DaqOutScanFlag(mDaqoFlags & 1);
    AOutFlag aoFlag = AOutFlag(mAoFlags & 1);

    parentWindow = qobject_cast<ChildWindow *>(this->parent());
    mDaqDeviceHandle = parentWindow->devHandle();
    mDevName = parentWindow->devName();
    mDevUID = parentWindow->devUID();
    if (!(devID == mDevUID))
        initDeviceParams();

    mAoFlags = (AOutFlag)parentWindow->aoFlags();
    mDaqoFlags = (DaqOutScanFlag)parentWindow->daqOutFlag();

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
    if ((mUtFunction == UL_AOUT_SCAN) | (mUtFunction == UL_DAQ_OUTSCAN)) {
        funcName = "UlDaqOutScan(";
        if (mUtFunction == UL_AOUT_SCAN)
                funcName = "UlAoutScan(";
        ui->lblStatus->setText(funcName + optNames + ")");
        showStop = true;
    }
    mUseTimer = parentWindow->tmrEnabled();
    mStopOnStart = parentWindow->tmrStopOnStart();
    mGoTmrIsRunning = parentWindow->tmrRunning();
    if (mUseTimer | showStop) {
        ui->cmdStop->setVisible(true);
        ui->cmdStop->setEnabled(mGoTmrIsRunning);
    }

    initDeviceParams();
    if (!(daqOutFlag == (mDaqoFlags & 1)))
            mWaves.clear();
    if (!(aoFlag == (mAoFlags & 1)))
        mWaves.clear();
    if (mCancelAOut)
        ui->cmdStop->setVisible(false);
    mCancelAOut = false;
    ui->leBlockSize->setEnabled(mUseGetStatus);
    this->setWindowTitle(mFuncName + ": " + mDevName + QString(" [%1]").arg(mDaqDeviceHandle));
}

void AoSubWidget::groupChanged(int newGroup)
{
    mCurGroup = newGroup;
}

void AoSubWidget::setUiForGroup()
{
    return;
}

void AoSubWidget::functionChanged(int utFunction)
{
    mUtFunction = utFunction;
    this->setUiForFunction();
}

void AoSubWidget::setUiForFunction()
{
    QFrame::Shape frameShape;
    bool scanVisible, showChanSelect;

    mChanList.clear();
    mRangeList.clear();
    mChanTypeList.clear();
    frameShape = QFrame::NoFrame;
    showChanSelect = true;
    scanVisible = false;
    switch (mUtFunction) {
    case UL_AOUT:
        mFuncName = "ulAOut";
        break;
    case UL_AOUT_SCAN:
    case UL_DAQ_OUTSCAN:
        mFuncName = "ulAOutScan";
        if (mUtFunction == UL_DAQ_OUTSCAN) {
            mFuncName = "ulDaqOutScan";
            showChanSelect = false;
        }
        scanVisible = true;
        ui->leRate->setText("10");
        ui->leNumSamples->setText("100");
        ui->leBlockSize->setText("20");
        //set default values for chanDescriptor
        mChanList.insert(0, 0);
        mRangeList.insert(0, mRange);
        mChanTypeList.insert(0, DAQO_ANALOG);
        if (!mPlot)
            frameShape = QFrame::Box;
        break;
    default:
        break;
    }
    ui->spnHighChan->setEnabled(showChanSelect);
    ui->spnLowChan->setEnabled(showChanSelect);
    ui->fraScan->setVisible(scanVisible);
    int numInStack = ui->stackedWidget->count();
    for (int i = 0; i < numInStack; i++) {
        QObjectList stackKids = ui->stackedWidget->widget(i)->children();
        foreach (QObject *kid, stackKids) {
            QSlider *sldr = qobject_cast<QSlider *>(kid);
            QLineEdit *ledt = qobject_cast<QLineEdit *>(kid);
            QTextEdit *txEdt = qobject_cast<QTextEdit *>(kid);
            if (sldr)
                sldr->setVisible(!scanVisible);
            if (ledt)
                ledt->setVisible(!scanVisible);
            if (txEdt)
                txEdt->setVisible(scanVisible);
        }
    }
    ui->stackedWidget->setFrameShape(frameShape);
    this->setWindowTitle(mFuncName + ": " + mDevName + QString(" [%1]").arg(mDaqDeviceHandle));
}

void AoSubWidget::setupPlot(QCustomPlot *dataPlot, int chanCount)
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

void AoSubWidget::showPlotWindow(bool showIt)
{
    QFrame::Shape frameShape;

    if (mUtFunction == UL_AOUT_SCAN)
        frameShape = QFrame::Box;
    mPlot = showIt;
    int curIndex = 0;
    if (showIt) {
        curIndex = 1;
        frameShape = QFrame::NoFrame;
    }
    ui->stackedWidget->setFrameShape(frameShape);
    ui->stackedWidget->setCurrentIndex(curIndex);
}

void AoSubWidget::showQueueConfig()
{
    queueSetup = new QueueDialog(this, FUNC_GROUP_AOUT);
    if (mChanList.count()) {
        queueSetup->setChanList(mChanList);
        queueSetup->setChanOutTypeList(mChanTypeList);
        //queueSetup->setModeList(mModeList);
        queueSetup->setRangeList(mRangeList);
        queueSetup->setNumQueueElements(mChanList.count());
    }
    connect(queueSetup, SIGNAL(accepted()), this, SLOT(queueDialogResponse()));
    queueSetup->exec();
}

void AoSubWidget::queueDialogResponse()
{
    mChanList = queueSetup->chanList();
    mChanTypeList = queueSetup->chanOutTypeList();
    //mModeList = queueSetup->modeList();
    mRangeList = queueSetup->rangeList();

    disconnect(queueSetup);
    delete queueSetup;
    //if (mUtFunction == UL_AINSCAN)
    //    setupQueue();
    unsigned int numElements = mChanList.count();
    if (mUtFunction == UL_DAQ_OUTSCAN) {
        if (numElements) {
            ui->spnLowChan->setValue(0);
            ui->spnHighChan->setValue(numElements - 1);
            ui->spnHighChan->setEnabled(false);
            ui->spnLowChan->setEnabled(false);
            mChanCount = numElements;
        }
    } else {
        ui->spnHighChan->setEnabled(true);
        ui->spnLowChan->setEnabled(true);
    }
}

void AoSubWidget::showDataGen()
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

void AoSubWidget::dataDialogResponse()
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
    //unsigned int numElements = mWaves.count();
    //ui->spnLowChan->setValue(0);
    //ui->spnHighChan->setValue(numElements - 1);
}

void AoSubWidget::initDeviceParams()
{
    QString nameOfFunc, funcArgs, argVals, funcStr;
    ChildWindow *parentWindow;
    DigitalPortType portType;
    AoInfoItem infoItem;
    DioInfoItem dioInfoItem;
    long long infoValue, numRanges;
    long long portValue, bitCount;
    parentWindow = qobject_cast<ChildWindow *>(this->parent());
    QTime t;
    QString sStartTime;

    QList<Range> rangeList;
    validPorts.clear();
    portBits.clear();

    nameOfFunc = "ulAOGetInfo";
    funcArgs = "(mDaqDeviceHandle, infoItem, 0, &infoValue)\n";

    Range curRange = mRange;
    infoItem = AO_INFO_RESOLUTION;
    sStartTime = t.currentTime().toString("hh:mm:ss.zzz") + "~";
    err = ulAOGetInfo(mDaqDeviceHandle, infoItem, 0, &infoValue);
    argVals = QString("(%1, %2, %3, %4)")
            .arg(mDaqDeviceHandle)
            .arg(infoItem)
            .arg(0)
            .arg(infoValue);

    funcStr = nameOfFunc + funcArgs + "Arg vals: " + argVals;
    if (err == ERR_NO_ERROR) {
        mMainWindow->addFunction(sStartTime + funcStr);
        mAoResolution = infoValue;
        ui->hSldAoutVal->setMaximum(pow(2, mAoResolution));
    } else {
        mMainWindow->setError(err, sStartTime + funcStr);
        return;
    }

    nameOfFunc = "ulAOGetInfo";
    funcArgs = "(mDaqDeviceHandle, infoItem, 0, &infoValue)\n";

    infoItem = AO_INFO_NUM_RANGES;
    sStartTime = t.currentTime().toString("hh:mm:ss.zzz") + "~";
    err = ulAOGetInfo(mDaqDeviceHandle, infoItem, 0, &infoValue);
    argVals = QString("(%1, %2, %3, %4)")
            .arg(mDaqDeviceHandle)
            .arg(infoItem)
            .arg(0)
            .arg(infoValue);

    funcStr = nameOfFunc + funcArgs + "Arg vals: " + argVals;
    if (err == ERR_NO_ERROR) {
        mMainWindow->addFunction(sStartTime + funcStr);
        numRanges = infoValue;
        infoItem = AO_INFO_RANGE;
        for (int rangeItem = 0; rangeItem < numRanges; rangeItem++) {
            //get a list of ranges
            nameOfFunc = "ulAOGetInfo";
            funcArgs = "(mDaqDeviceHandle, infoItem, 0, &infoValue)\n";

            sStartTime = t.currentTime().toString("hh:mm:ss.zzz") + "~";
            err = ulAOGetInfo(mDaqDeviceHandle, infoItem, 0, &infoValue);
            argVals = QString("(%1, %2, %3, %4)")
                    .arg(mDaqDeviceHandle)
                    .arg(infoItem)
                    .arg(0)
                    .arg(infoValue);

            funcStr = nameOfFunc + funcArgs + "Arg vals: " + argVals;
            if (err == ERR_NO_ERROR) {
                mMainWindow->addFunction(sStartTime + funcStr);
                rangeList.append((Range)infoValue);
            } else {
                mMainWindow->setError(err, sStartTime + funcStr);
                return;
            }
        }
        if (rangeList.count() == 1) {
            mFixedRange = true;
            mRange = rangeList.value(0);
        }
    } else {
        mMainWindow->setError(err, sStartTime + funcStr);
        return;
    }
    if (!(curRange == mRange)) {
        //report change to parent
        parentWindow->setCurRange(mRange);
    }

    mDefaultFSRange = getRangeVolts(mRange);

    dioInfoItem = DIO_INFO_NUM_PORTS;
    nameOfFunc = "ulDIOGetInfo";
    funcArgs = "(mDaqDeviceHandle, dioInfoItem, 0, &infoValue)\n";

    sStartTime = t.currentTime().toString("hh:mm:ss.zzz") + "~";
    err = ulDIOGetInfo(mDaqDeviceHandle, dioInfoItem, 0, &infoValue);
    argVals = QString("(%1, %2, %3, %4)")
            .arg(mDaqDeviceHandle)
            .arg(dioInfoItem)
            .arg(0)
            .arg(infoValue);

    funcStr = nameOfFunc + funcArgs + "Arg vals: " + argVals;
    if (err == ERR_NO_ERROR) {
        mMainWindow->addFunction(sStartTime + funcStr);
        for (int portIndex = 0; portIndex < infoValue; portIndex++) {

            funcArgs = "(mDaqDeviceHandle, dioInfoItem, 0, &portValue)\n";
            dioInfoItem = DIO_INFO_PORT_TYPE;
            sStartTime = t.currentTime().toString("hh:mm:ss.zzz") + "~";
            err = ulDIOGetInfo(mDaqDeviceHandle, dioInfoItem, 0, &portValue);
            argVals = QString("(%1, %2, %3, %4)")
                    .arg(mDaqDeviceHandle)
                    .arg(dioInfoItem)
                    .arg(0)
                    .arg(infoValue);

            funcStr = nameOfFunc + funcArgs + "Arg vals: " + argVals;
            if (err == ERR_NO_ERROR) {
                mMainWindow->addFunction(sStartTime + funcStr);
                portType = (DigitalPortType)portValue;
                validPorts.append(portType);
            } else {
                mMainWindow->setError(err, sStartTime + funcStr);
                return;
            }

            funcArgs = "(mDaqDeviceHandle, dioInfoItem, 0, &bitCount)\n";
            dioInfoItem = DIO_INFO_NUM_BITS;
            sStartTime = t.currentTime().toString("hh:mm:ss.zzz") + "~";
            err = ulDIOGetInfo(mDaqDeviceHandle, dioInfoItem, 0, &bitCount);
            argVals = QString("(%1, %2, %3, %4)")
                    .arg(mDaqDeviceHandle)
                    .arg(dioInfoItem)
                    .arg(0)
                    .arg(infoValue);

            funcStr = nameOfFunc + funcArgs + "Arg vals: " + argVals;
            if (err == ERR_NO_ERROR) {
                mMainWindow->addFunction(sStartTime + funcStr);
                portBits[portType] = bitCount;
            } else {
                mMainWindow->setError(err, sStartTime + funcStr);
                return;
            }
        }
        mDioResolution = portBits.value(portType);
    } else {
        mMainWindow->setError(err, sStartTime + funcStr);
        return;
    }
    ui->cmdGo->setFocus();
}

void AoSubWidget::updateValueBox()
{
    int sliderOut = ui->hSldAoutVal->value();
    switch (mAoFlags) {
    case AOUT_FF_NOSCALEDATA:
        ui->leAoutVal->setText(QString("%1").arg(sliderOut));
        break;
    default:
        double vOutVal = getVoltsFromCounts(mAoResolution, mRange, sliderOut);
        ui->leAoutVal->setText(QString("%1").arg(vOutVal));
        break;
    }
    runAOutFunc();
}

void AoSubWidget::onClickCmdGo()
{
    ChildWindow *parentWindow;
    parentWindow = qobject_cast<ChildWindow *>(this->parent());
    bool tmrIsEnabled;

    tmrIsEnabled = parentWindow->tmrEnabled();
    mUseTimer = tmrIsEnabled;
    //ui->teShowValues->clear();
    ui->lblRateReturned->clear();
    ui->lblStatus->clear();
    ui->lblInfo->clear();
    runSelectedFunc();
}

void AoSubWidget::getDataValues()
{
    QTextCursor curCursor;
    DataManager *genData = new DataManager();
    QVector<double> xValues;
    QVector<double> chanValues;
    QVector<QVector<double>> yChans;
    int defaultCycles = 1;
    int cycles;
    bool floatValue, chanFloat;
    bool truncate;
    double offset, amplitude;
    double defaultOffset, defaultAmplitude;
    long curSample;
    DMgr::WaveType waveType;
    int chanScale;
    QString dataText, str, val;

    //following sets up daqscan value ranges
    if (mUtFunction == UL_DAQ_OUTSCAN) {
        int daqList = mChanList.count();
        if (daqList) {
            for (int i = 0; i < daqList; i++) {
                //get the channel type
                DaqOutChanType outChan = mChanTypeList.value(i);
                if (outChan == DAQO_ANALOG) {
                    mResolution[i] = mAoResolution;
                    mBipolar[i] = (mRangeList.value(i) < 100);
                    if (mDaqoFlags & DAQOUTSCAN_FF_NOSCALEDATA) {
                        mDataScale[i] = DMgr::counts;
                        mFSRange[i] = qPow(2, mAoResolution);
                    } else {
                        mDataScale[i] = DMgr::volts;
                        mFSRange[i] = getRangeVolts(mRange);
                    }
                } else {
                    mResolution[i] = mDioResolution;
                    mBipolar[i] = false;
                    mDataScale[i] = DMgr::counts;
                    mFSRange[i] = pow(2, mDioResolution);
                }
                //chanDescriptors[i].channel = mChanList.value(i);
                //chanDescriptors[i].range = mRangeList.value(i);
                //chanDescriptors[i].type = mChanTypeList.value(i);
            }
        }
    }

    //delete existing buffer
    if (buffer) {
        delete[] buffer;
        buffer = NULL;
    }

    //setup the buffer
    DMgr::WaveType defaultWave = DMgr::sineWave;
    int dataSetSize = mSamplesPerChan * mChanCount;
    buffer = new double [dataSetSize];

    //setup the plot data
    xValues.resize(mSamplesPerChan);
    chanValues.resize(mSamplesPerChan);
    yChans.resize(mChanCount);
    for (int chan=0; chan<mChanCount; chan++)
        yChans[chan].resize(mSamplesPerChan);

    //these are overridden if using DaqOutScan
    defaultOffset = 0;
    if ((mAoFlags == AOUT_FF_NOSCALEDATA) | (mDaqoFlags == DAQOUTSCAN_FF_NOSCALEDATA)) {
        defaultOffset = qPow(2, mAoResolution) / 2;
        defaultAmplitude = qPow(2, mAoResolution) / 2;
        truncate = true;
    } else {
        double rangeVolts = getRangeVolts(mRange);
        truncate = false;
        if (!(mRange < 100))
            defaultOffset = rangeVolts / 2;
        defaultAmplitude = rangeVolts;
    }

    //get the data - populated in plot vectors
    for (int chan = 0; chan < mChanCount; chan++) {
        //set default wave parameters - half scale, sine wave
        if (mUtFunction == UL_DAQ_OUTSCAN) {
            chanScale = mDataScale.value(chan);
            chanFloat = ((chanScale == DMgr::volts));
            defaultOffset = 0;
            if (chanFloat) {
                if (mBipolar[chan]) {
                    defaultOffset = mFSRange[chan] / 2;
                defaultAmplitude = mFSRange[chan];
                }
            } else {
                if (mBipolar[chan]) {
                    defaultOffset = trunc(mFSRange[chan] / 2);
                defaultAmplitude = trunc(mFSRange[chan] / 2);
                }
            }
            truncate = !chanFloat;
        }
        offset = mOffset.value(chan, defaultOffset);
        amplitude = mAmplitude.value(chan, defaultAmplitude);
        waveType = (DMgr::WaveType)mWaves.value(chan, defaultWave);
        cycles = mCycles.value(chan, defaultCycles);
        genData->getChannelData(offset, amplitude, waveType,
                           cycles, chanValues, truncate);
        yChans[chan] = chanValues;
    }

    //transfer the data to the buffer from the plot vectors
    curSample = 0;
    for (long sample = 0; sample < mSamplesPerChan; sample++) {
        for (int chan = 0; chan < mChanCount; chan++) {
            buffer[curSample] = yChans[chan][sample];
            curSample +=1;
        }
    }

    if (mUtFunction == UL_DAQ_OUTSCAN)
        floatValue = (!(mDaqoFlags & DAQOUTSCAN_FF_NOSCALEDATA));
    if (mUtFunction == UL_AOUT_SCAN)
        floatValue = (!(mAoFlags & AOUTSCAN_FF_NOSCALEDATA));

    if (mPlot) {
        //int increment = 0;
        setupPlot(ui->plotOutData, mChanCount);
        for (int y = 0; y < mSamplesPerChan; y++) {
            xValues[y] = y;
            //for (int chan = 0; chan < mChanCount; chan++) {
            //    yChans[chan][y] = buffer[chan + increment];
           //}
            //increment += mChanCount;
        }

        int curChanCount = 1;
        if (mPlotChan == -1)
            curChanCount = mChanCount;

        for (int plotNum=0; plotNum<curChanCount; plotNum++) {
            int plotData = mPlotChan;
            if (mPlotChan == -1)
                plotData = plotNum;

            ui->plotOutData->graph(plotNum)->setData(xValues, yChans[plotData]);
        }
        updatePlot();
    } else {
        int increment = 0;
        int samplesToPrint;
        double curSample;
        //print only 500
        samplesToPrint = mSamplesPerChan < 500? mSamplesPerChan : 500;
        curCursor = QTextCursor(ui->teShowValues->textCursor());
        ui->teShowValues->clear();
        dataText = "<style> th, td { padding-right: 10px;}</style><tr>";
        for (int y = 0; y < samplesToPrint; y++) {
            //ui->teShowValues->append
            //        (QString("%1\t").arg(increment));
            dataText.append("<td>" + str.setNum(increment) + "</td>");
            for (int chan = 0; chan < mChanCount; chan++) {
                curSample = buffer[increment + chan];
                //curCursor.movePosition(QTextCursor::End);
                //ui->teShowValues->textCursor().setPosition(curCursor.position());
                if (floatValue)
                    val = QString("%1%2").arg((curSample < 0) ? "" : "+")
                            .arg(curSample, 2, 'f', 5, '0');
                    //ui->teShowValues->insertPlainText(QString("%1%2\t")
                    //                         .arg((curSample < 0) ? "" : "+")
                    //                         .arg(curSample, 2, 'f', 5, '0'));
                else
                    val = QString("%1").arg(curSample);
                    //ui->teShowValues->insertPlainText(
                    //            QString("%1  \t").arg(curSample));
                dataText.append("<td>" + val + "</td>");
            }
            dataText.append("</tr><tr>");
            increment +=mChanCount;
        }
        dataText.append("</td></tr>");
        ui->teShowValues->setHtml(dataText);
        if (samplesToPrint < mSamplesPerChan)
            ui->teShowValues->append("...");
    }
    delete genData;
}

void AoSubWidget::runSelectedFunc()
{
    ChildWindow *parentWindow;
    QFont goFont = ui->cmdGo->font();
    bool makeBold, tmrIsEnabled, tmrIsRunning;
    bool showStop, isBipolar, noScale;
    int chanScale;
    double defaultRange, offset;

    mLowChan = ui->spnLowChan->value();
    mHighChan = ui->spnHighChan->value();
    mBlockSize = ui->leBlockSize->text().toLongLong();
    mChanCount = (mHighChan - mLowChan) + 1;
    if(mChanCount < 1) mChanCount = 1;
    mSamplesPerChan = ui->leNumSamples->text().toLong();

    mPlotIndex = 0;
    mPlotCount = 0;
    parentWindow = qobject_cast<ChildWindow *>(this->parent());

    noScale = false;
    noScale = (mAoFlags == AOUT_FF_NOSCALEDATA);
    if (mUtFunction == UL_DAQ_OUTSCAN) {
        mChanCount = mChanList.count();
        if (mChanCount) {
            for (int i = 0; i < mChanCount; i++) {
                int numWaves = mWaves.count() - 1;
                if (i > numWaves) {
                    //if data hasn't been defined set default data
                    noScale = ((mChanTypeList.value(i) == DAQO_DIGITAL)
                               | (mDaqoFlags == DAQOUTSCAN_FF_NOSCALEDATA));
                    defaultRange = mDefaultFSRange;
                    isBipolar = (mDefaultFSRange < 100);
                    offset = 0;
                    if (noScale) {
                        chanScale = DMgr::counts;
                        defaultRange = pow(2, mAoResolution);
                        if (mChanTypeList.value(i) == DAQO_DIGITAL)
                            defaultRange = pow(2, mDioResolution);
                        offset = defaultRange / 2;
                        isBipolar = false;
                    }
                    mWaves.insert(i, DMgr::sineWave);
                    mCycles.insert(i, 1);
                    mAmplitude.insert(i, defaultRange);
                    mOffset.insert(i, offset);
                    mDataScale.insert(i, chanScale);
                    mBipolar.insert(i, isBipolar);
                }
                //if chans haven't been defined, set default channels
                chanDescriptors[i].channel = mChanList.value(i);
                chanDescriptors[i].range = mRangeList.value(i);
                chanDescriptors[i].type = mChanTypeList.value(i);
            }
        }
    } else {
        int numWaves = mWaves.count() - 1;
        defaultRange = mDefaultFSRange;
        isBipolar = (mDefaultFSRange < 100);
        offset = 0;
        if (noScale) {
            chanScale = DMgr::counts;
            defaultRange = pow(2, mAoResolution);
            offset = defaultRange / 2;
            isBipolar = false;
        }
        for (int i = 0; i < mChanCount; i++) {
            if (i > numWaves) {
                mWaves.insert(i, DMgr::sineWave);
                mCycles.insert(i, 1);
                mAmplitude.insert(i, defaultRange);
                mOffset.insert(i, offset);
                mDataScale.insert(i, chanScale);
                mBipolar.insert(i, isBipolar);
            }
        }
    }
    //int curRange = parentWindow->getCurrentRange();
    //mRange = (Range)curRange;
    switch (mUtFunction) {
    case UL_AOUT:
        getDataValues();
        runAOutFunc();
        break;
    case UL_AOUT_SCAN:
        getDataValues();
        if (!mTriggerType == TRIG_NONE)
            runSetTriggerFunc();
        runAOutScanFunc();
        break;
    case UL_DAQ_OUTSCAN:
        getDataValues();
        if (!mTriggerType == TRIG_NONE)
            runSetTriggerFunc();
        runDaqOutScanFunc();
        break;
    default:
        break;
    }

    tmrIsEnabled = parentWindow->tmrEnabled();
    tmrIsRunning = parentWindow->tmrRunning();

    showStop = ((mUtFunction == UL_AOUT_SCAN) | mCancelAOut | (mUtFunction == UL_DAQ_OUTSCAN));
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

void AoSubWidget::runSetTriggerFunc()
{
    QString nameOfFunc, funcArgs, argVals, funcStr;
    DaqInChanDescriptor trigChanDescriptor;
    QString trigChan;
    QTime t;
    QString sStartTime;

    //to do: make this user-settable
    trigChanDescriptor.type = DAQI_DIGITAL;
    trigChanDescriptor.channel = 1;
    if (mUtFunction == UL_DAQ_OUTSCAN) {
        nameOfFunc = "ulDaqOutSetTrigger";
        funcArgs = "(mDaqDeviceHandle, trigType, {trigChan, range}, level, variance, trigCount)\n";
        sStartTime = t.currentTime().toString("hh:mm:ss.zzz") + "~";
        err = ulDaqOutSetTrigger(mDaqDeviceHandle, mTriggerType, trigChanDescriptor, mTrigLevel, mTrigVariance, mRetrigCount);
        trigChan = (QString("{%1, %2}").arg(trigChanDescriptor.type)
                    .arg(trigChanDescriptor.channel)
                    .arg(trigChanDescriptor.range));
    } else {
        nameOfFunc = "ulAOutSetTrigger";
        funcArgs = "(mDaqDeviceHandle, trigType, trigChan, level, variance, trigCount)\n";
        sStartTime = t.currentTime().toString("hh:mm:ss.zzz") + "~";
        err = ulAOutSetTrigger(mDaqDeviceHandle, mTriggerType, mTrigChannel, mTrigLevel, mTrigVariance, mRetrigCount);
        trigChan = QString("%1").arg(mTrigChannel);
    }
    argVals = QStringLiteral("(%1, %2, %3, %4, %5, %6)")
            .arg(mDaqDeviceHandle)
            .arg(mTriggerType)
            .arg(trigChan)
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

void AoSubWidget::runEventSetup()
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

    if (!(eventsEnabled == DE_NONE))
        runEventEnable(eventsEnabled, eventParam);
    if (!(eventsDisabled == DE_NONE))
        runEventDisable(eventsDisabled);
}

void AoSubWidget::runEventEnable(DaqEventType eventType, unsigned long long eventParam)
{
    QString nameOfFunc, funcArgs, argVals, funcStr;
    QTime t;
    QString sStartTime;

    DaqEventCallback eventCallbackFunction = &AoSubWidget::eventCallback;

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

void AoSubWidget::runEventDisable(DaqEventType eventType)
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

void AoSubWidget::eventCallback(DaqDeviceHandle devHandle, DaqEventType eventType, unsigned long long eventData, void* userData)
{
    if (devHandle == 0)
        eventType = DE_ON_INPUT_SCAN_ERROR;
    AoSubWidget *aoChild = (AoSubWidget *)userData;
    aoChild->callbackHandler(eventType, eventData);
}

void AoSubWidget::callbackHandler(DaqEventType eventType, unsigned long long eventData)
{
    //called from eventCallback in testUtilities
    switch (eventType) {
    case DE_ON_DATA_AVAILABLE:
        ui->lblStatus->setText(QString("Data Available: %1, TotalCount: %2, Index: %3")
                               .arg(eventData)
                               .arg(mPlotCount)
                               .arg(mPlotIndex));
        //emit dataReady(eventData, mEventParams);
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

void AoSubWidget::runAOutFunc()
{
    int aOutChan;    //, aOutLastChan, numAoutChans;
    double data; //, curSample
    QString nameOfFunc, funcArgs, argVals, funcStr;
    QTime t;
    QString sStartTime;

    if (mCancelAOut) {
        ui->cmdStop->setVisible(true);
        return;
    }

    aOutChan = ui->spnLowChan->value();

    nameOfFunc = "ulAOut";
    funcArgs = "(mDaqDeviceHandle, aOutChan, range, flags, &data)\n";
    data = ui->leAoutVal->text().toDouble();

    sStartTime = t.currentTime().toString("hh:mm:ss.zzz") + "~";
    err = ulAOut(mDaqDeviceHandle, aOutChan, mRange, mAoFlags, data);
    argVals = QStringLiteral("(%1, %2, %3, %4, %5)")
            .arg(mDaqDeviceHandle)
            .arg(aOutChan)
            .arg(mRange)
            .arg(mAoFlags)
            .arg(data);
    ui->lblInfo->setText(nameOfFunc + argVals + QString(" [Error = %1]").arg(err));

    funcStr = nameOfFunc + funcArgs + "Arg vals: " + argVals;
    if (!err==ERR_NO_ERROR) {
        mMainWindow->setError(err, sStartTime + funcStr);
        mCancelAOut = true;
        ui->cmdStop->setVisible(true);
        return;
    } else {
        mMainWindow->addFunction(sStartTime + funcStr);
    }
}

void AoSubWidget::runAOutScanFunc()
{
    QString nameOfFunc, funcArgs, argVals, funcStr;
    double rate = ui->leRate->text().toDouble();
    QTime t;
    QString sStartTime;

    mFunctionFlag = (AOutScanFlag)mAoFlags;
    if (mStopOnStart) {
        nameOfFunc = "ulAOutScanStop";
        funcArgs = "(mDaqDeviceHandle)\n";
        sStartTime = t.currentTime().toString("hh:mm:ss.zzz") + "~";
        err = ulAOutScanStop(mDaqDeviceHandle);
        argVals = QString("(%1)")
                .arg(mDaqDeviceHandle);
        funcStr = nameOfFunc + funcArgs + "Arg vals: " + argVals;
        if (!err==ERR_NO_ERROR) {
            mMainWindow->setError(err, sStartTime + funcStr);
            return;
        } else {
            mMainWindow->addFunction(sStartTime + funcStr);
        }
    }

    nameOfFunc = "ulAOutScan";
    funcArgs = "(mDaqDeviceHandle, mLowChan, mHighChan, range, mSamplesPerChan, &rate, mScanOptions, flags, &data)\n";

    sStartTime = t.currentTime().toString("hh:mm:ss.zzz") + "~";
    err = ulAOutScan(mDaqDeviceHandle, mLowChan, mHighChan,
                        mRange, mSamplesPerChan, &rate, mScanOptions, mFunctionFlag, buffer);
    argVals = QStringLiteral("(%1, %2, %3, %4, %5, %6, %7, %8, %9)")
            .arg(mDaqDeviceHandle)
            .arg(mLowChan)
            .arg(mHighChan)
            .arg(mRange)
            .arg(mSamplesPerChan)
            .arg(rate)
            .arg(mScanOptions)
            .arg(mFunctionFlag)
            .arg(*buffer);
    ui->lblInfo->setText(nameOfFunc + argVals + QString(" [Error = %1]").arg(err));

    funcStr = nameOfFunc + funcArgs + "Arg vals: " + argVals;
    if (!err==ERR_NO_ERROR) {
        mMainWindow->setError(err, sStartTime + funcStr);
    } else {
        mMainWindow->addFunction(sStartTime + funcStr);
        ui->lblRateReturned->setText(QString("%1").arg(rate, 1, 'f', 4, '0'));
        if (mUseWait) {
            qApp->processEvents();
            WaitType waitType = WAIT_UNTIL_DONE;
            long long waitParam = 0;
            nameOfFunc = "ulAOutScanWait";
            funcArgs = "(mDaqDeviceHandle, waitType, waitParam, timeout)\n";
            sStartTime = t.currentTime().toString("hh:mm:ss.zzz") + "~";
            err = ulAOutScanWait(mDaqDeviceHandle, waitType, waitParam, mWaitTime);
            argVals = QString("(%1, %2, %3, %4)")
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
        }
    }
}

void AoSubWidget::runDaqOutScanFunc()
{
    QString nameOfFunc, funcArgs, argVals, funcStr;
    double rate = ui->leRate->text().toDouble();
    QTime t;
    QString sStartTime;

    if (mStopOnStart) {
        nameOfFunc = "ulDaqOutScan";
        funcArgs = "(mDaqDeviceHandle)";
        sStartTime = t.currentTime().toString("hh:mm:ss.zzz") + "~";
        err = ulDOutScanStop(mDaqDeviceHandle);
        argVals = QString("(%1)")
                .arg(mDaqDeviceHandle);

        funcStr = nameOfFunc + funcArgs + "Arg vals: " + argVals;
        if (err == ERR_NO_ERROR) {
            mMainWindow->addFunction(sStartTime + funcStr);
        } else {
            mMainWindow->setError(err, sStartTime + funcStr);
        }
    }

    nameOfFunc = "ulDaqOutScan";
    funcArgs = "(mDaqDeviceHandle, {chan0, type0, range0}, chanCount, "
               "mSamplesPerChan, &rate, mScanOptions, flags, &data)\n";

    sStartTime = t.currentTime().toString("hh:mm:ss.zzz") + "~";
    err = ulDaqOutScan(mDaqDeviceHandle, chanDescriptors, mChanCount,
                       mSamplesPerChan, &rate, mScanOptions, mDaqoFlags, buffer);
    argVals = QStringLiteral("(%1, {%2, %3, %4}, %5, %6, %7, %8, %9, %10)")
            .arg(mDaqDeviceHandle)
            .arg(chanDescriptors[0].channel)
            .arg(chanDescriptors[0].type)
            .arg(chanDescriptors[0].range)
            .arg(mChanCount)
            .arg(mSamplesPerChan)
            .arg(rate)
            .arg(mScanOptions)
            .arg(mDaqoFlags)
            .arg(*buffer);
    ui->lblInfo->setText(nameOfFunc + argVals + QString(" [Error = %1]").arg(err));

    funcStr = nameOfFunc + funcArgs + "Arg vals: " + argVals;
    if (!err==ERR_NO_ERROR) {
        mMainWindow->setError(err, sStartTime + funcStr);
    } else {
        mMainWindow->addFunction(sStartTime + funcStr);
        ui->lblRateReturned->setText(QString("%1").arg(rate, 1, 'f', 4, '0'));
        if (mUseWait) {
            qApp->processEvents();
            WaitType waitType = WAIT_UNTIL_DONE;
            long long waitParam = 0;
            nameOfFunc = "ulDaqOutScanWait";
            funcArgs = "(mDaqDeviceHandle, waitType, waitParam, timeout)\n";
            sStartTime = t.currentTime().toString("hh:mm:ss.zzz") + "~";
            err = ulDaqOutScanWait(mDaqDeviceHandle, waitType, waitParam, mWaitTime);
            argVals = QString("(%1, %2, %3, %4)")
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
        }
    }
}

void AoSubWidget::onClickCmdStop()
{
    QString nameOfFunc, funcArgs, argVals, funcStr;
    QTime t;
    QString sStartTime;

    mUseTimer = false;
    if ((mUtFunction == UL_AOUT_SCAN) | (mUtFunction == UL_DAQ_OUTSCAN)) {
        ScanStatus status;
        struct TransferStatus xferStatus;
        unsigned long long currentTotalCount;
        unsigned long long currentScanCount;
        long long currentIndex;

        if(mUseGetStatus) {
            funcArgs = "(mDaqDeviceHandle, &status, {&currentScanCount, &currentTotalCount, &currentIndex})\n";
            if (mUtFunction == UL_AOUT_SCAN) {
                nameOfFunc = "ulAOutScanStatus";
                sStartTime = t.currentTime().toString("hh:mm:ss.zzz") + "~";
                err = ulAOutScanStatus(mDaqDeviceHandle, &status, &xferStatus);
            } else {
                nameOfFunc = "ulDaqOutScanStatus";
                sStartTime = t.currentTime().toString("hh:mm:ss.zzz") + "~";
                err = ulDaqOutScanStatus(mDaqDeviceHandle, &status, &xferStatus);
            }
            mStatusTimerEnabled = false;
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
            ui->lblInfo->setText(QStringLiteral("Idle at %1, (%2 perChan), %3")
                                   .arg(currentTotalCount)
                                   .arg(currentScanCount)
                                   .arg(currentIndex));

            funcStr = nameOfFunc + funcArgs + "Arg vals: " + argVals;
            if (!(err == ERR_NO_ERROR)) {
                mMainWindow->setError(err, sStartTime + funcStr);
            } else {
                mMainWindow->addFunction(sStartTime + funcStr);
            }
        }
        err = stopScan(currentTotalCount, currentIndex);
        ui->cmdStop->setEnabled(mGoTmrIsRunning | mRunning);
    }
    if (mCancelAOut) {
        mCancelAOut = false;
        ui->cmdStop->setVisible(false);
    }
}

void AoSubWidget::checkStatus()
{
    ScanStatus status;
    struct TransferStatus xferStatus;
    QString nameOfFunc, funcArgs, argVals, funcStr;
    unsigned long long currentTotalCount;
    unsigned long long currentScanCount;
    long long currentIndex;
    QTime t;
    QString sStartTime;

    funcArgs = "(mDaqDeviceHandle, &status, {&currentScanCount, &currentTotalCount, &currentIndex})\n";
    if (mUtFunction == UL_DAQ_OUTSCAN) {
        nameOfFunc = "ulDaqOutScanStatus";
        sStartTime = t.currentTime().toString("hh:mm:ss.zzz") + "~";
        err = ulDaqOutScanStatus(mDaqDeviceHandle, &status, &xferStatus);
    } else {
        nameOfFunc = "ulAOutScanStatus";
        sStartTime = t.currentTime().toString("hh:mm:ss.zzz") + "~";
        err = ulAOutScanStatus(mDaqDeviceHandle, &status, &xferStatus);
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
    if ((!err==ERR_NO_ERROR) && mStatusTimerEnabled) {
        mStatusTimerEnabled = false;
        mMainWindow->setError(err, sStartTime + funcStr);
    } else if ((status == SS_RUNNING) && mStatusTimerEnabled) {
        //to do - add switch to enable logging
        //mMainWindow->addFunction(sStartTime + funcStr);
        ui->lblStatus->setText(QString("Running at total: %1, (%2 perPort), Index:  %3")
                               .arg(currentTotalCount)
                               .arg(currentScanCount)
                               .arg(currentIndex));
    } else {
        stopScan(currentTotalCount, currentIndex);
    }
    ui->cmdStop->setEnabled(mGoTmrIsRunning | mRunning);
}

UlError AoSubWidget::stopScan(long long curCount, long long curIndex)
{
    QString nameOfFunc, funcArgs, argVals, funcStr;
    QTime t;
    QString sStartTime;

    tmrCheckStatus->stop();
    if (mUtFunction == UL_DAQ_OUTSCAN) {
        nameOfFunc = "ulDaqOutScanStop";
        sStartTime = t.currentTime().toString("hh:mm:ss.zzz") + "~";
        err = ulDaqOutScanStop(mDaqDeviceHandle);
    } else {
        nameOfFunc = "ulAOutScanStop";
        sStartTime = t.currentTime().toString("hh:mm:ss.zzz") + "~";
        err = ulAOutScanStop(mDaqDeviceHandle);
    }
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
    if(mUseGetStatus) {
        ui->lblStatus->setText(QStringLiteral("Idle at    count:  %1,   index:  %2")
                               .arg(curCount).arg(curIndex));
    }
    mRunning = false;
    mPlotCount = curCount;
    return err;
}

void AoSubWidget::swStopScan()
{
    //this comes from escape key on main through ChildWindow
    onClickCmdStop();
}

void AoSubWidget::updatePlot()
{
    bool autoScale;
    double rangeBuf;
    double rangeUpper, rangeLower;

    autoScale = true;
    if(autoScale)
        ui->plotOutData->rescaleAxes();
    else {
        double rangeVolts = getRangeVolts(mRange);
        rangeBuf = rangeVolts / 10;
        rangeUpper = rangeVolts / 2;
        rangeLower = rangeUpper * -1;

        ui->plotOutData->xAxis->rescale();
        ui->plotOutData->yAxis->setRangeLower(rangeLower - rangeBuf);
        ui->plotOutData->yAxis->setRangeUpper(rangeUpper + rangeBuf);
    }
    ui->plotOutData->replot();
}
