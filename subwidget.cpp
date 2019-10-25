#include "subwidget.h"
#include "ui_subwidget.h"
#include <qfont.h>
#include "childwindow.h"
#include "unitest.h"
#include "uldaq.h"
#include "errordialog.h"

subWidget::subWidget(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::subWidget)
{
    ui->setupUi(this);
    int fontSize;
    QFont font;

    fontSize = 8;
    font.setPointSize(10);

#ifdef Q_OS_MAC
    fontSize = 12;
    font.setPointSize(12);
    this->setFont(font);
#endif

    ui->teShowValues->setFont(QFont ("Courier", fontSize));
    ui->fraCenter->setFont(font);
    ui->fraSelector->setFont(font);
    ui->teShowValues->setStyleSheet("QTextEdit { background-color : white; color : blue; }" );
    ui->lblStatus->setStyleSheet("QLabel { background-color : white; color : blue; }" );
    //connect(ui->spnIndex, SIGNAL(valueChanged(int)), SLOT(runSelectedFunc()));
    connect(ui->cmdStop, SIGNAL(clicked(bool)), this, SLOT(onStopCmd()));
    //connect(ui->cmdSet, SIGNAL(clicked(bool)), this, SLOT(runSelectedFunc()));
    //connect(ui->cmbConfigItem, SIGNAL(currentIndexChanged(int)), this, SLOT(runSelectedFunc()));
    //connect(ui->spnIndex, SIGNAL(valueChanged(int)), this, SLOT(runSelectedFunc()));
    mMainWindow = getMainWindow();
}

subWidget::~subWidget()
{
    delete ui;
}

void subWidget::keyPressEvent(QKeyEvent *event)
{
    int keyCode = event->key();
    if ((keyCode == Qt::Key_Plus) && (QApplication::keyboardModifiers() & Qt::AltModifier)) {
        mPrintResolution += 1;
        mHexResolution += 1;
        ui->lblStatus->setText(QString("Text resolution %1").arg(mPrintResolution));
    }
    if ((keyCode == Qt::Key_Minus) && (QApplication::keyboardModifiers() & Qt::AltModifier)) {
        mPrintResolution -= 1;
        mHexResolution -= 1;
        if (mPrintResolution < 0)
            mPrintResolution = 0;
        if (mHexResolution < 2)
            mHexResolution = 2;
        ui->lblStatus->setText(QString("Text resolution %1").arg(mPrintResolution));
    }
    if ((keyCode == Qt::Key_X) && (QApplication::keyboardModifiers() & Qt::AltModifier)) {
        mShowHex = !mShowHex;
        ui->lblStatus->setText(QString("Print hex %1").arg(mShowHex));
    }
}

MainWindow *subWidget::getMainWindow()
{
    foreach (QWidget *w, QApplication::topLevelWidgets())
        if (QMainWindow* mainWin = qobject_cast<QMainWindow*>(w))
            return qobject_cast<MainWindow *>(mainWin);
    return nullptr;
}

void subWidget::updateParameters()
{
    ChildWindow *parentWindow;

    parentWindow = qobject_cast<ChildWindow *>(this->parent());
    mDaqDeviceHandle = parentWindow->devHandle();
    mDevName = parentWindow->devName();
    mDevUID = parentWindow->devUID();
    mUseTimer = parentWindow->tmrEnabled();

    if(mUseTimer)
        ui->cmdStop->setEnabled(true);
    mGoTmrIsRunning = parentWindow->tmrRunning();
    ui->cmdStop->setEnabled(mGoTmrIsRunning);

    this->setWindowTitle(mFuncName + ": " + mDevName + QString(" [%1]").arg(mDaqDeviceHandle));
    ui->teShowValues->clear();
    //runSelectedFunc();
}

void subWidget::groupChanged(int newGroup)
{
    mCurGroup = newGroup;
    setUiForGroup();
}

void subWidget::setUiForGroup()
{
    //ChildWindow *parentWindow;
    bool configVisible;
    QString cmdLabel;
    //parentWindow = qobject_cast<ChildWindow *>(this->parent());

    configVisible = false;
    disconnect(ui->cmbInfoType, SIGNAL(currentIndexChanged(int)));
    disconnect(ui->cmdSet, SIGNAL(clicked(bool)));
    //disconnect(ui->cmbConfigItem, SIGNAL(currentIndexChanged(int)));
    disconnect(ui->spnIndex, SIGNAL(valueChanged(int)));
    switch (mCurGroup) {
    case FUNC_GROUP_MISC:
        connect(ui->cmdSet, SIGNAL(clicked(bool)), this, SLOT(runSelectedFunc()));
        //connect(ui->cmbConfigItem, SIGNAL(currentIndexChanged(int)), this, SLOT(runSelectedFunc()));
        //connect(ui->spnIndex, SIGNAL(valueChanged(int)), this, SLOT(runSelectedFunc()));
        cmdLabel = "Go";
        break;
    case FUNC_GROUP_CONFIG:
        //connect(ui->cmbInfoType, SIGNAL(currentIndexChanged(int)), this, SLOT(setConfigItemsForType()));
        connect(ui->cmdSet, SIGNAL(clicked(bool)), this, SLOT(setConfiguration()));
        connect(ui->spnIndex, SIGNAL(valueChanged(int)), this, SLOT(runSelectedFunc()));
        configVisible = true;
        cmdLabel = "Set";
        break;
    case FUNC_GROUP_MEM:
        connect(ui->cmdSet, SIGNAL(clicked(bool)), this, SLOT(memRead()));
        break;
    default:
        break;
    }
    ui->cmbConfigItem->setVisible(configVisible);
    //ui->teShowValues->setVisible(configVisible);
    ui->leSetValue->setVisible(configVisible);
    ui->cmdSet->setText(cmdLabel);
}

void subWidget::functionChanged(int utFunction)
{
    mUtFunction = utFunction;
    this->setUiForFunction();
    //runSelectedFunc();
}

void subWidget::setUiForFunction()
{
    bool userFrameVisible, configComboVisible;
    bool infoComboVisible, cmdSetVisible;
    QString cmdLabel;

    configComboVisible = false;
    infoComboVisible = true;
    cmdSetVisible = false;
    userFrameVisible = false;

    ui->cmbInfoType->clear();
    ui->spnIndex->setValue(0);
    disconnect(ui->cmbConfigItem, SIGNAL(currentIndexChanged(int)));
    disconnect(ui->cmbInfoType, SIGNAL(currentIndexChanged(int)));
    QCoreApplication::processEvents(QEventLoop::AllEvents, 100);
    switch (mCurGroup) {
    case FUNC_GROUP_MISC:
        cmdLabel = "Go";
        switch (mUtFunction) {
        case UL_FLASH_LED:
            mFuncName = "ulFlashLED";
            infoComboVisible = false;
            ui->cmbInfoType->addItem("ulFlashLED");
            ui->spnIndex->setValue(2);
            break;
        case UL_GET_ERR_MSG:
            mFuncName = "ulGetErrMsg";
            infoComboVisible = false;
            break;
        case UL_GET_STATUS:
            mFuncName = "ulGetStatus";
            ui->cmbInfoType->addItem("ulTmrPulseOutStatus");
            ui->cmbInfoType->addItem("ulAInScanStatus");
            ui->cmbInfoType->addItem("ulAOutScanStatus");
            ui->cmbInfoType->addItem("ulDInScanStatus");
            ui->cmbInfoType->addItem("ulDOutScanStatus");
            ui->cmbInfoType->addItem("ulCInScanStatus");
            ui->cmbInfoType->addItem("ulDaqInScanStatus");
            ui->cmbInfoType->addItem("ulDaqOutScanStatus");
            break;
        case UL_SCAN_STOP:
            mFuncName = "ulStopScan";
            ui->cmbInfoType->addItem("ulAInScanStop");
            ui->cmbInfoType->addItem("ulAOutScanStop");
            ui->cmbInfoType->addItem("ulDInScanStop");
            ui->cmbInfoType->addItem("ulDOutScanStop");
            ui->cmbInfoType->addItem("ulCInScanStop");
            ui->cmbInfoType->addItem("ulDaqInScanStop");
            ui->cmbInfoType->addItem("ulDaqOutScanStop");
            break;
        case UL_MEM_READ:
        case UL_MEM_WRITE:
            ui->cmbInfoType->addItem("Cal Region", MR_CAL);
            ui->cmbInfoType->addItem("User Region", MR_USER);
            ui->cmbInfoType->addItem("Settings Region", MR_SETTINGS);
            ui->cmbInfoType->addItem("Reserved Region0", MR_RESERVED0);
            connect(ui->cmdSet, SIGNAL(clicked(bool)), this, SLOT(memRead()));
            infoComboVisible = true;
            break;
        default:
            break;
        }
        cmdSetVisible = true;
        userFrameVisible = true;
        break;
    case FUNC_GROUP_CONFIG:
        ui->cmbInfoType->clear();
        cmdLabel = "Set";
        //configComboVisible = true;
        switch (mUtFunction) {
        case UL_GET_INFO:
            mFuncName = "ulGetInfo";
            ui->cmbInfoType->addItem("UL Info", TYPE_UL_INFO);
            ui->cmbInfoType->addItem("Dev Info", TYPE_DEV_INFO);
            ui->cmbInfoType->addItem("AI Info", TYPE_AI_INFO);
            ui->cmbInfoType->addItem("AO Info", TYPE_AO_INFO);
            ui->cmbInfoType->addItem("DIO Info", TYPE_DIO_INFO);
            ui->cmbInfoType->addItem("Ctr Info", TYPE_CTR_INFO);
            ui->cmbInfoType->addItem("Tmr Info", TYPE_TMR_INFO);
            ui->cmbInfoType->addItem("DaqIn Info", TYPE_DAQI_INFO);
            ui->cmbInfoType->addItem("DaqOut Info", TYPE_DAQO_INFO);
            ui->cmbInfoType->addItem("Mem Info", TYPE_MEM_INFO);
            connect(ui->cmbInfoType, SIGNAL(currentIndexChanged(int)), this, SLOT(runSelectedFunc()));
            break;
        case UL_GET_CONFIG:
            mFuncName = "ulGetConfig";
            ui->cmbInfoType->addItem("Get UL Config", TYPE_UL_INFO);
            ui->cmbInfoType->addItem("Get Dev Config", TYPE_DEV_INFO);
            ui->cmbInfoType->addItem("Get AI Config", TYPE_AI_INFO);
            ui->cmbInfoType->addItem("Get AO Config", TYPE_AO_INFO);
            ui->cmbInfoType->addItem("Get DIO Config", TYPE_DIO_INFO);
            ui->cmbInfoType->addItem("Get Ctr Config", TYPE_CTR_INFO);
            ui->cmbInfoType->addItem("Get Tmr Config", TYPE_TMR_INFO);
            //connect(ui->cmbInfoType, SIGNAL(currentIndexChanged(int)), this, SLOT(runSelectedFunc()));
            break;
        case UL_SET_CONFIG:
            //connect(ui->cmbConfigItem, SIGNAL(currentIndexChanged(int)), this, SLOT(runSelectedFunc()));
            mFuncName = "ulSetConfig";
            configComboVisible = true;
            //lineEditVisible = true;
            userFrameVisible = true;
            cmdSetVisible = true;
            ui->cmbInfoType->addItem("Set UL Config", TYPE_UL_INFO);
            ui->cmbInfoType->addItem("Set AI Config", TYPE_AI_INFO);
            ui->cmbInfoType->addItem("Set AO Config", TYPE_AO_INFO);
            ui->cmbInfoType->addItem("Set DIO Config", TYPE_DIO_INFO);
            ui->cmbInfoType->addItem("Set Ctr Config", TYPE_CTR_INFO);
            connect(ui->cmbInfoType, SIGNAL(currentIndexChanged(int)), this, SLOT(setConfigItemsForType()));
            break;
        default:
            break;
        }
    default:
        break;
    }
    ui->fraUser->setVisible(userFrameVisible);
    //ui->leSetValue->setVisible(lineEditVisible);
    ui->cmbConfigItem->setVisible(configComboVisible);
    ui->cmbInfoType->setVisible(infoComboVisible);
    ui->cmdSet->setVisible(cmdSetVisible);
    ui->cmdSet->setText(cmdLabel);
    ui->cmdSet->setFocus();
    this->setWindowTitle(mFuncName + ": " + mDevName + QString(" [%1]").arg(mDaqDeviceHandle));
}

void subWidget::setConfigItemsForType()
{
    int configType;

    configType = ui->cmbInfoType->currentData(Qt::UserRole).toInt();
    ui->cmbConfigItem->clear();
    switch (configType) {
    case TYPE_UL_INFO:
        ui->cmbConfigItem->addItem("USB Xfer Priority", UL_CFG_USB_XFER_PRIORITY);
        break;
    case TYPE_AI_INFO:
        ui->cmbConfigItem->addItem("AIn Chan Type", AI_CFG_CHAN_TYPE);
        ui->cmbConfigItem->addItem("Chan Tc Type", AI_CFG_CHAN_TC_TYPE);
        ui->cmbConfigItem->addItem("Chan Temp Unit", AI_CFG_SCAN_CHAN_TEMP_UNIT);
        ui->cmbConfigItem->addItem("Temp Unit", AI_CFG_SCAN_TEMP_UNIT);
        ui->cmbConfigItem->addItem("Adc Timing Mode", AI_CFG_ADC_TIMING_MODE);
        ui->cmbConfigItem->addItem("Auto Zero Mode", AI_CFG_AUTO_ZERO_MODE);
        ui->cmbConfigItem->addItem("IEPE Mode", AI_CFG_CHAN_IEPE_MODE);
        ui->cmbConfigItem->addItem("Chan Coupling Mode", AI_CFG_CHAN_COUPLING_MODE);
        ui->cmbConfigItem->addItem("Chan Snsr Connection", AI_CFG_CHAN_SENSOR_CONNECTION_TYPE);
        ui->cmbConfigItem->addItem("Chan OTD Mode", AI_CFG_CHAN_OTD_MODE);
        ui->cmbConfigItem->addItem("Chan Slope", AI_CFG_CHAN_SLOPE);
        ui->cmbConfigItem->addItem("Chan Offset", AI_CFG_CHAN_OFFSET);
        ui->cmbConfigItem->addItem("Chan Sensor Sensitivity", AI_CFG_CHAN_SENSOR_SENSITIVITY);
        ui->cmbConfigItem->addItem("Chan Data Rate", AI_CFG_CHAN_DATA_RATE);
        break;
    case TYPE_AO_INFO:
        ui->cmbConfigItem->addItem("AO Sync Mode", AO_CFG_SYNC_MODE);
        ui->cmbConfigItem->addItem("AO Sense Mode", AO_CFG_CHAN_SENSE_MODE);
        break;
    case TYPE_DIO_INFO:
        ui->cmbConfigItem->addItem("Dio Port Dir Mask", DIO_CFG_PORT_DIRECTION_MASK);
        ui->cmbConfigItem->addItem("Dio Port Out Val", DIO_CFG_PORT_INITIAL_OUTPUT_VAL);
        ui->cmbConfigItem->addItem("Dio Iso Filter Mask", DIO_CFG_PORT_ISO_FILTER_MASK);
        ui->cmbConfigItem->addItem("Dio Port Out Logic", DIO_CFG_PORT_LOGIC);
        break;
    case TYPE_CTR_INFO:
        ui->cmbConfigItem->addItem("Ctr Cfg Reg Mask", CTR_CFG_REG);
        break;
    default:
        break;
    }
}

void subWidget::runSelectedFunc()
{
    ChildWindow *parentWindow;
    QFont goFont = ui->cmdSet->font();
    bool makeBold, tmrIsEnabled, tmrIsRunning;
    //bool showStop;

    parentWindow = qobject_cast<ChildWindow *>(this->parent());
    mUseTimer = parentWindow->tmrEnabled();
    ui->teShowValues->clear();

    switch (mCurGroup) {
    case FUNC_GROUP_MISC:
        switch (mUtFunction) {
        case UL_FLASH_LED:
            setMiscFunction();
            break;
        case UL_GET_ERR_MSG:
            getErrorMessage();
            break;
        case UL_GET_STATUS:
            setMiscFunction();
            break;
        case UL_SCAN_STOP:
            setMiscFunction();
            break;
        default:
            break;
        }
        break;
    case FUNC_GROUP_CONFIG:
        switch (mUtFunction) {
        case UL_GET_INFO:
            readInfo();
            break;
        case UL_GET_CONFIG:
            readConfig();
            break;
        case UL_SET_CONFIG:
            //setConfiguration();
            break;
        default:
            break;
        }
        break;
    default:
        break;
    }

    tmrIsEnabled = parentWindow->tmrEnabled();
    tmrIsRunning = parentWindow->tmrRunning();
    if (!tmrIsEnabled) {
        if (tmrIsRunning)
            parentWindow->setTmrRunning(false);
        ui->cmdStop->setEnabled(false);
        mUseTimer = false;
        goFont.setBold(false);
        ui->cmdSet->setFont(goFont);
    } else {
        if (mUseTimer) {
            if (!tmrIsRunning) {
                parentWindow->setTmrRunning(true);
                ui->cmdStop->setEnabled(true);
            }
            makeBold = !ui->cmdSet->font().bold();
            goFont.setBold(makeBold);
            ui->cmdSet->setFont(goFont);
        } else {
            if (tmrIsRunning) {
                parentWindow->setTmrRunning(false);
                ui->cmdStop->setEnabled(false);
            }
            goFont.setBold(false);
            ui->cmdSet->setFont(goFont);
        }
    }
}

void subWidget::onStopCmd()
{
    mUseTimer = false;
    ui->cmdStop->setEnabled(false);
}

void subWidget::memRead()
{
    QString nameOfFunc, funcArgs, argVals;
    QString str, s;
    QTime t;
    QString sStartTime, funcStr;
    QString result = "";
    MemRegion memRegion;
    MemDescriptor memDescriptor;
    unsigned int address;
    unsigned int maxMemLen;

    memRegion = (MemRegion)ui->cmbInfoType->currentData(Qt::UserRole).toInt();
    nameOfFunc = "ulMemGetInfo";
    funcArgs = "(mDaqDeviceHandle, memRegion, {accessTypes, address, size})\n";
    sStartTime = t.currentTime().toString("hh:mm:ss.zzz") + "~";
    err = ulMemGetInfo(mDaqDeviceHandle, memRegion, &memDescriptor);
    argVals = QStringLiteral("(%1, %2, {%3, %4, %5})")
            .arg(mDaqDeviceHandle)
            .arg(memRegion)
            .arg(memDescriptor.accessTypes)
            .arg(memDescriptor.address)
            .arg(memDescriptor.size);

    funcStr = nameOfFunc + funcArgs + "Arg vals: " + argVals;
    if(err != ERR_NO_ERROR) {
        mMainWindow->setError(err, sStartTime + funcStr);
        ui->lblStatus->setText(nameOfFunc + argVals + QString(" [Error = %1]").arg(err));
    } else {
        maxMemLen = memDescriptor.size;
        address = memDescriptor.address;
        unsigned char memValue[maxMemLen];
        unsigned char *pMemValue = memValue;
        nameOfFunc = "ulMemRead";
        sStartTime = t.currentTime().toString("hh:mm:ss.zzz") + "~";
        err = ulMemRead(mDaqDeviceHandle, memRegion, address, pMemValue, maxMemLen);
        funcArgs = "(mDaqDeviceHandle, memRegion, address, pMemValue, maxMemLen)\n";
        argVals = QStringLiteral("(%1, %2, %3, %4, %5)")
                .arg(mDaqDeviceHandle)
                .arg(memRegion)
                .arg(address)
                .arg(memValue[0])
                .arg(maxMemLen);

        int rev = strlen(reinterpret_cast<const char*>(memValue));
        // Print String in Reverse order....
        for (int i = 0; i<rev; i++) {
            s = QString("%1").arg(memValue[i], 0, 16);
            if(s == "0")
                  s="00";
             result.append(s);
        }
        funcStr = nameOfFunc + funcArgs + "Arg vals: " + argVals;
        str = getRegionNames(memRegion);
        if(err != ERR_NO_ERROR) {
            mMainWindow->setError(err, sStartTime + funcStr);
            ui->lblStatus->setText(nameOfFunc + argVals + QString(" [Error = %1]").arg(err));
        } else {
            ui->lblStatus->setText(nameOfFunc + argVals);
            ui->teShowValues->setText(result);
            mMainWindow->addFunction(sStartTime + funcStr);
        }
    }
}

void subWidget::readInfo()
{
    int infoItem, infoType;
    QString devID, devInfo, infoText, str;

    infoType = ui->cmbInfoType->currentData(Qt::UserRole).toInt();
    showIndex = false;

    infoText = "<style> th, td { padding-right: 10px;}</style><tr>";

    switch (infoType) {
    case TYPE_UL_INFO:
        infoItem = UL_INFO_VER_STR;
        infoText = showInfoStr(0, infoItem, "UL Version");
        ui->teShowValues->setHtml(infoText);
        break;
    case TYPE_DEV_INFO:
        //devID = QStringLiteral("Device ID (%1) = %2")
        //        .arg(mDaqDeviceHandle).arg(mDevUID);
        //ui->teShowValues->append(devID);

        devID = "<td>Device ID [" + str.setNum(mDaqDeviceHandle)
                + "]</td><td></td><td>" + mDevUID + "</td></tr><tr>";
        infoText.append(devID + "</tr><tr>");
        infoItem = DEV_INFO_HAS_AI_DEV;
        devInfo = showInfo(infoType, infoItem, "DevHasAI");
        infoText.append(devInfo + "</tr><tr>");
        infoItem = DEV_INFO_HAS_AO_DEV;
        devInfo = showInfo(infoType, infoItem, "DevHasAO");
        infoText.append(devInfo  + "</tr><tr>");
        infoItem = DEV_INFO_HAS_DIO_DEV;
        devInfo = showInfo(infoType, infoItem, "DevHasDIO");
        infoText.append(devInfo  + "</tr><tr>");
        infoItem = DEV_INFO_HAS_CTR_DEV;
        devInfo = showInfo(infoType, infoItem, "DevHasCTR");
        infoText.append(devInfo  + "</tr><tr>");
        infoItem = DEV_INFO_HAS_TMR_DEV;
        devInfo = showInfo(infoType, infoItem, "DevHasTMR");
        infoText.append(devInfo  + "</tr><tr>");
        infoItem = DEV_INFO_HAS_DAQI_DEV;
        devInfo = showInfo(infoType, infoItem, "DevHasDAQI");
        infoText.append(devInfo  + "</tr><tr>");
        infoItem = DEV_INFO_HAS_DAQO_DEV;
        devInfo = showInfo(infoType, infoItem, "DevHasDAQO");
        infoText.append(devInfo  + "</tr><tr>");
        infoItem = DEV_INFO_DAQ_EVENT_TYPES;
        devInfo = showInfo(infoType, infoItem, "DevEvents");
        infoText.append(devInfo  + "</tr><tr>");
        infoItem = DEV_INFO_MEM_REGIONS;
        devInfo = showInfo(infoType, infoItem, "MemRegions");
        infoText.append(devInfo  + "</tr><tr>");
        ui->teShowValues->setHtml(infoText);
        break;
    case TYPE_AI_INFO:
        infoItem = AI_INFO_MIN_SCAN_RATE;
        devInfo = showInfoDbl(infoType, infoItem, "AIScan min");
        infoText.append(devInfo  + "</tr><tr>");
        infoItem = AI_INFO_MAX_SCAN_RATE;
        devInfo = showInfoDbl(infoType, infoItem, "AIScan max");
        infoText.append(devInfo  + "</tr><tr>");
        infoItem = AI_INFO_MAX_THROUGHPUT;
        devInfo = showInfoDbl(infoType, infoItem, "AIScan throughput");
        infoText.append(devInfo  + "</tr><tr>");
        infoItem = AI_INFO_MAX_BURST_RATE;
        devInfo = showInfoDbl(infoType, infoItem, "Max Burstrate");
        infoText.append(devInfo  + "</tr><tr>");
        infoItem = AI_INFO_MAX_BURST_THROUGHPUT;
        devInfo = showInfoDbl(infoType, infoItem, "Burst throughput");
        infoText.append(devInfo  + "</tr><tr>");

        infoItem = AI_INFO_RESOLUTION;
        devInfo = showInfo(infoType, infoItem, "AI Resolution");
        infoText.append(devInfo  + "</tr><tr>");
        infoItem = AI_INFO_NUM_CHANS;
        devInfo = showInfo(infoType, infoItem, "AI Num Chans");
        infoText.append(devInfo  + "</tr><tr>");
        infoItem = AI_INFO_NUM_CHANS_BY_MODE;
        showIndex = true;
        devInfo = showInfo(infoType, infoItem, "AI Num Chans");
        infoText.append(devInfo  + "</tr><tr>");
        infoItem = AI_INFO_NUM_CHANS_BY_TYPE;
        devInfo = showInfo(infoType, infoItem, "AI Num Chans");
        infoText.append(devInfo  + "</tr><tr>");
        showIndex = false;
        infoItem = AI_INFO_CHAN_TYPES;
        devInfo = showInfo(infoType, infoItem, "AI ChanTypes");
        infoText.append(devInfo  + "</tr><tr>");
        showIndex = false;
        infoItem = AI_INFO_SCAN_OPTIONS;
        devInfo = showInfo(infoType, infoItem, "AIScan Options");
        infoText.append(devInfo  + "</tr><tr>");
        infoItem = AI_INFO_HAS_PACER;
        devInfo = showInfo(infoType, infoItem, "AI Has Pacer");
        infoText.append(devInfo  + "</tr><tr>");
        infoItem = AI_INFO_NUM_DIFF_RANGES;
        devInfo = showInfo(infoType, infoItem, "AI Diff Ranges");
        infoText.append(devInfo  + "</tr><tr>");
        infoItem = AI_INFO_NUM_SE_RANGES;
        devInfo = showInfo(infoType, infoItem, "AI SE Ranges");
        infoText.append(devInfo  + "</tr><tr>");
        infoItem = AI_INFO_DIFF_RANGE;
        showIndex = true;
        devInfo = showInfo(infoType, infoItem, "AI Diff Range");
        infoText.append(devInfo  + "</tr><tr>");
        infoItem = AI_INFO_SE_RANGE;
        devInfo = showInfo(infoType, infoItem, "AI SE Range");
        infoText.append(devInfo  + "</tr><tr>");
        showIndex = false;
        //infoItem = AI_INFO_NUM_TRIG_TYPES;
        //devInfo = showInfo(infoType, infoItem, "AI Trig Types");
        infoItem = AI_INFO_TRIG_TYPES;
        devInfo = showInfo(infoType, infoItem, "AI Trig Type");
        infoText.append(devInfo  + "</tr><tr>");
        showIndex = true;
        infoItem = AI_INFO_MAX_QUEUE_LENGTH_BY_MODE;
        devInfo = showInfo(infoType, infoItem, "AI Max Queue");
        infoText.append(devInfo  + "</tr><tr>");
        showIndex = false;
        infoItem = AI_INFO_QUEUE_TYPES;
        devInfo = showInfo(infoType, infoItem, "AI Queue Type");
        infoText.append(devInfo  + "</tr><tr>");
        infoItem = AI_INFO_QUEUE_LIMITS;
        devInfo = showInfo(infoType, infoItem, "AI Queue Limits");
        infoText.append(devInfo  + "</tr><tr>");
        infoItem = AI_INFO_FIFO_SIZE;
        devInfo = showInfo(infoType, infoItem, "AI FIFO Size");
        infoText.append(devInfo  + "</tr><tr>");
        infoItem = AI_INFO_IEPE_SUPPORTED;
        devInfo = showInfo(infoType, infoItem, "AI IEPE Support");
        infoText.append(devInfo  + "</tr><tr>");
        ui->teShowValues->setHtml(infoText);
        break;
    case TYPE_AO_INFO:
        infoItem = AO_INFO_MIN_SCAN_RATE;
        devInfo = showInfoDbl(infoType, infoItem, "AOScan min");
        infoText.append(devInfo  + "</tr><tr>");
        infoItem = AO_INFO_MAX_SCAN_RATE;
        devInfo = showInfoDbl(infoType, infoItem, "AOScan max");
        infoText.append(devInfo  + "</tr><tr>");
        infoItem = AO_INFO_MAX_THROUGHPUT;
        devInfo = showInfoDbl(infoType, infoItem, "AOScan throughput");
        infoText.append(devInfo  + "</tr><tr>");

        infoItem = AO_INFO_RESOLUTION;
        devInfo = showInfo(infoType, infoItem, "AO Resolution");
        infoText.append(devInfo  + "</tr><tr>");
        infoItem = AO_INFO_NUM_CHANS;
        devInfo = showInfo(infoType, infoItem, "AO Num Chans");
        infoText.append(devInfo  + "</tr><tr>");
        infoItem = AO_INFO_SCAN_OPTIONS;
        devInfo = showInfo(infoType, infoItem, "AO Scan Options");
        infoText.append(devInfo  + "</tr><tr>");
        infoItem = AO_INFO_HAS_PACER;
        devInfo = showInfo(infoType, infoItem, "AO Has Pacer");
        infoText.append(devInfo  + "</tr><tr>");
        infoItem = AO_INFO_NUM_RANGES;
        devInfo = showInfo(infoType, infoItem, "AO Ranges");
        infoText.append(devInfo  + "</tr><tr>");
        infoItem = AO_INFO_RANGE;
        showIndex = true;
        devInfo = showInfo(infoType, infoItem, "AO Range");
        infoText.append(devInfo  + "</tr><tr>");
        showIndex = false;
        //infoItem = AO_INFO_NUM_TRIG_TYPES;
        //devInfo = showInfo(infoType, infoItem, "AO Trig Types");
        infoItem = AO_INFO_TRIG_TYPES;
        showIndex = true;
        devInfo = showInfo(infoType, infoItem, "AO Trig Types");
        infoText.append(devInfo  + "</tr><tr>");
        showIndex = false;
        infoItem = AO_INFO_FIFO_SIZE;
        devInfo = showInfo(infoType, infoItem, "AO FIFO Size");
        infoText.append(devInfo  + "</tr><tr>");
        ui->teShowValues->setHtml(infoText);
        break;
    case TYPE_DIO_INFO:
        showIndex = true;
        infoItem = DIO_INFO_MIN_SCAN_RATE;
        devInfo = showInfoDbl(infoType, infoItem, "DioScan min");
        infoText.append(devInfo  + "</tr><tr>");
        infoItem = DIO_INFO_MAX_SCAN_RATE;
        devInfo = showInfoDbl(infoType, infoItem, "DioScan max");
        infoText.append(devInfo  + "</tr><tr>");
        infoItem = DIO_INFO_MAX_THROUGHPUT;
        devInfo = showInfoDbl(infoType, infoItem, "DioScan throughput");
        infoText.append(devInfo  + "</tr><tr>");
        showIndex = false;

        infoItem = DIO_INFO_NUM_PORTS;
        devInfo = showInfo(infoType, infoItem, "Num Ports");
        infoText.append(devInfo  + "</tr><tr>");
        infoItem = DIO_INFO_PORT_TYPE;
        showIndex = true;
        devInfo = showInfo(infoType, infoItem, "Port Type");
        infoText.append(devInfo  + "</tr><tr>");
        infoItem = DIO_INFO_PORT_IO_TYPE;
        devInfo = showInfo(infoType, infoItem, "Port IO Type");
        infoText.append(devInfo  + "</tr><tr>");
        infoItem = DIO_INFO_NUM_BITS;
        devInfo = showInfo(infoType, infoItem, "Num Bits");
        infoText.append(devInfo  + "</tr><tr>");
        showIndex = false;
        infoItem = DIO_INFO_HAS_PACER;
        devInfo = showInfo(infoType, infoItem, "Has Pacer");
        infoText.append(devInfo  + "</tr><tr>");
        infoItem = DIO_INFO_SCAN_OPTIONS;
        devInfo = showInfo(infoType, infoItem, "Scan Options");
        infoText.append(devInfo  + "</tr><tr>");
        infoItem = DIO_INFO_TRIG_TYPES;
        devInfo = showInfo(infoType, infoItem, "Trig Types");
        infoText.append(devInfo  + "</tr><tr>");
        infoItem = DIO_INFO_FIFO_SIZE;
        devInfo = showInfo(infoType, infoItem, "Fifo Size");
        infoText.append(devInfo  + "</tr><tr>");
        ui->teShowValues->setHtml(infoText);
        break;
    case TYPE_CTR_INFO:
        infoItem = CTR_INFO_MIN_SCAN_RATE;
        devInfo = showInfoDbl(infoType, infoItem, "CtrScan min");
        infoText.append(devInfo  + "</tr><tr>");
        infoItem = CTR_INFO_MAX_SCAN_RATE;
        devInfo = showInfoDbl(infoType, infoItem, "CtrScan max");
        infoText.append(devInfo  + "</tr><tr>");
        infoItem = CTR_INFO_MAX_THROUGHPUT;
        devInfo = showInfoDbl(infoType, infoItem, "CtrScan throughput");
        infoText.append(devInfo  + "</tr><tr>");

        infoItem = CTR_INFO_NUM_CTRS;
        devInfo = showInfo(infoType, infoItem, "Num Ctrs");
        infoText.append(devInfo  + "</tr><tr>");
        infoItem = CTR_INFO_MEASUREMENT_TYPES;
        showIndex = true;
        devInfo = showInfo(infoType, infoItem, "Ctr Types");
        infoText.append(devInfo  + "</tr><tr>");
        infoItem = CTR_INFO_MEASUREMENT_MODES;
        devInfo = showInfo(infoType, infoItem, "Ctr Modes");
        infoText.append(devInfo  + "</tr><tr>");
        showIndex = false;
        infoItem = CTR_INFO_REGISTER_TYPES;
        devInfo = showInfo(infoType, infoItem, "Ctr Reg Types");
        infoText.append(devInfo  + "</tr><tr>");
        infoItem = CTR_INFO_RESOLUTION;
        devInfo = showInfo(infoType, infoItem, "Ctr Resolution");
        infoText.append(devInfo  + "</tr><tr>");
        infoItem = CTR_INFO_HAS_PACER;
        devInfo = showInfo(infoType, infoItem, "Ctr Has Pacer");
        infoText.append(devInfo  + "</tr><tr>");
        infoItem = CTR_INFO_SCAN_OPTIONS;
        devInfo = showInfo(infoType, infoItem, "Scan Options");
        infoText.append(devInfo  + "</tr><tr>");
        infoItem = CTR_INFO_TRIG_TYPES;
        devInfo = showInfo(infoType, infoItem, "Trig Types");
        infoText.append(devInfo  + "</tr><tr>");
        infoItem = CTR_INFO_FIFO_SIZE;
        devInfo = showInfo(infoType, infoItem, "Fifo Size");
        infoText.append(devInfo  + "</tr><tr>");
        ui->teShowValues->setHtml(infoText);
        break;
    case TYPE_TMR_INFO:
        infoItem = TMR_INFO_MIN_FREQ;
        devInfo = showInfoDbl(infoType, infoItem, "Tmr min frequency");
        infoText.append(devInfo  + "</tr><tr>");
        infoItem = TMR_INFO_MAX_FREQ;
        devInfo = showInfoDbl(infoType, infoItem, "Tmr max frequency");
        infoText.append(devInfo  + "</tr><tr>");

        infoItem = TMR_INFO_NUM_TMRS;
        devInfo = showInfo(infoType, infoItem, "Num Tmrs");
        infoText.append(devInfo  + "</tr><tr>");
        showIndex = true;
        infoItem = TMR_INFO_TYPE;
        devInfo = showInfo(infoType, infoItem, "Tmr Type");
        infoText.append(devInfo  + "</tr><tr>");
        ui->teShowValues->setHtml(infoText);
        showIndex = false;
        break;
    case TYPE_DAQI_INFO:
        infoItem = DAQI_INFO_MIN_SCAN_RATE;
        devInfo = showInfoDbl(infoType, infoItem, "DaqIScan min");
        infoText.append(devInfo  + "</tr><tr>");
        infoItem = DAQI_INFO_MAX_SCAN_RATE;
        devInfo = showInfoDbl(infoType, infoItem, "DaqIScan max");
        infoText.append(devInfo  + "</tr><tr>");
        infoItem = DAQI_INFO_MAX_THROUGHPUT;
        devInfo = showInfoDbl(infoType, infoItem, "DaqIScan throughput");
        infoText.append(devInfo  + "</tr><tr>");

        infoItem = DAQI_INFO_CHAN_TYPES;
        devInfo = showInfo(infoType, infoItem, "DaqIn Chan Types");
        infoText.append(devInfo  + "</tr><tr>");
        infoItem = DAQI_INFO_TRIG_TYPES;
        devInfo = showInfo(infoType, infoItem, "DaqIn Trig Types");
        infoText.append(devInfo  + "</tr><tr>");
        infoItem = DAQI_INFO_SCAN_OPTIONS;
        devInfo = showInfo(infoType, infoItem, "Scan Options");
        infoText.append(devInfo  + "</tr><tr>");
        infoItem = DAQI_INFO_FIFO_SIZE;
        devInfo = showInfo(infoType, infoItem, "Fifo Size");
        infoText.append(devInfo  + "</tr><tr>");
        ui->teShowValues->setHtml(infoText);
        break;
    case TYPE_DAQO_INFO:
        infoItem = DAQO_INFO_MIN_SCAN_RATE;
        devInfo = showInfoDbl(infoType, infoItem, "DaqOScan min");
        infoText.append(devInfo  + "</tr><tr>");
        infoItem = DAQO_INFO_MAX_SCAN_RATE;
        devInfo = showInfoDbl(infoType, infoItem, "DaqOScan max");
        infoText.append(devInfo  + "</tr><tr>");
        infoItem = DAQO_INFO_MAX_THROUGHPUT;
        devInfo = showInfoDbl(infoType, infoItem, "DaqOScan throughput");
        infoText.append(devInfo  + "</tr><tr>");

        infoItem = DAQO_INFO_CHAN_TYPES;
        devInfo = showInfo(infoType, infoItem, "DaqOut Chan Types");
        infoText.append(devInfo  + "</tr><tr>");
        infoItem = DAQO_INFO_TRIG_TYPES;
        devInfo = showInfo(infoType, infoItem, "DaqOut Trig Types");
        infoText.append(devInfo  + "</tr><tr>");
        infoItem = DAQO_INFO_SCAN_OPTIONS;
        devInfo = showInfo(infoType, infoItem, "Scan Options");
        infoText.append(devInfo  + "</tr><tr>");
        infoItem = DAQO_INFO_FIFO_SIZE;
        devInfo = showInfo(infoType, infoItem, "Fifo Size");
        infoText.append(devInfo  + "</tr><tr>");
        ui->teShowValues->setHtml(infoText);
        break;
    case TYPE_MEM_INFO:
        readMem();
        break;
    default:
        break;
    }
}

void subWidget::readConfig()
{
    int configItem, configType;
    QString verName, devID, configText;
    QString devConfig, str;
    bool noCfgExists;

    configType = ui->cmbInfoType->currentData(Qt::UserRole).toInt();
    showIndex = false;

    ui->teShowValues->clear();
    configText = "<style> th, td { padding-right: 10px;}</style><tr>";

    noCfgExists = false;
    switch (configType) {
    case TYPE_UL_INFO:
        configItem = UL_CFG_USB_XFER_PRIORITY;
        devConfig = showConfig(configType, configItem, "UL USB Xfer Priority");
        configText.append(devConfig + "</tr><tr>");
        ui->teShowValues->setHtml(configText);
        break;
    case TYPE_DEV_INFO:
        devID = "<td>Device ID: " + str.setNum(mDaqDeviceHandle)
                + "<td></td><td></td>" + mDevUID + "</td>";
        configText.append(devID + "</tr><tr>");
        configItem = DEV_VER_FW_MAIN;
        showIndex = true;
        verName = "FW Version";
        for(int verType = 0; verType < 3; verType++){
            if(verType == 1) verName = "FPGA Version";
            if(verType == 2) verName = "Radio Version";
            devConfig = showConfigStr(configType, verType, verName);
            configText.append(devConfig + "</tr><tr>");
        }
        ui->teShowValues->setHtml(configText);
        break;
    case TYPE_AI_INFO:
        configItem = AI_CFG_CHAN_TYPE;
        showIndex = true;
        devConfig = showConfig(configType, configItem, "AI Chan Type");
        configText.append(devConfig + "</tr><tr>");
        configItem = AI_CFG_CHAN_TC_TYPE;
        devConfig = showConfig(configType, configItem, "AI Chan TC Type");
        configText.append(devConfig + "</tr><tr>");
        configItem = AI_CFG_SCAN_CHAN_TEMP_UNIT;
        devConfig = showConfig(configType, configItem, "AI Chan Temp Unit");
        configText.append(devConfig + "</tr><tr>");
        showIndex = false;
        configItem = AI_CFG_SCAN_TEMP_UNIT;
        devConfig = showConfig(configType, configItem, "AI Temp Unit");
        configText.append(devConfig + "</tr><tr>");
        configItem = AI_CFG_ADC_TIMING_MODE;
        devConfig = showConfig(configType, configItem, "AI ADC Timing Mode");
        configText.append(devConfig + "</tr><tr>");
        configItem = AI_CFG_AUTO_ZERO_MODE;
        devConfig = showConfig(configType, configItem, "AI Auto Zero Mode");
        configText.append(devConfig + "</tr><tr>");
        configItem = AI_CFG_CAL_DATE;
        devConfig = showConfig(configType, configItem, "AI Cal Date");
        configText.append(devConfig + "</tr><tr>");
        configItem = AI_CFG_CAL_DATE_STR;
        devConfig = showConfigStr(configType, configItem, "AI Cal String");
        configText.append(devConfig + "</tr><tr>");
        configItem = AI_CFG_CHAN_IEPE_MODE;
        showIndex = true;
        devConfig = showConfig(configType, configItem, "AI Chan IEPE Mode");
        configText.append(devConfig + "</tr><tr>");
        configItem = AI_CFG_CHAN_COUPLING_MODE;
        devConfig = showConfig(configType, configItem, "AI Chan Coupling Mode");
        configText.append(devConfig + "</tr><tr>");
        configItem = AI_CFG_CHAN_SENSOR_CONNECTION_TYPE;
        devConfig = showConfig(configType, configItem, "AI Sensor Connection");
        configText.append(devConfig + "</tr><tr>");
        configItem = AI_CFG_CHAN_OTD_MODE;
        devConfig = showConfig(configType, configItem, "AI OTD Mode");
        configText.append(devConfig + "</tr><tr>");
        configItem = AI_CFG_CHAN_SLOPE;
        devConfig = showConfigDbl(configType, configItem, "AI Chan Slope");
        configText.append(devConfig + "</tr><tr>");
        configItem = AI_CFG_CHAN_OFFSET;
        devConfig = showConfigDbl(configType, configItem, "AI Chan Offset");
        configText.append(devConfig + "</tr><tr>");
        configItem = AI_CFG_CHAN_SENSOR_SENSITIVITY;
        devConfig = showConfigDbl(configType, configItem, "AI Chan Snsr Snstvty");
        configText.append(devConfig + "</tr><tr>");
        configItem = AI_CFG_CHAN_DATA_RATE;
        devConfig = showConfigDbl(configType, configItem, "AI Chan Data Rate");
        configText.append(devConfig + "</tr><tr>");
        configItem = AI_CFG_CHAN_COEFS_STR;
        devConfig = showConfigStr(configType, configItem, "AI Chan Coeffs");
        configText.append(devConfig + "</tr><tr>");
        showIndex = false;
        ui->teShowValues->setHtml(configText);
        break;
    case TYPE_AO_INFO:
        configItem = AO_CFG_SYNC_MODE;
        showIndex = true;
        devConfig = showConfig(configType, configItem, "AO Sync Mode");
        configText.append(devConfig + "</tr><tr>");
        configItem = AO_CFG_CHAN_SENSE_MODE;
        devConfig = showConfig(configType, configItem, "AO Sense Mode");
        configText.append(devConfig + "</tr><tr>");
        ui->teShowValues->setHtml(configText);
        break;
    case TYPE_DIO_INFO:
        configItem = DIO_CFG_PORT_DIRECTION_MASK;
        showIndex = true;
        devConfig = showConfig(configType, configItem, "DIO Port Direction Mask");
        configText.append(devConfig + "</tr><tr>");
        configItem = DIO_CFG_PORT_INITIAL_OUTPUT_VAL;
        showIndex = true;
        devConfig = showConfig(configType, configItem, "DIO Port Output Val");
        configText.append(devConfig + "</tr><tr>");
        configItem = DIO_CFG_PORT_ISO_FILTER_MASK;
        showIndex = true;
        devConfig = showConfig(configType, configItem, "DIO Iso Filter Mask");
        configText.append(devConfig + "</tr><tr>");
        configItem = DIO_CFG_PORT_LOGIC;
        showIndex = true;
        devConfig = showConfig(configType, configItem, "DIO Port Output Logic");
        configText.append(devConfig + "</tr><tr>");
        ui->teShowValues->setHtml(configText);
        showIndex = false;
        break;
    case TYPE_CTR_INFO:
        //noCfgExists = true;
        configItem = CTR_CFG_REG;
        devConfig = showConfig(configType, configItem, "Ctr Config Reg");
        configText.append(devConfig + "</tr><tr>");
        ui->teShowValues->setHtml(configText);
        break;
    case TYPE_TMR_INFO:
        noCfgExists = true;
        break;
    case TYPE_DAQI_INFO:
        noCfgExists = true;
        break;
    case TYPE_DAQO_INFO:
        noCfgExists = true;
        break;
    default:
        break;
    }
    if (noCfgExists) {
        //ui->teShowValues->clear();
        configText.append("<td>There are no ConfigItems for this ConfigType</td></tr><tr>");
        ui->teShowValues->setHtml(configText);
    }
}

void subWidget::readMem()
{
    QString memInfo, infoText;

    infoText = "<style> th, td { padding-right: 12px;}</style><tr>";

    memInfo = showInfoMem(MR_CAL);
    infoText.append(memInfo + "</tr><tr>");
    memInfo = showInfoMem(MR_USER);
    infoText.append(memInfo + "</tr><tr>");
    memInfo = showInfoMem(MR_SETTINGS);
    infoText.append(memInfo + "</tr><tr>");
    memInfo = showInfoMem(MR_RESERVED0);
    infoText.append(memInfo + "</tr><tr>");
    ui->teShowValues->setHtml(infoText);
}

QString subWidget::showConfig(int configType, int configItem, QString showItem)
{
    long long configValue;
    UlConfigItem ulConfigItem;
    AiConfigItem aiConfigItem;
    AoConfigItem aoConfigItem;
    DioConfigItem dioConfigItem;
    CtrConfigItem ctrConfigItem;
    QString nameOfFunc, stringValue;
    QString argVals, str;
    QString funcStr, funcArgs;
    QTime t;
    QString sStartTime, textToAdd;
    unsigned int index;
    bool noConfigItem;

    index = ui->spnIndex->value();
    QString errDesc, errNumStr;

    errDesc = "";
    noConfigItem = false;
    qint64 mSec;
    sStartTime = t.currentTime().toString("hh:mm:ss.zzz") + "~";

    switch (configType){
    case TYPE_UL_INFO:
        ulConfigItem = (UlConfigItem)configItem;
        nameOfFunc = "ulGetConfig";
        err = ulGetConfig(ulConfigItem, index, &configValue);
        break;
    case TYPE_DEV_INFO:
        noConfigItem = true;
        break;
    case TYPE_AI_INFO:
        aiConfigItem = (AiConfigItem)configItem;
        nameOfFunc = "ulAIGetConfig";
        err = ulAIGetConfig(mDaqDeviceHandle, aiConfigItem, index, &configValue);
        if (aiConfigItem == AI_CFG_CAL_DATE) {
            mSec = (qint64)(configValue * 1000);
            QDateTime calTime = QDateTime::fromMSecsSinceEpoch(mSec);
            errDesc = " (" + calTime.toString() + ")";
        }
        if (aiConfigItem == AI_CFG_CHAN_TYPE)
            errDesc = " (" + getChanTypeNames((AiChanType)configValue) + ")";
        if (aiConfigItem == AI_CFG_CHAN_TC_TYPE) {
            errDesc = " (" + getTcTypeName((TcType)configValue) + ")";
        }
        if ((aiConfigItem == AI_CFG_SCAN_TEMP_UNIT) |
                (aiConfigItem == AI_CFG_SCAN_CHAN_TEMP_UNIT)) {
            errDesc = " (" + getTempUnitName((TempUnit)configValue) + ")";
        }
        if (aiConfigItem == AI_CFG_CHAN_COUPLING_MODE)
            errDesc = " (" + getChanCouplingModeName((CouplingMode)configValue) + ")";
        if (aiConfigItem == AI_CFG_CHAN_SENSOR_CONNECTION_TYPE)
            errDesc = " (" + getSensorConnectNames((SensorConnectionType)configValue) + ")";
        if (aiConfigItem == AI_CFG_CHAN_OTD_MODE)
            errDesc = " (" + getEnableDisableName((int)configValue) + ")";
        break;
    case TYPE_AO_INFO:
        aoConfigItem = (AoConfigItem)configItem;
        nameOfFunc = "ulAOGetConfig";
        err = ulAOGetConfig(mDaqDeviceHandle, aoConfigItem, index, &configValue);
        if (aoConfigItem == AO_CFG_SYNC_MODE)
            errDesc = " (" + getAoSyncModeName((AOutSyncMode)configValue) + ")";
        if(aoConfigItem == AO_CFG_CHAN_SENSE_MODE)
            errDesc = " (" + getEnableDisableName((int)configValue) + ")";
        break;
    case TYPE_DIO_INFO:
        dioConfigItem = (DioConfigItem)configItem;
        nameOfFunc = "ulDIOGetConfig";
        err = ulDIOGetConfig(mDaqDeviceHandle, dioConfigItem, index, &configValue);
        break;
    case TYPE_CTR_INFO:
        //noConfigItem = true;
        ctrConfigItem = (CtrConfigItem)configItem;
        nameOfFunc = "ulCtrGetConfig";
        err = ctrGetConfig(mDaqDeviceHandle, ctrConfigItem, index, configValue);
        break;
    case TYPE_TMR_INFO:
        noConfigItem = true;
        break;
    default:
        noConfigItem = true;
        break;
    }

    if (noConfigItem) {
        ui->teShowValues->clear();
        //ui->teShowValues->setText("No configItem exist for this configType");
        return "No configItem exist for this configType";
    }

    if(configType == TYPE_UL_INFO) {
        funcArgs = "(configItem, index, &configValue)\n";
        argVals = QStringLiteral("(%1, %2, %3)")
                .arg(configItem)
                .arg(index)
                .arg(configValue);
    } else {
        funcArgs = "(mDaqDeviceHandle, configItem, index, &configValue)\n";
        argVals = QStringLiteral("(%1, %2, %3, %4)")
                .arg(mDaqDeviceHandle)
                .arg(configItem)
                .arg(index)
                .arg(configValue);
    }

    funcStr = nameOfFunc + funcArgs + "Arg vals: " + argVals;
    if(err != ERR_NO_ERROR) {
        errNumStr = QString("[Error %1: ").arg(err);
        switch (err) {
        case ERR_BAD_DEV_TYPE:
            errDesc = errNumStr + "Function incompatible with device]";
            break;
        case ERR_BAD_AI_CHAN:
            errDesc = errNumStr + "Invalid Ain chan]";
            break;
        case ERR_CONFIG_NOT_SUPPORTED:
            errDesc = errNumStr + "Configuration not supported]";
            break;
        case ERR_BAD_CONFIG_VAL:
            errDesc = errNumStr + "Invalid configVal]";
            break;
        case ERR_BAD_AI_CHAN_TYPE:
            errDesc = errNumStr + "bad AI chan type]";
            break;
        case ERR_BAD_CONFIG_ITEM:
            errDesc = errNumStr + "Invalid configItem]";
            break;
        case ERR_BAD_PORT_INDEX:
            errDesc = errNumStr + "Invalid port index]";
            break;
        default:
            break;
        }
        if (errDesc.length()) {
            QString strIndex = "<td></td>";
            if (showIndex)
                strIndex = "<td>(" + str.setNum(index) + ")</td>";
            textToAdd = "<td>" + showItem + "</td>";
            textToAdd.append(strIndex);
            textToAdd.append("<td>" + errDesc + "</td>");
            mMainWindow->addFunction(sStartTime + funcStr + errDesc);
        } else {
            mMainWindow->setError(err, sStartTime + funcStr);
            //errStr = funcStr + "\n(ConfigItem = " + showItem + ")";
        }
        ui->lblStatus->setText(nameOfFunc + argVals + QString(" [Error = %1]").arg(err));
    } else {
        ui->lblStatus->setText(nameOfFunc + argVals);
        QString strIndex = "<td></td>";
        if (showIndex)
            strIndex = "<td>(" + str.setNum(index) + ")</td>";
        textToAdd = "<td>" + showItem + "</td>";
        textToAdd.append(strIndex);
        if(mShowHex)
            stringValue = QString("0x%1").arg(configValue, mHexResolution, 16, QLatin1Char('0'));
        else
            stringValue = QString("%1").arg(configValue);
        textToAdd.append("<td>" + stringValue + errDesc + "</td>");
        //textToAdd.append("<td>" + infoDesc + "</td>");
        mMainWindow->addFunction(sStartTime + funcStr);
    }
    return textToAdd;
}

QString subWidget::showConfigDbl(int configType, int configItem, QString showItem)
{
    double configValueDbl;
    AiConfigItemDbl aiConfigItem;
    QTime t;
    QString sStartTime, textToAdd;
    QString nameOfFunc, errNumStr, errDesc;
    QString argVals, str;
    QString funcStr, funcArgs;
    unsigned int index;

    index = ui->spnIndex->value();

    sStartTime = t.currentTime().toString("hh:mm:ss.zzz") + "~";
    switch (configType){
    case TYPE_AI_INFO:
        nameOfFunc = "ulGetConfigDbl";
        aiConfigItem = (AiConfigItemDbl)configItem;
        err = ulAIGetConfigDbl(mDaqDeviceHandle, aiConfigItem, index, &configValueDbl);
        break;
    default:
        break;
    }

    if(configType == TYPE_UL_INFO) {
        funcArgs = "(configItem, index, &configValueDbl)\n";
        argVals = QStringLiteral("Arg vals: (%1, %2, %3)")
                .arg(configItem)
                .arg(index)
                .arg(configValueDbl);
    } else {
        funcArgs = "(mDaqDeviceHandle, configItem, index, &configValueDbl)\n";
        argVals = QStringLiteral("Arg vals: (%1, %2, %3, %4)")
                .arg(mDaqDeviceHandle)
                .arg(configItem)
                .arg(index)
                .arg(configValueDbl);
    }
    funcStr = nameOfFunc + funcArgs + argVals;

    if(err != ERR_NO_ERROR) {
        errNumStr = QString("[Error %1: ").arg(err);
        switch (err) {
        case ERR_BAD_DEV_TYPE:
            errDesc = errNumStr + "Function incompatible with device]";
            break;
        case ERR_BAD_AI_CHAN:
            errDesc = errNumStr + "Invalid Ain chan]";
            break;
        case ERR_CONFIG_NOT_SUPPORTED:
            errDesc = errNumStr + "Configuration not supported]";
            break;
        case ERR_BAD_CONFIG_VAL:
            errDesc = errNumStr + "Invalid configVal]";
            break;
        case ERR_BAD_CONFIG_ITEM:
            errDesc = errNumStr + "Invalid configItem]";
            break;
        case ERR_BAD_PORT_INDEX:
            errDesc = errNumStr + "Invalid port index]";
            break;
        default:
            break;
        }
        if (errDesc.length()) {
            QString strIndex = "<td></td>";
            if (showIndex)
                strIndex = "<td>(" + str.setNum(index) + ")</td>";
            textToAdd = "<td>" + showItem + "</td>";
            textToAdd.append(strIndex);
            textToAdd.append("<td>" + errDesc + "</td>");
            mMainWindow->addFunction(sStartTime + funcStr + errDesc);
        } else {
            mMainWindow->setError(err, sStartTime + funcStr);
            //errStr = funcStr + "\n(ConfigItem = " + showItem + ")";
        }
    } else {
        QString strIndex = "<td></td>";
        if (showIndex)
            strIndex = "<td>(" + str.setNum(index) + ")</td>";
        textToAdd = "<td>" + showItem + "</td>";
        textToAdd.append(strIndex);
        textToAdd.append("<td>" + str.setNum(configValueDbl) + errDesc + "</td>");
        mMainWindow->addFunction(sStartTime + funcStr);
    }
    return textToAdd;
}

QString subWidget::showConfigStr(int configType, int configItem, QString showItem)
{
    unsigned int maxConfigLen;
    maxConfigLen = 50;
    char configValue[maxConfigLen];
    char *pConfigValue = configValue;

    DevConfigItemStr devConfigItem;
    AiConfigItemStr aiConfigItem;
    QString errStr, argVals, returnStr;
    QString funcStr, funcArgs;
    QString nameOfFunc, errNumStr, errDesc;
    QTime t;
    QString sStartTime, textToAdd, str;
    unsigned int index;

    index = ui->spnIndex->value();

    sStartTime = t.currentTime().toString("hh:mm:ss.zzz") + "~";
    switch (configType){
    case TYPE_DEV_INFO:
        devConfigItem = DEV_CFG_VER_STR;
        //devConfigItem = (DevConfigItemStr)configItem;
        nameOfFunc = "ulDevGetConfigStr";
        index = (unsigned int)configItem;
        err = ulDevGetConfigStr(mDaqDeviceHandle, devConfigItem, index, pConfigValue, &maxConfigLen);
        break;
    case TYPE_AI_INFO:
        nameOfFunc = "ulAiGetConfigStr";
        aiConfigItem = (AiConfigItemStr)configItem;
        err = ulAIGetConfigStr(mDaqDeviceHandle, aiConfigItem, index, pConfigValue, &maxConfigLen);
        break;
    default:
        break;
    }

    returnStr = QString::fromLocal8Bit(pConfigValue, (int)maxConfigLen);
    if(configType == TYPE_UL_INFO) {
        funcArgs = "(configItem, index, &configValue, maxConfigLen)\n";
        argVals = QStringLiteral("Arg vals: (%1, %2, {%3}, %4)")
                .arg(configItem)
                .arg(index)
                .arg(returnStr)
                .arg(maxConfigLen);
    } else {
        funcArgs = "(mDaqDeviceHandle, configItem, index, &configValue, maxConfigLen)\n";
        argVals = QStringLiteral("Arg vals: (%1, %2, %3, {%4}, %5)")
                .arg(mDaqDeviceHandle)
                .arg(configItem)
                .arg(index)
                .arg(returnStr)
                .arg(maxConfigLen);
    }

    funcStr = nameOfFunc + funcArgs + argVals;
    if(err != ERR_NO_ERROR) {
        errNumStr = QString("[Error %1: ").arg(err);
        switch (err) {
        case ERR_BAD_DEV_TYPE:
            errDesc = errNumStr + "Function incompatible with device]";
            break;
        case ERR_BAD_AI_CHAN:
            errDesc = errNumStr + "Invalid Ain chan]";
            break;
        case ERR_CONFIG_NOT_SUPPORTED:
            errDesc = errNumStr + "Configuration not supported]";
            break;
        case ERR_BAD_CONFIG_VAL:
            errDesc = errNumStr + "Invalid configVal]";
            break;
        case ERR_BAD_CONFIG_ITEM:
            errDesc = errNumStr + "Invalid configItem]";
            break;
        case ERR_BAD_PORT_INDEX:
            errDesc = errNumStr + "Invalid port index]";
            break;
        default:
            break;
        }
        if (errDesc.length()) {
            QString strIndex = "<td></td>";
            if (showIndex)
                strIndex = "<td>(" + str.setNum(index) + ")</td>";
            textToAdd = "<td>" + showItem + "</td>";
            textToAdd.append(strIndex);
            textToAdd.append("<td>" + errDesc + "</td>");
            mMainWindow->addFunction(sStartTime + funcStr + errDesc);
        } else {
            errStr = funcStr + "\n(ConfigItem = " + showItem + ")";
            mMainWindow->setError(err, sStartTime + funcStr);
        }
    } else {
        QString strIndex = "<td></td>";
        if (showIndex)
            strIndex = "<td>(" + str.setNum(index) + ")</td>";
        textToAdd = "<td>" + showItem + "</td>";
        textToAdd.append(strIndex);
        textToAdd.append("<td>" + QString("%1").arg(returnStr) + "</td>");
        mMainWindow->addFunction(sStartTime + funcStr);
    }
    return textToAdd;
}

QString subWidget::showInfo(int infoType, int infoItem, QString showItem)
{
    long long infoValue;
    DevInfoItem tInfoItem;
    AiInfoItem aiInfoItem;
    AoInfoItem aoInfoItem;
    DioInfoItem dioInfoItem;
    CtrInfoItem ctrInfoItem;
    TmrInfoItem tmrInfoItem;
    DaqIInfoItem daqInInfoItem;
    DaqOInfoItem daqOutInfoItem;
    QString textToAdd, str;
    QString errNumStr, argVals, indexName;
    QString nameOfFunc, funcStr, funcArgs;
    QTime t;
    QString sStartTime, stringValue;
    unsigned int index;
    bool indexInfo;

    indexInfo = false;
    infoValue = 0;
    index = ui->spnIndex->value();

    sStartTime = t.currentTime().toString("hh:mm:ss.zzz") + "~";
    switch (infoType){
    case TYPE_DEV_INFO:
        tInfoItem = (DevInfoItem)infoItem;
        nameOfFunc = "ulDevGetInfo";
        err = ulDevGetInfo(mDaqDeviceHandle, tInfoItem, index, &infoValue);
        break;
    case TYPE_AI_INFO:
        aiInfoItem = (AiInfoItem)infoItem;
        nameOfFunc = "ulAIGetInfo";
        err = ulAIGetInfo(mDaqDeviceHandle, aiInfoItem, index, &infoValue);
        if ((aiInfoItem == AI_INFO_NUM_CHANS_BY_MODE)
                | (aiInfoItem == AI_INFO_MAX_QUEUE_LENGTH_BY_MODE)) {
            indexName = getAiInputModeName((AiInputMode)index);
            indexInfo = true;
        }
        if (aiInfoItem == AI_INFO_NUM_CHANS_BY_TYPE) {
            indexName = getAiChanTypeName((AiChanType)index);
            indexInfo = true;
        }
        break;
    case TYPE_AO_INFO:
        aoInfoItem = (AoInfoItem)infoItem;
        nameOfFunc = "ulAOGetInfo";
        err = ulAOGetInfo(mDaqDeviceHandle, aoInfoItem, index, &infoValue);
        break;
    case TYPE_DIO_INFO:
        dioInfoItem = (DioInfoItem)infoItem;
        nameOfFunc = "ulDIOGetInfo";
        err = ulDIOGetInfo(mDaqDeviceHandle, dioInfoItem, index, &infoValue);
        if ((dioInfoItem == DIO_INFO_SCAN_OPTIONS)
                | (dioInfoItem == DIO_INFO_FIFO_SIZE)
                | (dioInfoItem == DIO_INFO_TRIG_TYPES)) {
            indexName = getDigitalDirection((DigitalDirection)index);
            indexInfo = true;
        }
        break;
    case TYPE_CTR_INFO:
        ctrInfoItem = (CtrInfoItem)infoItem;
        nameOfFunc = "ulCtrGetInfo";
        err = ulCtrGetInfo(mDaqDeviceHandle, ctrInfoItem, index, &infoValue);
        if (ctrInfoItem == CTR_INFO_MEASUREMENT_MODES) {
            indexName = getCtrMeasTypeNames((CounterMeasurementType)index);
            indexInfo = true;
        }
        break;
    case TYPE_TMR_INFO:
        tmrInfoItem = (TmrInfoItem)infoItem;
        nameOfFunc = "ulTmrGetInfo";
        err = ulTmrGetInfo(mDaqDeviceHandle, tmrInfoItem, index, &infoValue);
        break;
    case TYPE_DAQI_INFO:
        daqInInfoItem = (DaqIInfoItem)infoItem;
        nameOfFunc = "ulDaqIGetInfo";
        err = ulDaqIGetInfo(mDaqDeviceHandle, daqInInfoItem, index, &infoValue);
        break;
    case TYPE_DAQO_INFO:
        daqOutInfoItem = (DaqOInfoItem)infoItem;
        nameOfFunc = "ulDaqOGetInfo";
        err = ulDaqOGetInfo(mDaqDeviceHandle, daqOutInfoItem, index, &infoValue);
        break;
    default:
        break;
    }

    QString infoDesc = getInfoDescription(infoType, infoItem, infoValue);
    if (infoType == TYPE_UL_INFO) {
        funcArgs = "(infoItem, index, &infoValue)\n";
        argVals = QStringLiteral("Arg vals: (%1, %2, %3)")
                .arg(infoItem)
                .arg(index)
                .arg(infoValue);
    } else {
        funcArgs = "(mDaqDeviceHandle, infoItem, index, &infoValue)\n";
        argVals = QStringLiteral("(%1, %2, %3, %4)")
                .arg(mDaqDeviceHandle)
                .arg(infoItem)
                .arg(index)
                .arg(infoValue);
    }
    ui->lblStatus->setText(nameOfFunc + argVals + QString(" [Error = %1]").arg(err));

    QString errDesc = "";
    funcStr = nameOfFunc + funcArgs + "Arg vals: " + argVals;
    if(err != ERR_NO_ERROR) {
        errNumStr = QString("[Error %1: ").arg(err);
        switch (err) {
        case ERR_BAD_DEV_TYPE:
            errDesc = errNumStr + "Function incompatible with device]";
            break;
        case ERR_BAD_AI_CHAN:
            errDesc = errNumStr + "Invalid Ain chan]";
            break;
        case ERR_CONFIG_NOT_SUPPORTED:
            errDesc = errNumStr + "Configuration not supported]";
            break;
        case ERR_BAD_CONFIG_VAL:
            errDesc = errNumStr + "Invalid configVal]";
            break;
        case ERR_BAD_CONFIG_ITEM:
            errDesc = errNumStr + "Invalid configItem]";
            break;
        case ERR_BAD_PORT_INDEX:
            errDesc = errNumStr + "Invalid port index]";
            break;
        default:
            break;
        }
        if (errDesc.length()) {
            QString strIndex = "<td></td>";
            if (showIndex)
                strIndex = "<td>(" + str.setNum(index) + ")</td>";
            textToAdd = "<td>" + showItem + "</td>";
            textToAdd.append(strIndex);
            textToAdd.append("<td>" + errDesc + "</td>");
            mMainWindow->addFunction(sStartTime + funcStr + errDesc);
        } else {
            //errStr = funcStr + "\n(InfoItem = " + showItem + ")";
            mMainWindow->setError(err, sStartTime + funcStr);
        }
    } else {
        QString strIndex = "<td></td>";
        if (indexInfo)
            strIndex = "<td>(" + indexName + ")</td>";
        else if (showIndex)
            strIndex = "<td>(" + str.setNum(index) + ")</td>";
        textToAdd = "<td>" + showItem + "</td>";
        textToAdd.append(strIndex);
        if(mShowHex)
            stringValue = QString("0x%1").arg(infoValue, mHexResolution, 16, QLatin1Char('0'));
        else
            stringValue = QString("%1").arg(infoValue);
        textToAdd.append("<td>" + stringValue + "</td>");
        textToAdd.append("<td>" + infoDesc + "</td>");
        mMainWindow->addFunction(sStartTime + funcStr);
    }
    return textToAdd;
}

QString subWidget::showInfoDbl(int infoType, int infoItem, QString showItem)
{
    long long infoValue;
    double infoValueDbl;
    DevInfoItem tInfoItem;
    AiInfoItemDbl aiInfoItem;
    AoInfoItemDbl aoInfoItem;
    DioInfoItemDbl dioInfoItem;
    CtrInfoItemDbl ctrInfoItem;
    TmrInfoItemDbl tmrInfoItem;
    DaqIInfoItemDbl daqIInfoItem;
    DaqOInfoItemDbl daqOInfoItem;
    QString nameOfFunc, textToAdd;
    QString str, infoDesc, indexName;
    QString errStr, argVals, errNumStr;
    QString funcStr, funcArgs;
    QString sStartTime;
    QTime t;
    unsigned int index;
    bool indexInfo;

    indexInfo = false;
    infoValue = 0;
    index = ui->spnIndex->value();

    sStartTime = t.currentTime().toString("hh:mm:ss.zzz") + "~";
    switch (infoType){
    case TYPE_UL_INFO:
        break;
    case TYPE_DEV_INFO:
        tInfoItem = (DevInfoItem)infoItem;
        nameOfFunc = "ulDevGetInfo";
        err = ulDevGetInfo(mDaqDeviceHandle, tInfoItem, index, &infoValue);
        break;
    case TYPE_AI_INFO:
        aiInfoItem = (AiInfoItemDbl)infoItem;
        nameOfFunc = "ulAIGetInfoDbl";
        err = ulAIGetInfoDbl(mDaqDeviceHandle, aiInfoItem, index, &infoValueDbl);
        break;
    case TYPE_AO_INFO:
        aoInfoItem = (AoInfoItemDbl)infoItem;
        nameOfFunc = "ulAOGetInfoDbl";
        err = ulAOGetInfoDbl(mDaqDeviceHandle, aoInfoItem, index, &infoValueDbl);
        break;
    case TYPE_DIO_INFO:
        dioInfoItem = (DioInfoItemDbl)infoItem;
        nameOfFunc = "ulDIOGetInfoDbl";
        err = ulDIOGetInfoDbl(mDaqDeviceHandle, dioInfoItem, index, &infoValueDbl);
        if ((dioInfoItem == DIO_INFO_MAX_SCAN_RATE)
                | (dioInfoItem == DIO_INFO_MIN_SCAN_RATE)
                | (dioInfoItem == DIO_INFO_MAX_THROUGHPUT)) {
            indexName = getDigitalDirection((DigitalDirection)index);
            indexInfo = true;
        }
        break;
    case TYPE_CTR_INFO:
        ctrInfoItem = (CtrInfoItemDbl)infoItem;
        nameOfFunc = "ulCtrGetInfoDbl";
        err = ulCtrGetInfoDbl(mDaqDeviceHandle, ctrInfoItem, index, &infoValueDbl);
        break;
    case TYPE_TMR_INFO:
        tmrInfoItem = (TmrInfoItemDbl)infoItem;
        nameOfFunc = "ulTmrGetInfoDbl";
        err = ulTmrGetInfoDbl(mDaqDeviceHandle, tmrInfoItem, index, &infoValueDbl);
        break;
    case TYPE_DAQI_INFO:
        daqIInfoItem = (DaqIInfoItemDbl)infoItem;
        nameOfFunc = "ulDaqIGetInfoDbl";
        err = ulDaqIGetInfoDbl(mDaqDeviceHandle, daqIInfoItem, index, &infoValueDbl);
        break;
    case TYPE_DAQO_INFO:
        daqOInfoItem = (DaqOInfoItemDbl)infoItem;
        nameOfFunc = "ulDaqOGetInfoDbl";
        err = ulDaqOGetInfoDbl(mDaqDeviceHandle, daqOInfoItem, index, &infoValueDbl);
        break;
    default:
        break;
    }
    if(infoType == TYPE_UL_INFO) {
        funcArgs = "(infoItemDbl, index, &infoValueDbl)\n";
        argVals = QStringLiteral("Arg vals: (%1, %2, %3)")
                .arg(infoItem)
                .arg(index)
                .arg(infoValueDbl);
    } else {
        funcArgs = "(mDaqDeviceHandle, infoItemDbl, index, &infoValueDbl)\n";
        argVals = QStringLiteral("Arg vals: (%1, %2, %3, %4)")
                .arg(mDaqDeviceHandle)
                .arg(infoItem)
                .arg(index)
                .arg(infoValueDbl);
    }

    QString errDesc = "";
    infoDesc = getInfoDescription(infoType, infoItem, infoValue);

    funcStr = nameOfFunc + funcArgs + "Arg vals: " + argVals;
    if(err != ERR_NO_ERROR) {
        errNumStr = QString("[Error %1: ").arg(err);
        switch (err) {
        case ERR_BAD_DEV_TYPE:
            errDesc = errNumStr + "Function incompatible with device]";
            break;
        case ERR_BAD_AI_CHAN:
            errDesc = errNumStr + "Invalid Ain chan]";
            break;
        case ERR_CONFIG_NOT_SUPPORTED:
            errDesc = errNumStr + "Configuration not supported]";
            break;
        case ERR_BAD_CONFIG_VAL:
            errDesc = errNumStr + "Invalid configVal]";
            break;
        case ERR_BAD_CONFIG_ITEM:
            errDesc = errNumStr + "Invalid configItem]";
            break;
        case ERR_BAD_PORT_INDEX:
            errDesc = errNumStr + "Invalid port index]";
            break;
        default:
            break;
        }
        if (errDesc.length()) {
            QString strIndex = "<td></td>";
            if (showIndex)
                strIndex = "<td>(" + str.setNum(index) + ")</td>";
            textToAdd = "<td>" + showItem + "</td>";
            textToAdd.append(strIndex);
            textToAdd.append("<td>" + errDesc + "</td>");
            mMainWindow->addFunction(sStartTime + funcStr + errDesc);
        } else {
            errStr = funcStr + "\n(InfoItem = " + showItem + ")";
            /*ErrorDialog errDlg;
            errDlg.setModal(true);
            errDlg.setError(err, errStr);
            errDlg.exec();*/
            mMainWindow->setError(err, sStartTime + funcStr);
        }
    } else {
        QString strIndex = "<td></td>";
        if (indexInfo)
            strIndex = "<td>(" + indexName + ")</td>";
        else if (showIndex)
            strIndex = "<td>(" + str.setNum(index) + ")</td>";
        textToAdd = "<td>" + showItem + "</td>";
        textToAdd.append(strIndex);
        textToAdd.append("<td>" + str.setNum(infoValueDbl, 'g', 8) + "</td>");
        textToAdd.append("<td>" + infoDesc + "</td>");
        mMainWindow->addFunction(sStartTime + funcStr);
    }
    return textToAdd;
}

QString subWidget::showInfoStr(int infoType, int infoItem, QString showItem)
{
    unsigned int maxConfigLen;
    maxConfigLen = 5;
    char configValue[maxConfigLen];
    char *pInfoValue = configValue;

    UlInfoItemStr ulInfoItem;
    //DevConfigItemStr devInfoItem;
    //AiConfigItemStr aiInfoItem;
    QString nameOfFunc, funcArgs, argVals;
    QString textToAdd, str, returnStr;
    QTime t;
    QString sStartTime, funcStr;
    unsigned int index;
    //bool indexInfo;

    //indexInfo = false;
    index = ui->spnIndex->value();

    sStartTime = t.currentTime().toString("hh:mm:ss.zzz") + "~";
    switch (infoType){
    case TYPE_UL_INFO:
        nameOfFunc = "ulGetInfoStr";
        ulInfoItem = (UlInfoItemStr)infoItem;
        index = (uint)0;
        err = ulGetInfoStr(ulInfoItem, index, pInfoValue, &maxConfigLen);
        break;
    /*case TYPE_DEV_INFO:
        devInfoItem = DEV_CFG_VER_STR;
        nameOfFunc = "ulDevGetConfigStr";
        index = (unsigned int)infoItem;
        err = ulDevGetConfigStr(mDaqDeviceHandle, devInfoItem, index, pInfoValue, &maxConfigLen);
        break;
    case TYPE_AI_INFO:
        aiInfoItem = (AiConfigItemStr)infoItem;
        nameOfFunc = "ulAIGetConfigStr";
        err = ulAIGetConfigStr(mDaqDeviceHandle, aiInfoItem, index, pInfoValue, &maxConfigLen);
        break;*/
    default:
        break;
    }

    returnStr = QString::fromLocal8Bit(pInfoValue, (int)maxConfigLen);
    //if (infoType == TYPE_UL_INFO) {
    funcArgs = "(ulInfoItemStr, index, pInfoString, &maxConfigLen)\n";
    argVals = QStringLiteral("(%1, %2, %3, %4)")
            .arg(infoItem)
            .arg(index)
            .arg(returnStr)
            .arg(maxConfigLen);
    /*} else {
        funcArgs = "(mDaqDeviceHandle, infoItemStr, index, pInfoString, &maxConfigLen)";
        argVals = QStringLiteral("(%1, %2, %3, %4, %5)")
                .arg(mDaqDeviceHandle)
                .arg(infoItem)
                .arg(index)
                .arg(configValue)
                .arg(maxConfigLen);
    }*/

    funcStr = nameOfFunc + funcArgs + "Arg vals: " + argVals;
    if(err != ERR_NO_ERROR) {
        mMainWindow->setError(err, sStartTime + funcStr);
        ui->lblStatus->setText(nameOfFunc + argVals + QString(" [Error = %1]").arg(err));
    } else {
        ui->lblStatus->setText(nameOfFunc + argVals);
        QString strIndex = "<td></td>";
        if (showIndex)
            strIndex = "<td>(" + str.setNum(index) + ")</td>";
        textToAdd = "<td>" + showItem + "</td>";
        textToAdd.append(strIndex);
        textToAdd.append("<td>" + QString("%1").arg(returnStr) + "</td>");
        mMainWindow->addFunction(sStartTime + funcStr);
    }
    return textToAdd;
}

QString subWidget::showInfoMem(MemRegion memRegion)
{
    QString nameOfFunc, funcStr, funcArgs;
    QString showItem, argVals, textToAdd;
    QTime t;
    QString sStartTime, errNumStr;
    QString infoDesc, str, iSt;
    MemDescriptor memDescriptor;
    long long infoValue;

    switch (memRegion) {
    case MR_CAL:
        showItem = "Cal Region";
        break;
    case MR_USER:
        showItem = "User Region";
        break;
    case MR_SETTINGS:
        showItem = "Settings Region";
        break;
    case MR_RESERVED0:
        showItem = "Reserved Region0";
        break;
    default:
        break;
    }
    nameOfFunc = "ulMemGetInfo";
    funcArgs = "(mDaqDeviceHandle, memRegion, {accessTypes, address, size})\n";
    sStartTime = t.currentTime().toString("hh:mm:ss.zzz") + "~";
    err = ulMemGetInfo(mDaqDeviceHandle, memRegion, &memDescriptor);
    argVals = QStringLiteral("(%1, %2, {%3, %4, %5})")
            .arg(mDaqDeviceHandle)
            .arg(memRegion)
            .arg(memDescriptor.accessTypes)
            .arg(memDescriptor.address)
            .arg(memDescriptor.size);
    ui->lblStatus->setText(nameOfFunc + argVals + QString(" [Error = %1]").arg(err));

    QString errDesc = "";
    funcStr = nameOfFunc + funcArgs + "Arg vals: " + argVals;
    if(err != ERR_NO_ERROR) {
        errNumStr = QString("[Error %1: ").arg(err);
        switch (err) {
        case ERR_BAD_MEM_ADDRESS:
            errDesc = errNumStr + "Bad mem address]";
            break;
        case ERR_BAD_MEM_REGION:
            errDesc = errNumStr + "Bad mem region]";
            break;
        case ERR_BAD_MEM_TYPE:
            errDesc = errNumStr + "Bad mem type]";
            break;
        default:
            errDesc = "";
            break;
        }
        if (errDesc.length()) {
            textToAdd = "<td>" + showItem + "</td>";
            textToAdd.append("<td>" + errDesc + "</td>");
            mMainWindow->addFunction(sStartTime + funcStr);
        } else {
            mMainWindow->setError(err, sStartTime + funcStr);
        }
    } else {
        infoValue = (long long)memDescriptor.accessTypes;
        int infoItem = 0;
        infoDesc = "";
        infoDesc = getInfoDescription(TYPE_MEM_INFO, infoItem, infoValue);
        textToAdd = "<td>" + showItem + "</td>";
        textToAdd.append("<td>" + iSt.setNum(infoValue) + "(" + infoDesc + ")</td>");
        textToAdd.append("<td>address: " + str.setNum(memDescriptor.address) + "</td>");
        textToAdd.append("<td>size: " + str.setNum(memDescriptor.size) + "</td>");
        mMainWindow->addFunction(sStartTime + funcStr);
    }
    return textToAdd;
}

void subWidget::setConfiguration()
{
    UlConfigItem ulConfigItem;
    AiConfigItem aiConfigItem;
    AoConfigItem aoConfigItem;
    DioConfigItem dioConfigItem;
    AiConfigItemDbl aiConfigItemDbl;
    CtrConfigItem ctrConfigItem;

    QString showItem = "";
    QString valueText = "";
    QString funcStr, nameOfFunc, funcArgs, argVals;
    QTime t;
    QString sStartTime, textToAdd, userVal;

    unsigned int index;
    long long configValue;
    double configValueDbl = 0;
    int configItem, configType;
    bool noConfigItem, dblConfigItem, useHex, hexOK;

    useHex = false;
    userVal = ui->leSetValue->text();
    if (userVal.startsWith("0x")) {
        useHex = true;
        userVal = userVal.remove("0x");
        configValue = userVal.toLongLong(&hexOK, 16);
    } else
        configValue = userVal.toLongLong();

    configType = ui->cmbInfoType->currentData(Qt::UserRole).toInt();
    configItem = ui->cmbConfigItem->currentData(Qt::UserRole).toInt();
    configValueDbl = userVal.toDouble();
    index = ui->spnIndex->value();

    if(configItem == 0)
        return;

    showItem = ui->cmbConfigItem->currentText();

    noConfigItem = false;
    dblConfigItem = false;
    sStartTime = t.currentTime().toString("hh:mm:ss.zzz") + "~";
    switch (configType){
    case TYPE_UL_INFO:
        ulConfigItem = (UlConfigItem)configItem;
        nameOfFunc = "ulSetConfig";
        err = ulSetConfig(ulConfigItem, index, configValue);
        break;
    case TYPE_DEV_INFO:
        noConfigItem = true;
        return;
    case TYPE_AI_INFO:
        if (configItem < 1000) {
            aiConfigItem = (AiConfigItem)configItem;
            nameOfFunc = "ulAISetConfig";
            err = ulAISetConfig(mDaqDeviceHandle, aiConfigItem, index, configValue);
            if(aiConfigItem == AI_CFG_CHAN_TYPE)
                valueText = " [" + getChanTypeNames((AiChanType)configValue) + "]";
            if(aiConfigItem == AI_CFG_CHAN_TC_TYPE)
                valueText = " [type " + getTcTypeName((TcType)configValue) + "]";
            if(aiConfigItem == AI_CFG_CHAN_IEPE_MODE)
                valueText = " [" + getEnableDisableName((int)configValue) + "]";
            if(aiConfigItem == AI_CFG_CHAN_COUPLING_MODE)
                valueText = " [" + getChanCouplingModeName((CouplingMode)configValue) + "]";
        } else {
            dblConfigItem = true;
            aiConfigItemDbl = (AiConfigItemDbl)configItem;
            nameOfFunc = "ulAISetConfigDbl";
            err = ulAISetConfigDbl(mDaqDeviceHandle, aiConfigItemDbl, index, configValueDbl);
        }
        break;
    case TYPE_AO_INFO:
        aoConfigItem = (AoConfigItem)configItem;
        err = ulAOSetConfig(mDaqDeviceHandle, aoConfigItem, index, configValue);
        if(aoConfigItem == AO_CFG_CHAN_SENSE_MODE)
            valueText = " [" + getEnableDisableName((int)configValue) + "]";
        break;
    case TYPE_DIO_INFO:
        dioConfigItem = (DioConfigItem)configItem;
        nameOfFunc = "ulDIOSetConfig";
        err = ulDIOSetConfig(mDaqDeviceHandle, dioConfigItem, index, configValue);
        break;
    case TYPE_CTR_INFO:
        ctrConfigItem = (CtrConfigItem)configItem;
        nameOfFunc = "ulCtrSetConfig";
        err = ulCtrSetConfig(mDaqDeviceHandle, ctrConfigItem, index, configValue);
        break;
    case TYPE_TMR_INFO:
        noConfigItem = true;
        break;
    default:
        noConfigItem = true;
        break;
    }

    if (noConfigItem) {
        ui->teShowValues->clear();
        ui->teShowValues->setText("No configItem exist for this configType");
        return;
    }

    if (configType == TYPE_UL_INFO) {
        funcArgs = "(configItem, index, configValue)\n";
        argVals = QStringLiteral("Arg vals: (%1, %2, %3)")
                .arg(configItem)
                .arg(index)
                .arg(configValue);
    } else if (dblConfigItem){
        funcArgs = "(mDaqDeviceHandle, configItem, index, configValue)\n";
        argVals = QStringLiteral("Arg vals: (%1, %2, %3, %4)")
                .arg(mDaqDeviceHandle)
                .arg(configItem)
                .arg(index)
                .arg(configValueDbl);
    } else {
        funcArgs = "(mDaqDeviceHandle, configItem, index, configValue)\n";
        argVals = QStringLiteral("Arg vals: (%1, %2, %3, %4)")
                .arg(mDaqDeviceHandle)
                .arg(configItem)
                .arg(index)
                .arg(configValue);
    }

    ui->lblStatus->setText(nameOfFunc + argVals + QString(" [Error = %1]").arg(err));
    funcStr = nameOfFunc + funcArgs + "Arg vals: " + argVals;
    if (err != ERR_NO_ERROR) {
        //funcStr = nameOfFunc + funcArgs + argVals;
        mMainWindow->setError(err, sStartTime + funcStr);
    } else {
        mMainWindow->addFunction(sStartTime + funcStr);
        if (dblConfigItem)
            textToAdd = QString("%1 = %2")
                    .arg(showItem)
                    .arg(configValueDbl);
        else
            if (useHex)
                textToAdd = QString("%1 = 0x%2").arg(showItem)
                        .arg(configValue, 4, 16, QLatin1Char('0'));
            else
                textToAdd = QString("%1 = %2").arg(showItem).arg(configValue);
        if(showIndex)
            textToAdd += QString(" (index %1)").arg(index) + valueText;
        else
            textToAdd += valueText;
        ui->teShowValues->append(textToAdd);
    }
}

void subWidget::setMiscFunction()
{
    int flashCount;
    QString nameOfFunc;
    QString funcStr, argVals, funcArgs;
    ScanStatus status;
    TmrStatus tmrStatus;
    struct TransferStatus xferStatus;
    unsigned long long currentTotalCount;
    unsigned long long currentScanCount;
    long long curIndex;
    QTime t;
    QString sStartTime;

    flashCount = 0;
    tmrStatus = TMRS_IDLE;
    nameOfFunc = ui->cmbInfoType->currentText();

    sStartTime = t.currentTime().toString("hh:mm:ss.zzz") + "~";
    if(nameOfFunc == "ulFlashLED") {
        flashCount = ui->spnIndex->value();
        err = ulFlashLed(mDaqDeviceHandle, flashCount);
    } else if(nameOfFunc == "ulTmrPulseOutStatus") {
        flashCount = ui->spnIndex->value();
        err = ulTmrPulseOutStatus(mDaqDeviceHandle, flashCount, &tmrStatus);
    } else if(nameOfFunc == "ulAInScanStatus") {
        err = ulAInScanStatus(mDaqDeviceHandle, &status, &xferStatus);
    } else if(nameOfFunc == "ulAOutScanStatus") {
        err = ulAOutScanStatus(mDaqDeviceHandle, &status, &xferStatus);
    } else if(nameOfFunc == "ulDInScanStatus") {
        err = ulDInScanStatus(mDaqDeviceHandle, &status, &xferStatus);
    } else if(nameOfFunc == "ulDOutScanStatus") {
        err = ulDOutScanStatus(mDaqDeviceHandle, &status, &xferStatus);
    } else if(nameOfFunc == "ulCInScanStatus") {
        err = ulCInScanStatus(mDaqDeviceHandle, &status, &xferStatus);
    } else if(nameOfFunc == "ulDaqOutScanStatus") {
        err = ulDaqOutScanStatus(mDaqDeviceHandle, &status, &xferStatus);
    } else if(nameOfFunc == "ulDaqInScanStatus") {
        err = ulDaqInScanStatus(mDaqDeviceHandle, &status, &xferStatus);
    } else if(nameOfFunc == "ulAInScanStop") {
        err = ulAInScanStop(mDaqDeviceHandle);
    } else if(nameOfFunc == "ulAOutScanStop") {
        err = ulAOutScanStop(mDaqDeviceHandle);
    } else if(nameOfFunc == "ulDInScanStop") {
        err = ulDInScanStop(mDaqDeviceHandle);
    } else if(nameOfFunc == "ulDOutScanStop") {
        err = ulDOutScanStop(mDaqDeviceHandle);
    } else if(nameOfFunc == "ulCInScanStop") {
        err = ulCInScanStop(mDaqDeviceHandle);
    } else if(nameOfFunc == "ulDaqInScanStop") {
        err = ulDaqInScanStop(mDaqDeviceHandle);
    } else if(nameOfFunc == "ulDaqOutScanStop") {
        err = ulDaqOutScanStop(mDaqDeviceHandle);
    }

    curIndex = xferStatus.currentIndex;
    currentScanCount = xferStatus.currentScanCount;
    currentTotalCount = xferStatus.currentTotalCount;
    if(nameOfFunc == "ulFlashLED") {
        funcArgs = "(mDaqDeviceHandle, flashCount)\n";
        argVals = QString("(%1, %2)")
                .arg(mDaqDeviceHandle)
                .arg(flashCount);
    } else if(nameOfFunc == "ulTmrPulseOutStatus") {
        funcArgs = "(mDaqDeviceHandle, timerNum, &status)\n";
        argVals = QStringLiteral("(%1, %2, %3)")
                .arg(mDaqDeviceHandle)
                .arg(flashCount)
                .arg(tmrStatus);
    } else if((nameOfFunc == "ulAInScanStatus")
              || (nameOfFunc == "ulAOutScanStatus")
              || (nameOfFunc == "ulDInScanStatus")
              || (nameOfFunc == "ulDOutScanStatus")
              || (nameOfFunc == "ulCInScanStatus")
              || (nameOfFunc == "ulDaqOutScanStatus")
              || (nameOfFunc == "ulDaqInScanStatus")) {
        funcArgs = "(mDaqDeviceHandle, status, &currentScanCount, currentTotalCount, curIndex)\n";
        argVals = QStringLiteral("(%1, %2, %3, %4, %5)")
                .arg(mDaqDeviceHandle)
                .arg(status)
                .arg(currentScanCount)
                .arg(currentTotalCount)
                .arg(curIndex);
    } else if((nameOfFunc == "ulAInScanStop")
              || (nameOfFunc == "ulAOutScanStop")
              || (nameOfFunc == "ulDInScanStop")
              || (nameOfFunc == "ulDOutScanStop")
              || (nameOfFunc == "ulCInScanStop")
              || (nameOfFunc == "ulDaqOutScanStop")
              || (nameOfFunc == "ulDaqInScanStop")) {
        funcArgs = "(mDaqDeviceHandle)\n";
        argVals = QString("(%1)")
                .arg(mDaqDeviceHandle);
    }

    funcStr = nameOfFunc + funcArgs + "Arg vals: " + argVals;
    if (err != ERR_NO_ERROR) {
        mMainWindow->setError(err, sStartTime + funcStr);
        ui->lblStatus->setText(nameOfFunc + argVals + QString(" [Error = %1]").arg(err));
    } else {
        mMainWindow->addFunction(sStartTime + funcStr);
        ui->lblStatus->setText(nameOfFunc + argVals);
    }

}

void subWidget::getErrorMessage()
{
    QString nameOfFunc;
    QString funcStr, argVals, funcArgs;
    char errMsg[ERR_MSG_LEN];

    funcArgs = "(curError, errMsg)\n";
    UlError curError, err;

    curError = (UlError)ui->spnIndex->value();
    err = ulGetErrMsg(curError, errMsg);
    argVals = QString("(%1, %2)")
            .arg(curError)
            .arg(errMsg);

    funcStr = nameOfFunc + argVals;
    ui->lblStatus->setText(funcStr + QString(" [Error = %1]").arg(err));
}

//stub slots for childwindow signals

void subWidget::initDeviceParams()
{
    return;
}

void subWidget::showPlotWindow(bool)
{
    return;
}

void subWidget::runEventSetup()
{
    return;
}

void subWidget::showQueueConfig()
{
    return;
}

void subWidget::showDataGen()
{
    return;
}

void subWidget::updateText(QString infoText)
{
    (void)infoText;
    return;
}

void subWidget::swStopScan()
{
    return;
}

#ifdef UL_1_20

UlError subWidget::ctrGetConfig(DaqDeviceHandle daqDeviceHandle, CtrConfigItem configItem, unsigned int index, long long &configValue)
{
    err = ulCtrGetConfig(daqDeviceHandle, configItem, index, &configValue);
    return err;
}

UlError subWidget::ctrSetConfig(DaqDeviceHandle daqDeviceHandle, CtrConfigItem configItem, unsigned int index, long long configValue)
{
    err = ulCtrSetConfig(daqDeviceHandle, configItem, index, configValue);
    return err;
}

#else

UlError subWidget::ctrGetConfig(DaqDeviceHandle daqDeviceHandle, CtrConfigItem configItem, unsigned int index, long long *configValue)
{
    (void)daqDeviceHandle;
    (void)configItem;
    (void)index;
    (void)configValue;
    return ERR_BAD_DEV_TYPE;
}

UlError subWidget::ctrSetConfig(DaqDeviceHandle daqDeviceHandle, CtrConfigItem configItem, unsigned int index, long long configValue)
{
    (void)daqDeviceHandle;
    (void)configItem;
    (void)index;
    (void)configValue;
    return ERR_BAD_DEV_TYPE;
}

#endif
