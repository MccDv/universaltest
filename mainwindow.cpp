#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "unitest.h"
#include "uldaq.h"
#include "errordialog.h"

#define MAX_DEV_COUNT  100

struct DaqDeviceDescriptor devDescriptors[MAX_DEV_COUNT];
QHash<QString, DaqDeviceHandle> devList;

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    //int fontSize;
    QFont font;

    //fontSize = 8;
    font.setPointSize(10);

#ifdef Q_OS_MAC
    //fontSize = 12;
    font.setPointSize(12);
    this->setFont(font);
#endif

    mHistListSize = 50;
    functionGroup = new QActionGroup(this);

    inputModeGroup = new QActionGroup(this);
    inputModeGroup->addAction(ui->actionAI_DIFFERENTIAL);
    inputModeGroup->addAction(ui->actionAI_SINGLE_ENDED);
    inputModeGroup->addAction(ui->actionAI_PSEUDO_DIFFERENTIAL);

    ui->actionAI_DIFFERENTIAL->setData(AI_DIFFERENTIAL);
    ui->actionAI_SINGLE_ENDED->setData(AI_SINGLE_ENDED);
    ui->actionAI_PSEUDO_DIFFERENTIAL->setData(AI_PSEUDO_DIFFERENTIAL);

    ui->actionSO_DEFAULTIO->setData(SO_DEFAULTIO);
    ui->actionSO_SINGLEIO->setData(SO_SINGLEIO);
    ui->actionSO_BLOCKIO->setData(SO_BLOCKIO);
    ui->actionSO_BURSTIO->setData(SO_BURSTIO);

    optionGroup = new QActionGroup(this);
    optionGroup->addAction(ui->actionSO_DEFAULTIO);
    optionGroup->addAction(ui->actionSO_SINGLEIO);
    optionGroup->addAction(ui->actionSO_BLOCKIO);
    optionGroup->addAction(ui->actionSO_BURSTIO);

    ui->actionSO_CONTINUOUS->setData(SO_CONTINUOUS);
    ui->actionSO_EXTCLOCK->setData(SO_EXTCLOCK);
    ui->actionSO_EXTTRIGGER->setData(SO_EXTTRIGGER);
    ui->actionSO_RETRIGGER->setData(SO_RETRIGGER);
    ui->actionSO_BURSTMODE->setData(SO_BURSTMODE);
    ui->actionSO_PACEROUT->setData(SO_PACEROUT);

    rangeGroup = new QActionGroup(this);
    rangeGroup->addAction(ui->actionBIP20VOLTS);
    rangeGroup->addAction(ui->actionBIP10VOLTS);
    rangeGroup->addAction(ui->actionBIP5VOLTS);
    rangeGroup->addAction(ui->actionBIP4VOLTS);
    rangeGroup->addAction(ui->actionBIP2PT5VOLTS);
    rangeGroup->addAction(ui->actionBIP2VOLTS);
    rangeGroup->addAction(ui->actionBIP1PT25VOLTS);
    rangeGroup->addAction(ui->actionBIP1VOLTS);
    rangeGroup->addAction(ui->actionBIPPT078VOLTS);
    rangeGroup->addAction(ui->actionUNI10VOLTS);
    rangeGroup->addAction(ui->actionUNI5VOLTS);
    rangeGroup->addAction(ui->actionMA0TO20);

    ui->actionBIP20VOLTS->setData(BIP20VOLTS);
    ui->actionBIP10VOLTS->setData(BIP10VOLTS);
    ui->actionBIP5VOLTS->setData(BIP5VOLTS);
    ui->actionBIP4VOLTS->setData(BIP4VOLTS);
    ui->actionBIP2PT5VOLTS->setData(BIP2PT5VOLTS);
    ui->actionBIP2VOLTS->setData(BIP2VOLTS);
    ui->actionBIP1PT25VOLTS->setData(BIP1PT25VOLTS);
    ui->actionBIP1VOLTS->setData(BIP1VOLTS);
    ui->actionBIPPT078VOLTS->setData(BIPPT078VOLTS);
    ui->actionUNI10VOLTS->setData(UNI10VOLTS);
    ui->actionUNI5VOLTS->setData(UNI5VOLTS);
    ui->actionMA0TO20->setData(MA0TO20);

    scaleGroup = new QActionGroup(this);
    scaleGroup->addAction(ui->actionCelsius);
    scaleGroup->addAction(ui->actionFahrenheit);
    scaleGroup->addAction(ui->actionKelvin);
    scaleGroup->addAction(ui->actionVolts);
    scaleGroup->addAction(ui->actionNo_Scale);

    ui->actionCelsius->setData(TS_CELSIUS);
    ui->actionFahrenheit->setData(TS_FAHRENHEIT);
    ui->actionKelvin->setData(TS_KELVIN);
    ui->actionVolts->setData(TS_VOLTS);
    ui->actionNo_Scale->setData(TS_NOSCALE);

    trigTypeGroup = new QActionGroup(this);
    trigTypeGroup->addAction(ui->actionTRIG_NONE);
    trigTypeGroup->addAction(ui->actionTRIG_POS_EDGE);
    trigTypeGroup->addAction(ui->actionTRIG_NEG_EDGE);
    trigTypeGroup->addAction(ui->actionTRIG_HIGH);
    trigTypeGroup->addAction(ui->actionTRIG_LOW);
    trigTypeGroup->addAction(ui->actionGATE_HIGH);
    trigTypeGroup->addAction(ui->actionGATE_LOW);
    trigTypeGroup->addAction(ui->actionTRIG_RISING);
    trigTypeGroup->addAction(ui->actionTRIG_FALLING);
    trigTypeGroup->addAction(ui->actionTRIG_ABOVE);
    trigTypeGroup->addAction(ui->actionTRIG_BELOW);
    trigTypeGroup->addAction(ui->actionGATE_ABOVE);
    trigTypeGroup->addAction(ui->actionGATE_BELOW);
    trigTypeGroup->addAction(ui->actionGATE_IN_WINDOW);
    trigTypeGroup->addAction(ui->actionGATE_OUT_WINDOW);
    trigTypeGroup->addAction(ui->actionTRIG_PATTERN_EQ);
    trigTypeGroup->addAction(ui->actionTRIG_PATTERN_NE);
    trigTypeGroup->addAction(ui->actionTRIG_PATTERN_ABOVE);
    trigTypeGroup->addAction(ui->actionTRIG_PATTERN_BELOW);

    ui->actionTRIG_NONE->setData(TRIG_NONE);
    ui->actionTRIG_POS_EDGE->setData(TRIG_POS_EDGE);
    ui->actionTRIG_NEG_EDGE->setData(TRIG_NEG_EDGE);
    ui->actionTRIG_HIGH->setData(TRIG_HIGH);
    ui->actionTRIG_LOW->setData(TRIG_LOW);
    ui->actionGATE_HIGH->setData(GATE_HIGH);
    ui->actionGATE_LOW->setData(GATE_LOW);
    ui->actionTRIG_RISING->setData(TRIG_RISING);
    ui->actionTRIG_FALLING->setData(TRIG_FALLING);
    ui->actionTRIG_ABOVE->setData(TRIG_ABOVE);
    ui->actionTRIG_BELOW->setData(TRIG_BELOW);
    ui->actionGATE_ABOVE->setData(GATE_ABOVE);
    ui->actionGATE_BELOW->setData(GATE_BELOW);
    ui->actionGATE_IN_WINDOW->setData(GATE_IN_WINDOW);
    ui->actionGATE_OUT_WINDOW->setData(GATE_OUT_WINDOW);
    ui->actionTRIG_PATTERN_EQ->setData(TRIG_PATTERN_EQ);
    ui->actionTRIG_PATTERN_NE->setData(TRIG_PATTERN_NE);
    ui->actionTRIG_PATTERN_ABOVE->setData(TRIG_PATTERN_ABOVE);
    ui->actionTRIG_PATTERN_BELOW->setData(TRIG_PATTERN_BELOW);

    connect(ui->actionSet_Trigger, SIGNAL(triggered(bool)), this, SLOT(setTriggerParameters()));
    connect(ui->actionEventSetup, SIGNAL(triggered(bool)), this, SLOT(showEventSetup()));
    connect(trigTypeGroup, SIGNAL(triggered(QAction*)), this, SLOT(changeTrigType()));
    connect(optionGroup, SIGNAL(triggered(QAction*)), this, SLOT(curIoOptionChanged()));
    connect(ui->actionSO_CONTINUOUS, SIGNAL(triggered(bool)), this, SLOT(curOptionChanged()));
    connect(ui->actionSO_EXTCLOCK, SIGNAL(triggered(bool)), this, SLOT(curOptionChanged()));
    connect(ui->actionSO_EXTTRIGGER, SIGNAL(triggered(bool)), this, SLOT(curOptionChanged()));
    connect(ui->actionSO_RETRIGGER, SIGNAL(triggered(bool)), this, SLOT(curOptionChanged()));
    connect(ui->actionSO_BURSTMODE, SIGNAL(triggered(bool)), this, SLOT(curOptionChanged()));
    connect(ui->actionSO_PACEROUT, SIGNAL(triggered(bool)), this, SLOT(curOptionChanged()));

    connect(rangeGroup, SIGNAL(triggered(QAction*)), this, SLOT(curRangeChanged()));
    connect(inputModeGroup, SIGNAL(triggered(QAction*)), this, SLOT(curInputModeChanged()));
    connect(scaleGroup, SIGNAL(triggered(QAction*)), this, SLOT(curScaleChanged()));

    //connect(ui->actionRefresh_Devices, SIGNAL(triggered(bool)), this, SLOT(discoverDevices()));
    connect(ui->actionTmrLoop, SIGNAL(triggered(bool)), this, SLOT(setTimer()));
    connect(ui->actionFF_DEFAULT, SIGNAL(triggered(bool)), this, SLOT(setAiFlags()));
    connect(ui->actionFF_NOCALIBRATEDATA, SIGNAL(triggered(bool)), this, SLOT(setAiFlags()));
    connect(ui->actionFF_NOSCALEDATA, SIGNAL(triggered(bool)), this, SLOT(setAiFlags()));
    connect(ui->actionFF_SIMULTANEOUS, SIGNAL(triggered(bool)), this, SLOT(setAiFlags()));
    connect(ui->actionFF_CTR48_BIT, SIGNAL(triggered(bool)), this, SLOT(setAiFlags()));
    connect(ui->actionFF_CTR64_BIT, SIGNAL(triggered(bool)), this, SLOT(setAiFlags()));
    connect(ui->actionFF_NOCLEAR, SIGNAL(triggered(bool)), this, SLOT(setAiFlags()));

    connect(ui->cmdDiscover, SIGNAL(clicked(bool)), this, SLOT(createDiscChild()));
    connect(ui->cmdAIn, SIGNAL(clicked(bool)), this, SLOT(createAiChild()));
    connect(ui->cmdAOut, SIGNAL(clicked(bool)), this, SLOT(createAoChild()));
    connect(ui->cmdDIn, SIGNAL(clicked(bool)), this, SLOT(createDinChild()));
    connect(ui->cmdDOut, SIGNAL(clicked(bool)), this, SLOT(createDOutChild()));
    connect(ui->cmdCtr, SIGNAL(clicked(bool)), this, SLOT(createCtrChild()));
    connect(ui->cmdConfig, SIGNAL(clicked(bool)), this, SLOT(createCfgChild()));
    connect(ui->cmdMisc, SIGNAL(clicked(bool)), this, SLOT(createMiscChild()));
    connect(ui->cmdHistory, SIGNAL(clicked(bool)), this, SLOT(showHistory()));
    connect(ui->mdiArea, SIGNAL(subWindowActivated(QMdiSubWindow*)), this, SLOT(setBoardMenuSelect(QMdiSubWindow*)));
    connect(ui->actionVolts_vs_Time, SIGNAL(triggered(bool)), this, SLOT(showPlot(bool)));
    connect(ui->actionConfigure_Data, SIGNAL(triggered(bool)), this, SLOT(configureData()));
    connect(ui->actionLoad_Queue, SIGNAL(triggered(bool)), this, SLOT(configureQueue()));
    readWindowPosition();
    ui->chkAutoDetect->setChecked(mAutoConnect);

    qApp->processEvents();
    this->updateInventory();
    this->createMenus();
    this->createFuncMenus();

    mTrigChannel = 0;
    mTrigChanType = 0;
    mTrigRange = BIP10VOLTS;
    mTrigLevel = 1.0;
    mTrigVariance = 0.002;
    mRetrigCount = 0;
    mScanOptions = SO_DEFAULTIO;

}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::keyPressEvent(QKeyEvent *event)
{
    ChildWindow *curChild;
    int lastIndex;

    curChild = activeMdiChild();
    if (curChild) {
        int keyCode = event->key();
        if (keyCode == Qt::Key_Escape) {
            curChild->setTmrRunning(false);
            scanStopRequested(curChild);
        }
        ui->lblAppStatus->setText(QString("Key code %1").arg(keyCode));
        if ((keyCode == Qt::Key_Greater)
                | (keyCode == Qt::Key_Less)) {
            int curFuncIndex = functionGroup->actions().indexOf(functionGroup->checkedAction());
            functionGroup->actions().at(curFuncIndex)->setChecked(false);
            lastIndex = functionGroup->actions().count() - 1;
            if (keyCode == Qt::Key_Greater) {
                if (functionGroup->actions().at(curFuncIndex) == functionGroup->actions().at(lastIndex))
                    curFuncIndex = 0;
                else
                    curFuncIndex++;
            } else {
                if (functionGroup->actions().at(curFuncIndex) == (functionGroup->actions().at(0)))
                    curFuncIndex = functionGroup->actions().indexOf(functionGroup->actions().at(lastIndex));
                else
                    curFuncIndex--;
            }
            QAction* funcAction = functionGroup->actions().at(curFuncIndex);
            funcAction->setChecked(true);
            funcAction->triggered(true);
        }
        if (keyCode == Qt::Key_F5)
            curChild->refreshSelectedFunc();
    }
}

void MainWindow::closeEvent(QCloseEvent *event)
{
    foreach(DaqDeviceHandle dev, devList) {
        ulReleaseDaqDevice(dev);
    }
    writeWindowPosition();
    event->accept();
}

ChildWindow *MainWindow::activeMdiChild() const
{
    if (QMdiSubWindow *activeSubWindow = ui->mdiArea->activeSubWindow())
        return qobject_cast<ChildWindow *>(activeSubWindow);
    return 0;
}

void MainWindow::setTimer()
{
    ChildWindow *curChild;

    curChild = activeMdiChild();
    if (curChild) {
        curChild->setUpTimer();
    }
}

void MainWindow::readTmrParams()
{
    /*ChildWindow *curChild;
    int tmrInterval;
    bool tmrEnabled;

    tmrEnabled = tmrDialog->tmrEnable;
    tmrInterval = tmrDialog->tmrInterval;
    curChild = activeMdiChild();
    if (curChild) {
        curChild->setTmrEnabled(tmrEnabled);
        curChild->setTmrInterval(tmrInterval);
        ui->lblAppStatus->setText(curChild->windowTitle());
    } else {
        tmrEnabled = false;
    }
    //delete tmrDialog;
    ui->actionTmrLoop->setChecked(tmrEnabled);*/
}

void MainWindow::disableTimer()
{
    /*ChildWindow *curChild;
    bool tmrEnabled;

    tmrEnabled = false;
    curChild = activeMdiChild();
    if (curChild) {
        curChild->setTmrEnabled(tmrEnabled);
    }
    //delete tmrDialog;
    ui->actionTmrLoop->setChecked(tmrEnabled);*/
}

void MainWindow::showPlot(bool showIt)
{
    ChildWindow *curChild;

    curChild = activeMdiChild();
    if (curChild) {
        curChild->setShowPlot(showIt);
    }
    /*plotWindow = new PlotWindow(ui->mdiArea);
    plotWindow->setWindowState(Qt::WindowMaximized);
    plotWindow->setWindowState(Qt::WindowActive);
    plotWindow->show();*/
}

void MainWindow::createDiscChild()
{
    curFunctionGroupName = "Discovery Functions";
    createChild(FUNC_GROUP_DISC, UL_DISC);
}

void MainWindow::createCfgChild()
{
    curFunctionGroupName = "Information Functions";
    createChild(FUNC_GROUP_CONFIG, UL_GET_INFO);
}

void MainWindow::createMiscChild()
{
    curFunctionGroupName = "Miscellaneous Functions";
    createChild(FUNC_GROUP_MISC, UL_FLASH_LED);
}

void MainWindow::createAiChild()
{
    curFunctionGroupName = "Analog Input";
    createChild(FUNC_GROUP_AIN, UL_AIN);
}

void MainWindow::createAoChild()
{
    curFunctionGroupName = "Analog Output";
    createChild(FUNC_GROUP_AOUT, UL_AOUT);
}

void MainWindow::createDinChild()
{
    curFunctionGroupName = "Digital Input";
    createChild(FUNC_GROUP_DIN, UL_D_CONFIG_PORT);
}

void MainWindow::createDOutChild()
{
    curFunctionGroupName = "Digital Output";
    createChild(FUNC_GROUP_DOUT, UL_D_CONFIG_PORT);
}

void MainWindow::createCtrChild()
{
    ChildWindow *curChild;

    curFunctionGroupName = "Counter Functions";
    createChild(FUNC_GROUP_CTR, UL_C_LOAD); //windows doesn't size for some reason
    curChild = activeMdiChild();
    if (curChild) {
        curChild->setCurFunction(UL_C_LOAD);
        curChild->adjustSize();
    }
}

void MainWindow::createChild(UtFunctionGroup utFuncGroup, int defaultFunction)
{
    QString uidKey, displayIDString;
    DaqDeviceHandle devHandle;
    uidKey.clear();
    int descriptorIndex;

    devHandle = 0;
    descriptorIndex = 0;
    foreach (QAction *devAct, ui->menuBoards->actions()) {
        if (devAct->isChecked()) {
            uidKey = devAct->data().toString();
            break;
        }
    }
    ui->actionVolts_vs_Time->setChecked(false);

    for (int i = 0; i < MAX_DEV_COUNT; i++) {
        if (devDescriptors[i].uniqueId == uidKey) {
            descriptorIndex = i;
            break;
        }
    }
    if (devList.contains(uidKey)) {
        curBoardName = devDescriptors[descriptorIndex].devString;
        devHandle = devList.value(uidKey);
    }

    ChildWindow *childWindow = new ChildWindow(ui->mdiArea, utFuncGroup);
    childWindow->setWindowState(Qt::WindowMaximized);
    childWindow->setWindowState(Qt::WindowActive);
    childWindow->show();
    childWindow->setCurFunction(defaultFunction);
    createFuncMenus();
    childWindow->setDevHandle(devHandle);
    childWindow->setDevUID(uidKey);
    childWindow->setDevName(curBoardName);
    childWindow->setShowPlot(false);
    setBoardMenuSelect(childWindow);
    curUniqueID = uidKey;
    if (!curUniqueID.isEmpty())
        displayIDString = " [" + curUniqueID + "]";
    ui->lblAppStatus->setText(curFunctionGroupName + ": " + curBoardName + displayIDString);
    //connect(this, SIGNAL(scanStopRequested()), childWindow, SLOT(stopScan()));
    childWindow->refreshBoardParams();
    //childWindow->refreshSelectedFunc();
}

void MainWindow::createMenus()
{
    uint numDevs;
    QString bdName, uidKey;

    numDevs = devList.count();
    for (uint i=0;i<numDevs;i++) {
        bdName = devDescriptors[i].devString;
        uidKey = devDescriptors[i].uniqueId;
        bdAction = ui->menuBoards->addAction(bdName);
        bdAction->setCheckable(true);
        bdAction->setData(uidKey);
        connect(bdAction, SIGNAL(triggered(bool)), this, SLOT(setSelectedDevice()));
        if (i==0) {
            bdAction->setChecked(true);
            curBoardName = bdName;
            curUniqueID = uidKey;
        }
    }
}

void MainWindow::createFuncMenus()
{
    QAction *funcAction;
    ChildWindow *curChild;
    bool optionVisible, rangeVisible, trigVisible;
    bool dataVisible, inModeVisible, flagsVisible;
    bool plotVisible, simultaneousVisible;

    functionGroup->actions().clear();
    rangeVisible = false;
    optionVisible = false;
    trigVisible = true;
    dataVisible = false;
    inModeVisible = false;
    flagsVisible = true;
    simultaneousVisible = false;
    plotVisible = true;
    curChild = activeMdiChild();
    ui->actionFF_DEFAULT->setText("FF_DEFAULT");
    ui->actionFF_NOCALIBRATEDATA->setText("FF_NOCALIBRATEDATA");
    ui->actionFF_NOSCALEDATA->setText("FF_NOSCALEDATA");
    ui->actionFF_CTR48_BIT->setVisible(false);
    ui->actionFF_CTR64_BIT->setVisible(false);
    ui->actionFF_NOCLEAR->setVisible(false);

    if (curChild) {
        int funcGroup = curChild->currentFunctionGroup();
        ui->menuFunction->clear();

        switch (funcGroup) {
        case FUNC_GROUP_AIN:
            rangeVisible = true;
            optionVisible = true;
            inModeVisible = true;
            funcAction = ui->menuFunction->addAction("ulAIn");
            funcAction->setCheckable(true);
            funcAction->setChecked(true);
            funcAction->setData(UL_AIN);
            functionGroup->addAction(funcAction);
            funcAction = ui->menuFunction->addAction("ulAInScan");
            funcAction->setCheckable(true);
            funcAction->setData(UL_AINSCAN);
            functionGroup->addAction(funcAction);
            funcAction = ui->menuFunction->addAction("ulDaqInScan");
            funcAction->setCheckable(true);
            funcAction->setData(UL_DAQ_INSCAN);
            functionGroup->addAction(funcAction);
            funcAction = ui->menuFunction->addAction("ulTIn");
            funcAction->setCheckable(true);
            funcAction->setData(UL_TIN);
            functionGroup->addAction(funcAction);
            funcAction = ui->menuFunction->addAction("ulTInArray");
            funcAction->setCheckable(true);
            funcAction->setData(UL_TINARRAY);
            functionGroup->addAction(funcAction);
            ui->actionFF_DEFAULT->setData(AIN_FF_DEFAULT);
            ui->actionFF_NOCALIBRATEDATA->setData(AIN_FF_NOCALIBRATEDATA);
            ui->actionFF_NOSCALEDATA->setData(AIN_FF_NOSCALEDATA);
            ui->actionFF_NOCLEAR->setVisible(true);
            break;
        case FUNC_GROUP_AOUT:
            rangeVisible = true;
            optionVisible = true;
            dataVisible = true;
            funcAction = ui->menuFunction->addAction("ulAOut");
            funcAction->setCheckable(true);
            funcAction->setChecked(true);
            funcAction->setData(UL_AOUT);
            functionGroup->addAction(funcAction);
            funcAction = ui->menuFunction->addAction("ulAOutScan");
            funcAction->setCheckable(true);
            funcAction->setData(UL_AOUT_SCAN);
            functionGroup->addAction(funcAction);
            funcAction = ui->menuFunction->addAction("ulDaqOutScan");
            funcAction->setCheckable(true);
            funcAction->setData(UL_DAQ_OUTSCAN);
            functionGroup->addAction(funcAction);
            funcAction = ui->menuFunction->addAction("ulAOutArray");
            funcAction->setCheckable(true);
            funcAction->setData(UL_AOUTARRAY);
            functionGroup->addAction(funcAction);
            ui->actionFF_DEFAULT->setData(AOUT_FF_DEFAULT);
            ui->actionFF_NOCALIBRATEDATA->setData(AOUT_FF_NOCALIBRATEDATA);
            ui->actionFF_NOSCALEDATA->setData(AOUT_FF_NOSCALEDATA);
            ui->actionFF_SIMULTANEOUS->setData(AOUTARRAY_FF_SIMULTANEOUS);
            simultaneousVisible = true;
            break;
        case FUNC_GROUP_DIN:
            optionVisible = true;
            flagsVisible = false;
            funcAction = ui->menuFunction->addAction("ulDConfigPort");
            funcAction->setCheckable(true);
            funcAction->setChecked(true);
            functionGroup->addAction(funcAction);
            funcAction->setData(UL_D_CONFIG_PORT);
            funcAction = ui->menuFunction->addAction("ulDConfigBit");
            funcAction->setCheckable(true);
            functionGroup->addAction(funcAction);
            funcAction->setData(UL_D_CONFIG_BIT);
            funcAction = ui->menuFunction->addAction("ulDIn");
            funcAction->setCheckable(true);
            functionGroup->addAction(funcAction);
            funcAction->setData(UL_D_IN);
            funcAction = ui->menuFunction->addAction("ulDBitIn");
            funcAction->setCheckable(true);
            functionGroup->addAction(funcAction);
            funcAction->setData(UL_D_BIT_IN);
            funcAction = ui->menuFunction->addAction("ulDInScan");
            funcAction->setCheckable(true);
            functionGroup->addAction(funcAction);
            funcAction->setData(UL_D_INSCAN);
            funcAction = ui->menuFunction->addAction("ulDInArray");
            funcAction->setCheckable(true);
            functionGroup->addAction(funcAction);
            funcAction->setData(UL_D_INARRAY);
            break;
        case FUNC_GROUP_DOUT:
            optionVisible = true;
            flagsVisible = false;
            dataVisible = true;
            funcAction = ui->menuFunction->addAction("ulDConfigPort");
            funcAction->setCheckable(true);
            funcAction->setChecked(true);
            functionGroup->addAction(funcAction);
            funcAction->setData(UL_D_CONFIG_PORT);
            funcAction = ui->menuFunction->addAction("ulDConfigBit");
            funcAction->setCheckable(true);
            functionGroup->addAction(funcAction);
            funcAction->setData(UL_D_CONFIG_BIT);
            funcAction = ui->menuFunction->addAction("ulDOut");
            funcAction->setCheckable(true);
            functionGroup->addAction(funcAction);
            funcAction->setData(UL_D_OUT);
            funcAction = ui->menuFunction->addAction("ulDBitOut");
            funcAction->setCheckable(true);
            functionGroup->addAction(funcAction);
            funcAction->setData(UL_D_BIT_OUT);
            funcAction = ui->menuFunction->addAction("ulDOutScan");
            funcAction->setCheckable(true);
            functionGroup->addAction(funcAction);
            funcAction->setData(UL_D_OUTSCAN);
            funcAction = ui->menuFunction->addAction("ulDOutArray");
            funcAction->setCheckable(true);
            functionGroup->addAction(funcAction);
            funcAction->setData(UL_D_OUTARRAY);
            break;
        case FUNC_GROUP_CTR:
            optionVisible = true;
            /*funcAction = ui->menuFunction->addAction("Select Ctr");
            funcAction->setCheckable(true);
            funcAction->setChecked(true);
            functionGroup->addAction(funcAction);
            funcAction->setData(UL_C_SELECT);*/
            funcAction = ui->menuFunction->addAction("ulCLoad");
            funcAction->setCheckable(true);
            funcAction->setChecked(true);
            functionGroup->addAction(funcAction);
            funcAction->setData(UL_C_LOAD);
            funcAction = ui->menuFunction->addAction("ulCIn");
            funcAction->setCheckable(true);
            functionGroup->addAction(funcAction);
            funcAction->setData(UL_C_IN);
            funcAction = ui->menuFunction->addAction("ulCClear");
            funcAction->setCheckable(true);
            functionGroup->addAction(funcAction);
            funcAction->setData(UL_C_CLEAR);
            funcAction = ui->menuFunction->addAction("ulCInScan");
            funcAction->setCheckable(true);
            functionGroup->addAction(funcAction);
            funcAction->setData(UL_C_INSCAN);
            funcAction = ui->menuFunction->addAction("ulTmrPulseOut");
            funcAction->setCheckable(true);
            functionGroup->addAction(funcAction);
            funcAction->setData(UL_TMR_OUT);
            funcAction = ui->menuFunction->addAction("ulCConfigScan");
            funcAction->setCheckable(true);
            functionGroup->addAction(funcAction);
            funcAction->setData(UL_C_CONFIG_SCAN);
            ui->actionFF_DEFAULT->setText("FF_DEFAULT");
            ui->actionFF_NOCALIBRATEDATA->setText("FF_CTR16_BIT");
            ui->actionFF_NOSCALEDATA->setText("FF_CTR32_BIT");
            ui->actionFF_DEFAULT->setData(CINSCAN_FF_DEFAULT);
            ui->actionFF_NOCALIBRATEDATA->setData(CINSCAN_FF_CTR16_BIT);
            ui->actionFF_NOSCALEDATA->setData(CINSCAN_FF_CTR32_BIT);
            ui->actionFF_CTR48_BIT->setData(CINSCAN_FF_CTR48_BIT);
            ui->actionFF_CTR64_BIT->setData(CINSCAN_FF_CTR64_BIT);
            ui->actionFF_CTR48_BIT->setVisible(true);
            ui->actionFF_CTR64_BIT->setVisible(true);
            ui->actionFF_NOCLEAR->setData(CINSCAN_FF_NOCLEAR);
            ui->actionFF_NOCLEAR->setVisible(true);
            break;
        case FUNC_GROUP_MISC:
            flagsVisible = false;
            trigVisible = false;
            plotVisible = false;
            funcAction = ui->menuFunction->addAction("ulFlashLED");
            funcAction->setCheckable(true);
            funcAction->setChecked(true);
            funcAction->setData(UL_FLASH_LED);
            functionGroup->addAction(funcAction);
            funcAction = ui->menuFunction->addAction("ulGetErrMsg");
            funcAction->setCheckable(true);
            funcAction->setData(UL_GET_ERR_MSG);
            functionGroup->addAction(funcAction);
            funcAction = ui->menuFunction->addAction("ulGetStatus");
            funcAction->setCheckable(true);
            funcAction->setData(UL_GET_STATUS);
            functionGroup->addAction(funcAction);
            funcAction = ui->menuFunction->addAction("ulStopBackground");
            funcAction->setCheckable(true);
            funcAction->setData(UL_SCAN_STOP);
            functionGroup->addAction(funcAction);
            funcAction = ui->menuFunction->addAction("ulMemRead");
            funcAction->setCheckable(true);
            funcAction->setData(UL_MEM_READ);
            functionGroup->addAction(funcAction);
            funcAction = ui->menuFunction->addAction("ulMemWrite");
            funcAction->setCheckable(true);
            funcAction->setData(UL_MEM_WRITE);
            functionGroup->addAction(funcAction);
            break;
        case FUNC_GROUP_CONFIG:
            flagsVisible = false;
            trigVisible = false;
            plotVisible = false;
            funcAction = ui->menuFunction->addAction("ulGetInfo");
            funcAction->setCheckable(true);
            funcAction->setChecked(true);
            funcAction->setData(UL_GET_INFO);
            functionGroup->addAction(funcAction);
            funcAction = ui->menuFunction->addAction("ulGetConfig");
            funcAction->setCheckable(true);
            funcAction->setData(UL_GET_CONFIG);
            functionGroup->addAction(funcAction);
            funcAction = ui->menuFunction->addAction("ulSetConfig");
            funcAction->setCheckable(true);
            funcAction->setData(UL_SET_CONFIG);
            functionGroup->addAction(funcAction);
            break;
        default:
            break;
        }
        disconnect(functionGroup);
        connect(functionGroup, SIGNAL(triggered(QAction*)), this, SLOT(curFunctionChanged()));
    }
    ui->menuRange->menuAction()->setVisible(rangeVisible);
    ui->menuOptions->menuAction()->setVisible(optionVisible);
    ui->menuTriggering->menuAction()->setVisible(trigVisible);
    ui->menuPlot->menuAction()->setVisible(plotVisible);
    ui->menuData_Generator->menuAction()->setVisible(dataVisible);
    ui->actionAI_DIFFERENTIAL->setVisible(inModeVisible);
    ui->actionAI_PSEUDO_DIFFERENTIAL->setVisible(inModeVisible);
    ui->actionAI_SINGLE_ENDED->setVisible(inModeVisible);
    ui->actionFF_DEFAULT->setVisible(flagsVisible);
    ui->actionFF_NOSCALEDATA->setVisible(flagsVisible);
    ui->actionFF_NOCALIBRATEDATA->setVisible(flagsVisible);
    ui->actionFF_SIMULTANEOUS->setVisible(simultaneousVisible);
}

void MainWindow::setBoardMenuSelect(QMdiSubWindow*)
{
    bool optionVisible, rangeVisible;
    bool plotShowing;
    int curFunc, curRange;
    Range newRange;
    double rangeVolts;
    QString preFix, str;
    QString curBoard, displayIDString;
    QString curUID, actionName;
    ChildWindow *curChild;

    rangeVisible = false;
    optionVisible = false;
    plotShowing = false;
    curChild = activeMdiChild();
    if (curChild) {
        curBoard = curChild->devName();
        if (curBoard == "")
            return;

        curUID = curChild->devUID();
        createFuncMenus();
        foreach (QAction *boardAction, ui->menuBoards->actions()) {
            QString bdName = boardAction->text();
            if (bdName == curBoard) {
                QString childUId = boardAction->data().toString();
                if (childUId == curUID) {
                    boardAction->setChecked(true);
                    curBoardName = curBoard;
                    curUniqueID = curChild->devUID();
                } else
                    boardAction->setChecked(false);
            } else
                boardAction->setChecked(false);
        }
        int curFuncGroup = curChild->currentFunctionGroup();
        curFunc = curChild->currentFunction();
        mScanOptions = curChild->scanOptions();
        switch (curFuncGroup) {
        case FUNC_GROUP_AIN:
            curFunctionGroupName = "Analog Input";
            rangeVisible = true;
            optionVisible = true;
            switch (curFunc) {
            case UL_AIN:
                actionName = "ulAIn";
                break;
            case UL_AINSCAN:
                actionName = "ulAInScan";
                break;
            case UL_DAQ_INSCAN:
                actionName = "ulDaqInScan";
                break;
            case UL_TIN:
                actionName = "ulTIn";
                break;
            default:
                actionName = "ulAIn";
                break;
            }
            break;
        case FUNC_GROUP_AOUT:
            curFunctionGroupName = "Analog Output";
            rangeVisible = true;
            optionVisible = true;
            switch (curFunc) {
            case UL_AOUT:
                actionName = "ulAOut";
                break;
            case UL_AOUT_SCAN:
                actionName = "ulAOutScan";
                break;
            case UL_DAQ_OUTSCAN:
                actionName = "ulDaqOutScan";
                break;
            default:
                actionName = "ulAOut";
                break;
            }
            break;
        case FUNC_GROUP_DIN:
            curFunctionGroupName = "Digital Input";
            optionVisible = true;
            switch (curFunc) {
            case UL_D_CONFIG_PORT:
                actionName = "ulDConfigPort";
                break;
            case UL_D_CONFIG_BIT:
                actionName = "ulDConfigBit";
                break;
            case UL_D_IN:
                actionName = "ulDIn";
                break;
            case UL_D_BIT_IN:
                actionName = "ulDBitIn";
                break;
            case UL_D_OUT:
                actionName = "ulDOut";
                break;
            case UL_D_BIT_OUT:
                actionName = "ulDBitOut";
                break;
            default:
                actionName = "ulDConfigPort";
                break;
            }
            break;
        case FUNC_GROUP_DOUT:
            curFunctionGroupName = "Digital Output";
            optionVisible = true;
            switch (curFunc) {
            case UL_D_CONFIG_PORT:
                actionName = "ulDConfigPort";
                break;
            case UL_D_CONFIG_BIT:
                actionName = "ulDConfigBit";
                break;
            case UL_D_OUT:
                actionName = "ulDOut";
                break;
            case UL_D_BIT_OUT:
                actionName = "ulDBitOut";
                break;
            default:
                actionName = "ulDConfigPort";
                break;
            }
            break;
        case FUNC_GROUP_CTR:
            curFunctionGroupName = "Counter Functions";
            optionVisible = true;
            switch (curFunc) {
            case UL_C_SELECT:
                actionName = "Select Ctr";
                break;
            case UL_C_IN:
                actionName = "ulCIn";
                break;
            case UL_C_CLEAR:
                actionName = "ulCClear";
                break;
            case UL_C_INSCAN:
                actionName = "ulCInScan";
                break;
            case UL_TMR_OUT:
                actionName = "ulTmrPulseOut";
                break;
            default:
                break;
            }
            break;
        case FUNC_GROUP_MISC:
            curFunctionGroupName = "Miscellaneous Functions";
            rangeVisible = false;
            optionVisible = false;
            switch (curFunc) {
            case UL_FLASH_LED:
                actionName = "ulFlashLED";
                break;
            case UL_GET_ERR_MSG:
                actionName = "ulGetErrMsg";
                break;
            case UL_GET_STATUS:
                actionName = "ulGetStatus";
                break;
            case UL_SCAN_STOP:
                actionName = "ulStopScan";
                break;
            default:
                actionName = "ulFlashLED";
                break;
            }
            break;
        case FUNC_GROUP_CONFIG:
            curFunctionGroupName = "Information Functions";
            rangeVisible = false;
            optionVisible = false;
            switch (curFunc) {
            case UL_GET_INFO:
                actionName = "ulGetInfo";
                break;
            case UL_GET_CONFIG:
                actionName = "ulGetConfig";
                break;
            case UL_SET_CONFIG:
                actionName = "ulSetConfig";
                break;
            default:
                actionName = "ulGetInfo";
                break;
            }
            break;
        case FUNC_GROUP_DISC:
            curFunctionGroupName = "Discovery Functions";
            rangeVisible = false;
            optionVisible = false;
            break;
        default:
            break;
        }
        foreach (QAction *funcAction, ui->menuFunction->actions()) {
            if (funcAction->text() == actionName) {
                funcAction->setChecked(true);
                break;
            }
        }
        if (rangeVisible) {
            curRange = curChild->getCurrentRange();
            foreach (QAction *rangeAction, ui->menuRange->actions()) {
                bool update = ((rangeAction->actionGroup() == rangeGroup) && (curFunc < 6));
                update = update | ((rangeAction->actionGroup() == rangeGroup) && (curFunc > 5));
                if (update) {
                    rangeAction->setChecked(false);
                    if (rangeAction->data().toInt() == (int)curRange)
                        rangeAction->setChecked(true);
                }
            }
            newRange = (Range)curRange;
            double vSpan = getRangeVolts(newRange);
            if (newRange < 100) {
                rangeVolts = vSpan / 2;
                preFix = "±";
            } else {
                rangeVolts = vSpan;
                preFix = "0 to ";
            }
            ui->menuRange->menuAction()->setText("Range (" + preFix + str.setNum(rangeVolts) +")");
        }
        if (optionVisible) {
            //set menu selections to match options for selected child window
            long long curFlags = 0;
            switch (curFuncGroup) {
            case FUNC_GROUP_AIN:
                switch (curFunc) {
                case UL_AIN:
                case UL_AINSCAN:
                    curFlags = (long long)curChild->aiFlags();
                    break;
                case UL_DAQ_INSCAN:
                    curFlags = (long long)curChild->daqiFlags();
                    break;
                default:
                    break;
                }
                break;
            case FUNC_GROUP_AOUT:
                switch (curFunc) {
                case UL_AOUT_SCAN:
                    curFlags = (long long)curChild->aoFlags();
                    break;
                case UL_DAQ_OUTSCAN:
                    curFlags = (long long)curChild->daqOutFlag();
                    break;
                default:
                    break;
                }
                break;
            case FUNC_GROUP_CTR:
                curFlags = (long long)curChild->ciFlags();
                break;
            default:
                break;
            }
            AiInputMode curInMode = curChild->inputMode();
            ui->actionFF_DEFAULT->setChecked(true);
            ui->actionSO_DEFAULTIO->setChecked(true);
            foreach (QAction *ioMode, ui->menuOptions->actions()) {
                QString menuName = ioMode->text();
                if (menuName.startsWith("SO_")) {
                    if (!optionGroup->actions().contains(ioMode))
                        ioMode->setChecked(false);
                    ScanOption curMenuVal = (ScanOption)(ioMode->data().toLongLong());
                    if (curMenuVal & mScanOptions) {
                        ioMode->setChecked(true);
                    }
                }
                if (menuName.startsWith("FF_")) {
                    long long menuVal = ioMode->data().toLongLong();
                    if (menuVal != 0)
                        ioMode->setChecked(false);
                    if (menuVal & curFlags) {
                        ioMode->setChecked(true);
                        ui->actionFF_DEFAULT->setChecked(false);
                    }
                }
                if (inputModeGroup->actions().contains(ioMode)) {
                    AiInputMode curMenuVal = (AiInputMode)(ioMode->data().toLongLong());
                    if (curMenuVal == curInMode)
                        ioMode->setChecked(true);
                }
            }
        }
        if (!curUniqueID.isEmpty())
            displayIDString = " [" + curUniqueID + "]";
        ui->lblAppStatus->setText(curFunctionGroupName + ": " + curBoardName + displayIDString);
        plotShowing = curChild->showPlot();
    }
    ui->actionVolts_vs_Time->setChecked(plotShowing);
    ui->menuRange->menuAction()->setVisible(rangeVisible);
    ui->menuOptions->menuAction()->setVisible(optionVisible);
}

void MainWindow::setSelectedDevice()
{
    ChildWindow *mdiChild = activeMdiChild();

    QString function, displayIDString;
    DaqDeviceHandle devHandle;
    int descriptorIndex;

    descriptorIndex = 0;
    foreach (QAction *devAct, ui->menuBoards->actions()) {
        devAct->setChecked(false);
    }
    QObject *devSelected = this->sender();
    QAction *actDevSelected = qobject_cast<QAction *>(devSelected);
    devSelected->setProperty("checked", true);
    QVariant uidVar = actDevSelected->data();
    curUniqueID = uidVar.toString();
    for (int i = 0; i < MAX_DEV_COUNT; i++) {
        if (devDescriptors[i].uniqueId == curUniqueID) {
            descriptorIndex = i;
            break;
        }
    }
    devHandle = devList.value(curUniqueID); // deviceHandle.toInt();
    curBoardName = devDescriptors[descriptorIndex].productName;
    curUniqueID = devDescriptors[descriptorIndex].uniqueId;

    if(mdiChild) {
        function = mdiChild->currentFunction();
        mdiChild->setDevUID(curUniqueID);
        mdiChild->setDevHandle(devHandle);
        mdiChild->setDevName(curBoardName);
        if (!curUniqueID.isEmpty())
            displayIDString = " [" + curUniqueID + "]";
        ui->lblAppStatus->setText(curFunctionGroupName + ": " + curBoardName + displayIDString);

        mdiChild->refreshBoardParams();
        //mdiChild->refreshSelectedFunc();
    }
}

void MainWindow::curFunctionChanged()
{
    int curFunction;
    ChildWindow *curChild;

    curChild = activeMdiChild();
    if (curChild) {
        curFunction = functionGroup->checkedAction()->data().toInt();
        curChild->setCurFunction(curFunction);
        curChild->setTmrEnabled(false);
        bool showIt = curChild->showPlot();
        ui->actionVolts_vs_Time->setChecked(showIt);
    }
}

void MainWindow::curRangeChanged()
{
    ChildWindow *curChild = activeMdiChild();
    Range newRange;
    double rangeVolts;
    QString preFix, str;

    int rangeVar = rangeGroup->checkedAction()->data().toInt();
    newRange = (Range)rangeVar;
    double vSpan = getRangeVolts(newRange);
    if (newRange < 100) {
        rangeVolts = vSpan / 2;
        preFix = "±";
    } else {
        rangeVolts = vSpan;
        preFix = "0 to ";
    }
    ui->menuRange->menuAction()->setText("Range (" + preFix + str.setNum(rangeVolts) +")");
    if (curChild) {
        curChild->setCurRange(newRange);
    }
}

void MainWindow::curScaleChanged()
{
    ChildWindow *curChild = activeMdiChild();
    TempScale newScale;
    //double rangeVolts;
    QString preFix, str;

    int scaleVar = scaleGroup->checkedAction()->data().toInt();
    newScale = (TempScale)scaleVar;
    /*double vSpan = getRangeVolts(newRange);
    if (newRange < 100) {
        rangeVolts = vSpan / 2;
        preFix = "±";
    } else {
        rangeVolts = vSpan;
        preFix = "0 to ";
    }*/
    //ui->menuRange->menuAction()->setText("Range (" + preFix + str.setNum(rangeVolts) +")");
    if (curChild) {
        curChild->setCurScale(newScale);
    }
}

void MainWindow::curInputModeChanged()
{
    ChildWindow *curChild = activeMdiChild();
    int newMode;
    AiInputMode inputMode;

    newMode = inputModeGroup->checkedAction()->data().toInt();
    if (curChild) {
        inputMode = (AiInputMode)newMode;
        curChild->setInputMode(inputMode);
    }
}

void MainWindow::setAiFlags()
{
    ChildWindow *mdiChild = activeMdiChild();
    AInFlag aiFlag;
    AOutFlag aoFlag;
    CInScanFlag ctrFlag;
    DaqInScanFlag daqiFlag;
    DaqOutScanFlag daqoFlag;
    int tempFlag;

    aiFlag = (AInFlag)0;
    aoFlag = (AOutFlag)0;
    ctrFlag = (CInScanFlag)0;
    daqiFlag = (DaqInScanFlag)0;
    daqoFlag = (DaqOutScanFlag)0;
    tempFlag = 0;

    QObject *flagSelected = this->sender();
    QAction *actflagSelected = qobject_cast<QAction *>(flagSelected);
    actflagSelected->setChecked(actflagSelected->isChecked());
    if (actflagSelected == ui->actionFF_DEFAULT) {
        if (ui->actionFF_DEFAULT->isChecked()) {
            ui->actionFF_NOCALIBRATEDATA->setChecked(false);
            ui->actionFF_NOSCALEDATA->setChecked(false);
            ui->actionFF_SIMULTANEOUS->setChecked(false);
            ui->actionFF_CTR48_BIT->setChecked(false);
            ui->actionFF_CTR64_BIT->setChecked(false);
            ui->actionFF_NOCLEAR->setChecked(false);
        }
    } else {
        if (ui->actionFF_NOSCALEDATA->isChecked())
            tempFlag = ui->actionFF_NOSCALEDATA->data().toInt();
        if (ui->actionFF_NOCALIBRATEDATA->isChecked())
            tempFlag |= ui->actionFF_NOCALIBRATEDATA->data().toInt();
        if (ui->actionFF_SIMULTANEOUS->isChecked())
            tempFlag |= ui->actionFF_SIMULTANEOUS->data().toInt();
        if (ui->actionFF_CTR48_BIT->isChecked())
            tempFlag |= ui->actionFF_CTR48_BIT->data().toInt();
        if (ui->actionFF_CTR64_BIT->isChecked())
            tempFlag |= ui->actionFF_CTR64_BIT->data().toInt();
        if (ui->actionFF_NOCLEAR->isChecked())
            tempFlag |= ui->actionFF_NOCLEAR->data().toInt();
    }

    ui->actionFF_DEFAULT->setChecked(!tempFlag);
    if (mdiChild) {
        int curFuncGroup = mdiChild->currentFunctionGroup();
        int curFunction = mdiChild->currentFunction();
        switch (curFuncGroup) {
        case FUNC_GROUP_AIN:
            switch (curFunction) {
            case UL_AIN:
            case UL_AINSCAN:
                aiFlag = static_cast<AInFlag>(tempFlag);
                mdiChild->setAiFlags(aiFlag);
                break;
            case UL_DAQ_INSCAN:
                daqiFlag = static_cast<DaqInScanFlag>(tempFlag);
                mdiChild->setDaqiFlags(daqiFlag);
                break;
            default:
                break;
            }
            break;
        case FUNC_GROUP_AOUT:
            if ((curFunction == UL_AOUT_SCAN) | (curFunction == UL_AOUT)) {
                aoFlag = static_cast<AOutFlag>(tempFlag);
                mdiChild->setAoFlags(aoFlag);
            }
            if (curFunction == UL_DAQ_OUTSCAN) {
                daqoFlag = static_cast<DaqOutScanFlag>(tempFlag);
                mdiChild->setDaqOutFlag(daqoFlag);
            }
            if (curFunction == UL_AOUTARRAY) {
                aoFlag = static_cast<AOutFlag>(tempFlag);
                mdiChild->setAoFlags(aoFlag);
            }
            break;
        case FUNC_GROUP_CTR:
            ctrFlag = static_cast<CInScanFlag>(tempFlag);
            mdiChild->setCiFlags(ctrFlag);
            break;
        case FUNC_GROUP_DAQSCAN:
            //this group never set
            break;
        default:
            break;
        }
    }
}

void MainWindow::curIoOptionChanged()
{
    ChildWindow *curChild = activeMdiChild();

    QVariant optionValue = optionGroup->checkedAction()->data();
    ScanOption optVal = (ScanOption)optionValue.toLongLong();
    ScanOption maskFilter = (ScanOption)(SO_SINGLEIO | SO_BLOCKIO | SO_BURSTIO);
    ScanOption mask = getSoMask(maskFilter);
    mScanOptions = (ScanOption)(mScanOptions & mask);
    mScanOptions = (ScanOption)(mScanOptions | optVal);
    if (curChild) {
        curChild->setScanOptions(mScanOptions);
    }
}

void MainWindow::changeTrigType()
{
    ChildWindow *curChild = activeMdiChild();

    QVariant trigSetVal = trigTypeGroup->checkedAction()->data();
    TriggerType trigType = (TriggerType)trigSetVal.toLongLong();

    if (curChild) {
       // if (trigType > TRIG_FALLING) {
       //     setTriggerParameters();
       //}
        curChild->setTrigChannel(mTrigChannel);
        curChild->setTrigChanType(mTrigChanType);
        curChild->setTrigLevel(mTrigLevel);
        curChild->setTrigVariance(mTrigVariance);
        curChild->setRetrigCount(mRetrigCount);
        curChild->setTrigRange(mTrigRange);
        curChild->setTriggerType(trigType);
    }
}

void MainWindow::configureData()
{
    ChildWindow *curChild = activeMdiChild();

    if (curChild) {
        curChild->showDataConfig();
    }
}

void MainWindow::showEventSetup()
{
    ChildWindow *curChild = activeMdiChild();
    bool status, waitEnabled;
    double waitTime;

    if (curChild) {
        int curGroup = curChild->currentFunctionGroup();
        if ((curGroup == FUNC_GROUP_CONFIG)
                | (curGroup == FUNC_GROUP_DISC)
                | (curGroup == FUNC_GROUP_MISC)) {
            QMessageBox box;
            box.setText("Selected window isn't compatible with events.");
            box.exec();
            return;
        }
        DaqDeviceHandle devHandle = curChild->devHandle();
        eventSetup = new EventsDialog(this, devHandle);
        status = curChild->statusEnabled();
        waitEnabled = curChild->waitEnabled();
        waitTime = curChild->waitTime();
        eventSetup->setCheckStatusEnabled(status);
        eventSetup->setWaitForDone(waitEnabled);
        eventSetup->setWaitTime(waitTime);
        connect(eventSetup, SIGNAL(accepted()), this, SLOT(eventDialogResponse()));
        eventSetup->exec();
    } else {
        QMessageBox box;
        box.setText("First open a window to receive the events, then try again.");
    }
}

void MainWindow::eventDialogResponse()
{
    ChildWindow *curChild = activeMdiChild();

    bool statusEnabled = eventSetup->checkStatusEnabled();
    bool waitEnabled = eventSetup->waitForDone();
    double waitTime = eventSetup->waitTime();

    if (curChild) {
        curChild->setStatusEnabled(statusEnabled);
        curChild->setWaitEnabled(waitEnabled);
        curChild->setWaitTime(waitTime);
        curChild->updateEventSetup();
    }
    disconnect(eventSetup);
    delete eventSetup;
}

void MainWindow::configureQueue()
{
    ChildWindow *curChild = activeMdiChild();

    if (curChild) {
        curChild->showQueueConfig();
    }
}

void MainWindow::setTriggerParameters()
{
    trigDialog = new TrigDialog(this);
    connect(trigDialog, SIGNAL(accepted()), this, SLOT(trigDialogResponse()));
    trigDialog->setTrigChannel(mTrigChannel);
    trigDialog->setTrigChanType(mTrigChanType);
    trigDialog->setTrigLevel(mTrigLevel);
    trigDialog->setTrigVariance(mTrigVariance);
    trigDialog->setRetrigCount(mRetrigCount);
    trigDialog->setTrigRange(mTrigRange);
    trigDialog->exec();
}

void MainWindow::trigDialogResponse()
{
    mTrigChannel = trigDialog->trigChannel();
    mTrigChanType = trigDialog->trigChanType();
    mTrigLevel = trigDialog->trigLevel();
    mTrigVariance = trigDialog->trigVariance();
    mRetrigCount = trigDialog->retrigCount();
    mTrigRange = trigDialog->trigRange();
    disconnect(trigDialog);
    delete trigDialog;
    changeTrigType();
}

void MainWindow::curOptionChanged()
{
    ChildWindow *curChild = activeMdiChild();

    QAction *menuOption = (QAction *)this->sender();
    QVariant optionValue = menuOption->data();
    bool enableOption = menuOption->isChecked();
    ScanOption optVal = (ScanOption)optionValue.toLongLong();

    if (enableOption)
        mScanOptions = (ScanOption)(mScanOptions | optVal);
    else {
        ScanOption mask = getSoMask(optVal);
        mScanOptions = (ScanOption)(mScanOptions & mask);
    }
    if (curChild) {
        curChild->setScanOptions(mScanOptions);
    }
}

ScanOption MainWindow::getSoMask(ScanOption optSelected)
{
    ScanOption allOpts = (ScanOption)(
                SO_BLOCKIO | SO_CONTINUOUS | SO_EXTCLOCK |
                SO_SINGLEIO | SO_EXTTRIGGER | SO_RETRIGGER |
                SO_BURSTMODE | SO_BURSTIO | SO_PACEROUT);
    ScanOption maskVal = (ScanOption)(allOpts ^ optSelected);
    return maskVal;
}

void MainWindow::scanStopRequested(ChildWindow *curChild)
{
    curChild->stopScan();
}

void MainWindow::addFunction(QString funcString)
{
    mFuncHistoryList.append(funcString);
    if (mFuncHistoryList.size() > mHistListSize)
        mFuncHistoryList.removeFirst();
}

void MainWindow::setError(UlError curError, QString funcText)
{
    QString errStr, str;

    errStr = "Error" + str.setNum((int)curError) + "_";
    mFuncHistoryList.append(errStr + funcText);
    showHistory();
}

void MainWindow::showHistory()
{
    errDlg.setModal(true);
    errDlg.showHistory(mFuncHistoryList);
    errDlg.exec();
    if (errDlg.historyCleared())
        mFuncHistoryList.clear();
    mHistListSize = errDlg.getHistSize();
    for (int i = mFuncHistoryList.size(); i > mHistListSize; i--)
        mFuncHistoryList.removeFirst();
    errDlg.setHistoryCleared(false);
}

void MainWindow::discoverDevices()
{
    ChildWindow *curChild = activeMdiChild();

    if (curChild) {
        if (curChild->currentFunctionGroup() == FUNC_GROUP_DISC) {
            curChild->setCurFunction(UL_DISC);
        }
    }
}

void MainWindow::updateInventory()
{
    unsigned int numDevs = MAX_DEV_COUNT;
    QString uidKey, funcName;
    QString funcStr, argString, argVals;
    DaqDeviceHandle deviceHandle;
    QTime t;
    QString sStartTime;

    DaqDeviceInterface interfaceType = ANY_IFC;
    sStartTime = t.currentTime().toString("hh:mm:ss.zzz") + "~";
    UlError err = ulGetDaqDeviceInventory(interfaceType, devDescriptors, &numDevs);

    funcName = "ulGetDaqDeviceInventory";
    argString = "(interfaceType, devDescriptors, &numDevs)\n";
    argVals = QStringLiteral("(%1, %2, %3)")
            .arg(interfaceType)
            .arg("devDescriptors")
            .arg(numDevs);
    ui->lblAppStatus->setText(funcName + argVals + QString(" [Error = %1]").arg(err));

    funcStr = funcName + argString + "Arg vals: " + argVals;
    if(err != ERR_NO_ERROR) {
        setError(err, sStartTime + funcStr);
        //funcStr = funcName + argString + "\n";
        //QString errStr = funcStr + "Arg vals: " + argVals;
    } else {
        addFunction(sStartTime + funcStr);
        QString temp;
        temp = ui->lblAppStatus->text() + ";  ";
        for (uint i=0; i<numDevs;i++) {
            sStartTime = t.currentTime().toString("hh:mm:ss.zzz") + "~";
            deviceHandle = ulCreateDaqDevice(devDescriptors[i]);
            funcName = "ulCreateDaqDevice";
            argString = "({devDescriptor, devString, productId, uniqueId})\n";

            argVals = QStringLiteral("({%1, %2, %3, %4})")
                    .arg(devDescriptors[i].devInterface)
                    .arg(devDescriptors[i].devString)
                    .arg(devDescriptors[i].productId)
                    .arg(devDescriptors[i].uniqueId);
            temp = temp.append(funcName + argVals + ";  ");
            ui->lblAppStatus->setText(temp);

            funcStr = funcName + argString + "Arg vals: " + argVals;
            addFunction(sStartTime + funcStr);
            if (deviceHandle != 0) {
                uidKey = devDescriptors[i].uniqueId;
                devList.insert(uidKey, deviceHandle);
            }

            if(mAutoConnect) {
                funcName = "ulConnectDaqDevice";
                argString = "(deviceHandle)\n";
                sStartTime = t.currentTime().toString("hh:mm:ss.zzz") + "~";
                qApp->processEvents();
                UlError err = ulConnectDaqDevice(deviceHandle);
                argVals = QStringLiteral("(%1)")
                        .arg(deviceHandle);
                funcStr = funcName + argString + "Arg vals: " + argVals;
                if (err != ERR_NO_ERROR) {
                    setError(err, sStartTime + funcStr);
                } else {
                    addFunction(sStartTime + funcStr);
                }
            }
            temp = temp.append(funcName + argVals + QString(" [Error = %1]").arg(err) + ";  ");

            ui->lblAppStatus->setText(temp);

            //funcStr = funcName + argString + "Arg vals: " + argVals;
            //if (err != ERR_NO_ERROR) {
            //    setError(err, sStartTime + funcStr);
            //} else {
            //    addFunction(sStartTime + funcStr);
                if (numDevs) {
                    for (uint i=0;i<numDevs;i++) {
                        uidKey = devDescriptors[i].uniqueId;
                        if (!devList.contains(uidKey))
                            devList.remove(uidKey);
                    }
                }
            //}
        }
    }
}

void MainWindow::removeDeviceFromMenu(QString devUiD)
{
    unsigned int numDevs = MAX_DEV_COUNT;
    QString funcName;
    QString funcStr, argString, argVals;
    bool wasChecked;

    wasChecked = false;
    foreach (QAction *boardMenu, ui->menuBoards->actions()) {
        QVariant dataVal = boardMenu->data();
        if (dataVal.toString() == devUiD) {
            if (curUniqueID == devUiD) {
                curBoardName = "";
                curUniqueID = "";
            }
            devList.remove(devUiD);
            wasChecked = boardMenu->isChecked();
            delete boardMenu;
        }
        if (wasChecked)
            if (ui->menuBoards->actions().count())
                ui->menuBoards->actions().at(0)->setChecked(true);
    }

    DaqDeviceInterface interfaceType = ANY_IFC;
    UlError err = ulGetDaqDeviceInventory(interfaceType, devDescriptors, &numDevs);

    funcName = "ulGetDaqDeviceInventory";
    argString = "(interfaceType, devDescriptors, &numDevs)";
    argVals = QStringLiteral("(%1, %2, %3)")
            .arg(interfaceType)
            .arg("devDescriptors")
            .arg(numDevs);
    ui->lblAppStatus->setText(funcName + argVals + QString(" [Error = %1]").arg(err));

    if(err != ERR_NO_ERROR)
    {
        funcStr = funcName + argString + "\n";
        QString errStr = funcStr + "Arg vals: " + argVals;
        ErrorDialog errDlg;
        errDlg.setModal(true);
        errDlg.setError(err, errStr);
        errDlg.exec();
    }
}

void MainWindow::addDeviceToMenu(QString devName, QString devUiD, DaqDeviceHandle devHandle)
{
    unsigned int numDevs = MAX_DEV_COUNT;
    QString funcName;
    QString funcStr, argString, argVals;
    QAction *newAction, *checkedAction;
    bool itemExists;

    itemExists = false;
    checkedAction = NULL;
    devList.insert(devUiD, devHandle);
    foreach (QAction *bAct, ui->menuBoards->actions()) {
        QString menuDevUiD = bAct->data().toString();
        if (menuDevUiD == devUiD) {
            itemExists = true;
            break;
        }
        if (bAct->isChecked()) {
            bAct->setChecked(false);
            checkedAction = bAct;
        }
    }
    if (itemExists) {
        if (checkedAction)
            checkedAction->setChecked(true);
        return;
    }

    newAction = ui->menuBoards->addAction(devName);
    newAction->setCheckable(true);
    newAction->setData(devUiD);
    newAction->setChecked(true);
    connect(newAction, SIGNAL(triggered(bool)), this, SLOT(setSelectedDevice()));

    DaqDeviceInterface interfaceType = ANY_IFC;
    UlError err = ulGetDaqDeviceInventory(interfaceType, devDescriptors, &numDevs);

    funcName = "ulGetDaqDeviceInventory";
    argString = "(interfaceType, devDescriptors, &numDevs)";
    argVals = QStringLiteral("(%1, %2, %3)")
            .arg(interfaceType)
            .arg("devDescriptors")
            .arg(numDevs);
    ui->lblAppStatus->setText(funcName + argVals + QString(" [Error = %1]").arg(err));

    if(err != ERR_NO_ERROR)
    {
        funcStr = funcName + argString + "\n";
        QString errStr = funcStr + "Arg vals: " + argVals;
        ErrorDialog errDlg;
        errDlg.setModal(true);
        errDlg.setError(err, errStr);
        errDlg.exec();
    }
}

void MainWindow::syncScale(TempScale childScale)
{
    foreach (QAction* scaleAct, ui->menuRange->actions()) {
        if (scaleAct->data() == childScale) {
            scaleAct->setChecked(true);
        }
    }
}

void MainWindow::syncRange(Range range)
{
    double rangeVolts;
    QString preFix, str;

    foreach (QAction* rangeAct, ui->menuRange->actions()) {
        if (rangeAct->data() == range) {
            rangeAct->setChecked(true);
        }
    }
    double vSpan = getRangeVolts(range);
    if (range < 100) {
        rangeVolts = vSpan / 2;
        preFix = "±";
    } else {
        rangeVolts = vSpan;
        preFix = "0 to ";
    }
    ui->menuRange->menuAction()->setText("Range (" + preFix + str.setNum(rangeVolts) +")");
}

QHash<QString, DaqDeviceHandle> MainWindow::getListedDevices()
{
    return devList;
}

void MainWindow::readWindowPosition()
{
    QSettings windowSettings("Measurement Computing", "Universal Test Linux");
    QVariant autoConnect;

    windowSettings.beginGroup("mainwindow");

    autoConnect = windowSettings.value("autoconnect", true);
    restoreGeometry(windowSettings.value("geometry", saveGeometry()).toByteArray());
    restoreState(windowSettings.value("savestate", saveState()).toByteArray());
    move(windowSettings.value("pos", pos()).toPoint());
    resize(windowSettings.value("size", size()).toSize());
    if (windowSettings.value("maximized", isMaximized()).toBool())
        showMaximized();

    windowSettings.endGroup();
    mAutoConnect = autoConnect.toBool();
}

void MainWindow::writeWindowPosition()
{
    QSettings windowSettings("Measurement Computing", "Universal Test Linux");

    mAutoConnect = ui->chkAutoDetect->isChecked();
    windowSettings.beginGroup("mainwindow");

    windowSettings.setValue("autoconnect", mAutoConnect);
    windowSettings.setValue("geometry", saveGeometry());
    windowSettings.setValue("savestate", saveState());
    windowSettings.setValue("maximized", isMaximized());
    if (!isMaximized()) {
        windowSettings.setValue("pos", pos());
        windowSettings.setValue("size", size());
    }
    windowSettings.endGroup();
}
