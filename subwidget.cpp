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

    ui->teShowValues->setFont(QFont ("Courier", 8));
    ui->teShowValues->setStyleSheet("QTextEdit { background-color : white; color : blue; }" );
    ui->lblStatus->setStyleSheet("QLabel { background-color : white; color : blue; }" );
    //connect(ui->spnIndex, SIGNAL(valueChanged(int)), SLOT(runSelectedFunc()));
    connect(ui->cmdStop, SIGNAL(clicked(bool)), this, SLOT(onStopCmd()));
    connect(ui->cmbInfoType, SIGNAL(currentIndexChanged(int)), this, SLOT(setConfigItemsForType()));
    connect(ui->cmdSet, SIGNAL(clicked(bool)), this, SLOT(runSelectedFunc()));
    //connect(ui->cmbConfigItem, SIGNAL(currentIndexChanged(int)), this, SLOT(runSelectedFunc()));
    //connect(ui->spnIndex, SIGNAL(valueChanged(int)), this, SLOT(runSelectedFunc()));
}

subWidget::~subWidget()
{
    delete ui;
}

/*MainWindow *subWidget::getMainWindow()
{
    foreach (QWidget *w, QApplication::topLevelWidgets())
        if (QMainWindow* mainWin = qobject_cast<QMainWindow*>(w))
            return qobject_cast<MainWindow *>(mainWin);
    return nullptr;
}*/

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
    ChildWindow *parentWindow;
    bool configVisible;
    QString cmdLabel;
    parentWindow = qobject_cast<ChildWindow *>(this->parent());

    disconnect(ui->cmbInfoType, SIGNAL(currentIndexChanged(int)));
    disconnect(ui->cmdSet, SIGNAL(clicked(bool)));
    disconnect(ui->cmbConfigItem, SIGNAL(currentIndexChanged(int)));
    disconnect(ui->spnIndex, SIGNAL(valueChanged(int)));
    switch (mCurGroup) {
    case FUNC_GROUP_MISC:
        connect(ui->cmdSet, SIGNAL(clicked(bool)), this, SLOT(runSelectedFunc()));
        //connect(ui->cmbConfigItem, SIGNAL(currentIndexChanged(int)), this, SLOT(runSelectedFunc()));
        //connect(ui->spnIndex, SIGNAL(valueChanged(int)), this, SLOT(runSelectedFunc()));
        cmdLabel = "Go";
        break;
    case FUNC_GROUP_CONFIG:
        connect(ui->cmbInfoType, SIGNAL(currentIndexChanged(int)), this, SLOT(setConfigItemsForType()));
        connect(ui->cmdSet, SIGNAL(clicked(bool)), this, SLOT(setConfiguration()));
        connect(ui->cmbConfigItem, SIGNAL(currentIndexChanged(int)), this, SLOT(runSelectedFunc()));
        connect(ui->spnIndex, SIGNAL(valueChanged(int)), this, SLOT(runSelectedFunc()));
        configVisible = true;
        cmdLabel = "Set";
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
        default:
            break;
        }
        cmdSetVisible = true;
        userFrameVisible = true;
        break;
    case FUNC_GROUP_CONFIG:
        ui->cmbInfoType->clear();
        cmdLabel = "Set";
        configComboVisible = true;
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
            break;
        case UL_SET_CONFIG:
            mFuncName = "ulSetConfig";
            configComboVisible = true;
            //lineEditVisible = true;
            userFrameVisible = true;
            cmdSetVisible = true;
            ui->cmbInfoType->addItem("Set UL Config", TYPE_UL_INFO);
            ui->cmbInfoType->addItem("Set AI Config", TYPE_AI_INFO);
            ui->cmbInfoType->addItem("Set AO Config", TYPE_AO_INFO);
            ui->cmbInfoType->addItem("Set DIO Config", TYPE_DIO_INFO);
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
        ui->cmbConfigItem->addItem("Chan Temp Unit", AI_CFG_CHAN_TEMP_UNIT);
        ui->cmbConfigItem->addItem("Temp Unit", AI_CFG_TEMP_UNIT);
        ui->cmbConfigItem->addItem("Adc Timing Mode", AI_CFG_ADC_TIMING_MODE);
        ui->cmbConfigItem->addItem("Auto Zero Mode", AI_CFG_AUTO_ZERO_MODE);
        ui->cmbConfigItem->addItem("IEPE Mode", AI_CFG_CHAN_IEPE_MODE);
        ui->cmbConfigItem->addItem("Chan Coupling Mode", AI_CFG_CHAN_COUPLING_MODE);
        ui->cmbConfigItem->addItem("Auto Zero Mode", AI_CFG_CHAN_SLOPE);
        ui->cmbConfigItem->addItem("IEPE Mode", AI_CFG_CHAN_OFFSET);
        ui->cmbConfigItem->addItem("Chan Coupling Mode", AI_CFG_CHAN_SENSOR_SENSIVITY);
        break;
    case TYPE_DIO_INFO:
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
            setConfiguration();
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
        configItem = AI_CFG_CHAN_TEMP_UNIT;
        devConfig = showConfig(configType, configItem, "AI Chan Temp Unit");
        configText.append(devConfig + "</tr><tr>");
        showIndex = false;
        configItem = AI_CFG_TEMP_UNIT;
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
        configItem = AI_CFG_CHAN_IEPE_MODE;
        showIndex = true;
        devConfig = showConfig(configType, configItem, "AI Chan IEPE Mode");
        configText.append(devConfig + "</tr><tr>");
        configItem = AI_CFG_CHAN_COUPLING_MODE;
        devConfig = showConfig(configType, configItem, "AI Chan Coupling Mode");
        configItem = AI_CFG_CHAN_SENSOR_CONNECTION_TYPE;
        devConfig = showConfig(configType, configItem, "AI Sensor Connection");
        configText.append(devConfig + "</tr><tr>");
        showIndex = false;
        ui->teShowValues->setHtml(configText);
        break;
    case TYPE_AO_INFO:
        noCfgExists = true;
        break;
    case TYPE_DIO_INFO:
        configItem = DIO_CFG_PORT_DIRECTION_MASK;
        showIndex = true;
        devConfig = showConfig(configType, configItem, "DIO Port Direction Mask");
        configText.append(devConfig + "</tr><tr>");
        ui->teShowValues->setHtml(configText);
        showIndex = false;
        break;
    case TYPE_CTR_INFO:
        noCfgExists = true;
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
    ui->teShowValues->setHtml(infoText);
}

QString subWidget::showConfig(int configType, int configItem, QString showItem)
{
    long long configValue;
    UlConfigItem ulConfigItem;
    AiConfigItem aiConfigItem;
    DioConfigItem dioConfigItem;
    QString nameOfFunc, textToAdd;
    QString errStr, argVals, str;
    QString funcStr, funcArgs;
    unsigned int index;
    bool noConfigItem;

    index = ui->spnIndex->value();
    QString errDesc;

    errDesc = "";
    noConfigItem = false;
    qint64 mSec;
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
        if (aiConfigItem == AI_CFG_TEMP_UNIT) {
            errDesc = " (" + getTcTypeName((AiChanType)configValue) + ")";
        }
        err = ulAIGetConfig(mDaqDeviceHandle, aiConfigItem, index, &configValue);
        if (aiConfigItem == AI_CFG_CAL_DATE) {
            mSec = (qint64)(configValue * 1000);
            QDateTime calTime = QDateTime::fromMSecsSinceEpoch(mSec);
            errDesc = " (" + calTime.toString() + ")";
        }
        if (aiConfigItem == AI_CFG_CHAN_TC_TYPE) {
            errDesc = " (" + getTcTypeName((AiChanType)configValue) + ")";
        }
        break;
    case TYPE_AO_INFO:
        noConfigItem = true;
        break;
    case TYPE_DIO_INFO:
        dioConfigItem = (DioConfigItem)configItem;
        nameOfFunc = "ulDIOGetConfig";
        err = ulDIOGetConfig(mDaqDeviceHandle, dioConfigItem, index, &configValue);
        break;
    case TYPE_CTR_INFO:
        noConfigItem = true;
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

    if(!err==ERR_NO_ERROR) {
        switch (err) {
        case ERR_BAD_DEV_TYPE:
            errDesc = " [error: bad device type]";
            break;
        case ERR_BAD_AI_CHAN:
            errDesc = " [error: bad Ain chan]";
            break;
        case ERR_CONFIG_NOT_SUPPORTED:
            errDesc = " [error: not supported]";
            break;
        case ERR_BAD_CONFIG_VAL:
            errDesc = " [error: bad configVal]";
            break;
        case ERR_BAD_CONFIG_ITEM:
            errDesc = " [error: bad configItem]";
            break;
        default:
            break;
        }
        if (errDesc.length()) {
            //textToAdd = QString("%1 %2")
            //        .arg(showItem)
            //        .arg(errDesc);
            //if(showIndex) textToAdd += QString(" (%1)").arg(index);
            //ui->teShowValues->append(textToAdd);
            QString strIndex = "<td></td>";
            if (showIndex)
                strIndex = "<td>(" + str.setNum(index) + ")</td>";
            textToAdd = "<td>" + showItem + "</td>";
            textToAdd.append(strIndex);
            textToAdd.append("<td>" + errDesc + "</td>");
        } else {
            funcStr = nameOfFunc + funcArgs + "Arg vals: " + argVals;
            errStr = funcStr + "\n(ConfigItem = " + showItem + ")";
            ErrorDialog errDlg;
            errDlg.setModal(true);
            errDlg.setError(err, errStr);
            errDlg.exec();
        }
    } else {
        funcStr = nameOfFunc + argVals;
        ui->lblStatus->setText(funcStr);
        //textToAdd = QString("%1 = %2")
        //        .arg(showItem)
        //        .arg(configValue);
        //if(showIndex) textToAdd += QString(" (%1)").arg(index);
        //ui->teShowValues->append(textToAdd);
        QString strIndex = "<td></td>";
        if (showIndex)
            strIndex = "<td>(" + str.setNum(index) + ")</td>";
        textToAdd = "<td>" + showItem + "</td>";
        textToAdd.append(strIndex);
        textToAdd.append("<td>" + str.setNum(configValue) + errDesc + "</td>");
        //textToAdd.append("<td>" + infoDesc + "</td>");
    }
    return textToAdd;
}

QString subWidget::showConfigDbl(int configType, int configItem, QString showItem)
{
    double configValueDbl;
    //UlInfoItemDbl ulInfoItem;
    //DevInfoItem tInfoItem;
    AiConfigItemDbl aiConfigItem;
    //AoInfoItemDbl aoInfoItem;
    //DioInfoItemDbl dioInfoItem;
    //CtrInfoItemDbl ctrInfoItem;
    //TmrInfoItemDbl tmrInfoItem;
    QString nameOfFunc, textToAdd;
    unsigned int index;

    index = ui->spnIndex->value();

    switch (configType){
    case TYPE_AI_INFO:
        nameOfFunc = "ulGetConfigDbl";
        aiConfigItem = (AiConfigItemDbl)configItem;
        err = ulAIGetConfigDbl(mDaqDeviceHandle, aiConfigItem, index, &configValueDbl);
        break;
    default:
        break;
    }

    if(!err==ERR_NO_ERROR)
    {
        QString errStr, argVals;
        QString funcStr, funcArgs;

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
        errStr = funcStr + "\n(ConfigItem = " + showItem + ")";
        ErrorDialog errDlg;
        errDlg.setModal(true);
        errDlg.setError(err, errStr);
        errDlg.exec();
    }
    else
    {
        textToAdd = QString("%1 = %2")
                .arg(showItem)
                .arg(configValueDbl);
        if(showIndex) textToAdd += QString(" (%1)").arg(index);
        ui->teShowValues->append(textToAdd);
    }
    return textToAdd;
}

QString subWidget::showConfigStr(int configType, int configItem, QString showItem)
{
    unsigned int maxConfigLen;
    maxConfigLen = 15;
    char configValue[maxConfigLen];
    char *pConfigValue = configValue;

    DevConfigItemStr devConfigItem;
    QString nameOfFunc;
    QString textToAdd, str;
    unsigned int index;

    index = (uint)configItem;

    switch (configType){
    case TYPE_DEV_INFO:
        devConfigItem = DEV_CFG_VER_STR;
        nameOfFunc = "ulDevGetConfigStr";
        index = (unsigned int)configItem;
        err = ulDevGetConfigStr(mDaqDeviceHandle, devConfigItem, index, pConfigValue, &maxConfigLen);
        break;
    default:
        break;
    }

    if(!err==ERR_NO_ERROR)
    {
        QString errStr, argVals;
        QString funcStr, funcArgs;

        if(configType == TYPE_UL_INFO) {
            funcArgs = "(configItem, index, &configValue, maxConfigLen)\n";
            argVals = QStringLiteral("Arg vals: (%1, %2, %3, %4)")
                    .arg(configItem)
                    .arg(index)
                    .arg(configValue)
                    .arg(maxConfigLen);
        } else {
            funcArgs = "(mDaqDeviceHandle, configItem, index, &configValue, maxConfigLen)\n";
            argVals = QStringLiteral("Arg vals: (%1, %2, %3, %4, %5)")
                    .arg(mDaqDeviceHandle)
                    .arg(configItem)
                    .arg(index)
                    .arg(configValue)
                    .arg(maxConfigLen);
        }
        funcStr = nameOfFunc + funcArgs + argVals;
        errStr = funcStr + "\n(ConfigItem = " + showItem + ")";
        ErrorDialog errDlg;
        errDlg.setModal(true);
        errDlg.setError(err, errStr);
        errDlg.exec();
    } else {
        QString strIndex = "<td></td>";
        if (showIndex)
            strIndex = "<td>(" + str.setNum(index) + ")</td>";
        textToAdd = "<td>" + showItem + "</td>";
        textToAdd.append(strIndex);
        textToAdd.append("<td>" + QString("%1").arg(configValue) + "</td>");
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
    QString errStr, argVals, indexName;
    QString nameOfFunc, funcStr, funcArgs;
    unsigned int index;
    bool indexInfo;

    indexInfo = false;
    index = ui->spnIndex->value();

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
    if(!err==ERR_NO_ERROR) {
        switch (err) {
        case ERR_BAD_DEV_TYPE:
            errDesc = " [error: bad device type]";
            break;
        case ERR_BAD_AI_CHAN:
            errDesc = " [error: bad Ain chan]";
            break;
        case ERR_CONFIG_NOT_SUPPORTED:
            errDesc = " [error: not supported]";
            break;
        case ERR_BAD_CONFIG_VAL:
            errDesc = " [error: bad configVal]";
            break;
        case ERR_BAD_CONFIG_ITEM:
            errDesc = " [error: bad configItem]";
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
        } else {
            funcStr = nameOfFunc + funcArgs + argVals;
            errStr = funcStr + "\n(InfoItem = " + showItem + ")";
            ErrorDialog errDlg;
            errDlg.setModal(true);
            errDlg.setError(err, errStr);
            errDlg.exec();
        }
    } else {
        QString strIndex = "<td></td>";
        if (indexInfo)
            strIndex = "<td>(" + indexName + ")</td>";
        else if (showIndex)
            strIndex = "<td>(" + str.setNum(index) + ")</td>";
        textToAdd = "<td>" + showItem + "</td>";
        textToAdd.append(strIndex);
        textToAdd.append("<td>" + str.setNum(infoValue) + "</td>");
        textToAdd.append("<td>" + infoDesc + "</td>");
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
    unsigned int index;
    bool indexInfo;

    indexInfo = false;
    index = ui->spnIndex->value();

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

    QString errDesc = "";
    infoDesc = getInfoDescription(infoType, infoItem, infoValue);
    if(!err==ERR_NO_ERROR) {
        switch (err) {
        case ERR_BAD_DEV_TYPE:
            errDesc = " [error: bad device type]";
            break;
        case ERR_BAD_AI_CHAN:
            errDesc = " [error: bad Ain chan]";
            break;
        case ERR_CONFIG_NOT_SUPPORTED:
            errDesc = " [error: not supported]";
            break;
        case ERR_BAD_CONFIG_VAL:
            errDesc = " [error: bad configVal]";
            break;
        case ERR_BAD_CONFIG_ITEM:
            errDesc = " [error: bad configItem]";
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
        } else {
            QString errStr, argVals;
            QString funcStr, funcArgs;

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
            funcStr = nameOfFunc + funcArgs + argVals;
            errStr = funcStr + "\n(InfoItem = " + showItem + ")";
            ErrorDialog errDlg;
            errDlg.setModal(true);
            errDlg.setError(err, errStr);
            errDlg.exec();
        }
    } else {
        QString strIndex = "<td></td>";
        if (indexInfo)
            strIndex = "<td>(" + indexName + ")</td>";
        else if (showIndex)
            strIndex = "<td>(" + str.setNum(index) + ")</td>";
        textToAdd = "<td>" + showItem + "</td>";
        textToAdd.append(strIndex);
        textToAdd.append("<td>" + str.setNum(infoValueDbl) + "</td>");
        textToAdd.append("<td>" + infoDesc + "</td>");
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
    DevConfigItemStr devInfoItem;
    AiConfigItemStr aiInfoItem;
    QString nameOfFunc, funcArgs, argVals;
    QString textToAdd, str;
    unsigned int index;

    index = (uint)infoItem;

    switch (infoType){
    case TYPE_UL_INFO:
        nameOfFunc = "ulGetInfoStr";
        ulInfoItem = (UlInfoItemStr)infoItem;
        index = (uint)0;
        err = ulGetInfoStr(ulInfoItem, index, pInfoValue, &maxConfigLen);
        break;
    case TYPE_DEV_INFO:
        devInfoItem = DEV_CFG_VER_STR;
        nameOfFunc = "ulDevGetConfigStr";
        index = (unsigned int)infoItem;
        err = ulDevGetConfigStr(mDaqDeviceHandle, devInfoItem, index, pInfoValue, &maxConfigLen);
        break;
    case TYPE_AI_INFO:
        aiInfoItem = AI_CFG_CAL_DATE_STR;
        index = 0;
        nameOfFunc = "ulAIGetConfigStr";
        err = ulAIGetConfigStr(mDaqDeviceHandle, aiInfoItem, index, pInfoValue, &maxConfigLen);
        break;
    default:
        break;
    }

    if (infoType == TYPE_UL_INFO) {
        funcArgs = "(ulInfoItemStr, index, pInfoString, &maxConfigLen)";
        argVals = QStringLiteral("(%1, %2, %3, %4)")
                .arg(infoItem)
                .arg(index)
                .arg(configValue)
                .arg(maxConfigLen);
    } else {
        funcArgs = "(mDaqDeviceHandle, infoItemStr, index, pInfoString, &maxConfigLen)";
        argVals = QStringLiteral("(%1, %2, %3, %4, %5)")
                .arg(mDaqDeviceHandle)
                .arg(infoItem)
                .arg(index)
                .arg(configValue)
                .arg(maxConfigLen);
    }

    if(!err==ERR_NO_ERROR) {
        QString funcStr = nameOfFunc + funcArgs + "\n";
        QString errStr = funcStr + "Arg vals: " + argVals;
        errStr += "\n(InfoItem = " + showItem + ")";
        ErrorDialog errDlg;
        errDlg.setModal(true);
        errDlg.setError(err, errStr);
        errDlg.exec();
    } else {
        ui->lblStatus->setText(nameOfFunc + argVals);
        QString strIndex = "<td></td>";
        if (showIndex)
            strIndex = "<td>(" + str.setNum(index) + ")</td>";
        textToAdd = "<td>" + showItem + "</td>";
        textToAdd.append(strIndex);
        textToAdd.append("<td>" + QString("%1").arg(configValue) + "</td>");
    }
    return textToAdd;
}

QString subWidget::showInfoMem(MemRegion memRegion)
{
    QString nameOfFunc, funcStr, funcArgs;
    QString showItem, argVals, textToAdd;
    QTime t;
    QString sStartTime;
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
    if(!err==ERR_NO_ERROR) {
        switch (err) {
        case ERR_BAD_MEM_ADDRESS:
            errDesc = "Bad address";
            break;
        case ERR_BAD_MEM_REGION:
            errDesc = "Bad region";
            break;
        case ERR_BAD_MEM_TYPE:
            errDesc = "Bad memtype";
            break;
        default:
            errDesc = "";
            break;
        }
        if (errDesc.length()) {
            textToAdd = "<td>" + showItem + "</td>";
            textToAdd.append("<td>" + errDesc + "</td>");
        } else {
            //mMainWindow->setError(err, sStartTime + funcStr);
        }
    } else {
        //mMainWindow->addFunction(sStartTime + funcStr);
        infoValue = (long long)memDescriptor.accessTypes;
        int infoItem = 0;
        infoDesc = "";
        infoDesc = getInfoDescription(TYPE_MEM_INFO, infoItem, infoValue);
        textToAdd = "<td>" + showItem + "</td>";
        textToAdd.append("<td>" + iSt.setNum(infoValue) + "(" + infoDesc + ")</td>");
        textToAdd.append("<td>address: " + str.setNum(memDescriptor.address) + "</td>");
        textToAdd.append("<td>size: " + str.setNum(memDescriptor.size) + "</td>");
    }
    return textToAdd;
}

void subWidget::setConfiguration()
{
    UlConfigItem ulConfigItem;
    AiConfigItem aiConfigItem;
    AiConfigItemDbl aiConfigItemDbl;

    QString showItem = "";
    QString textToAdd, errStr;
    QString funcStr, nameOfFunc, funcArgs, argVals;

    unsigned int index;
    long long configValue;
    double configValueDbl = 0;
    int configItem, configType;
    bool noConfigItem, dblConfigItem;

    configType = ui->cmbInfoType->currentData(Qt::UserRole).toInt();
    configItem = ui->cmbConfigItem->currentData(Qt::UserRole).toInt();
    configValue = ui->leSetValue->text().toLongLong();
    index = ui->spnIndex->value();

    showItem = ui->cmbConfigItem->currentText();

    noConfigItem = false;
    dblConfigItem = false;
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
        } else {
            dblConfigItem = true;
            aiConfigItemDbl = (AiConfigItemDbl)configItem;
            nameOfFunc = "";
            err = ulAISetConfigDbl(mDaqDeviceHandle, aiConfigItemDbl, index, configValueDbl);
        }
        break;
    case TYPE_AO_INFO:
        noConfigItem = true;
        break;
    case TYPE_DIO_INFO:
        noConfigItem = true;
        break;
    case TYPE_CTR_INFO:
        noConfigItem = true;
        break;
    case TYPE_TMR_INFO:
        noConfigItem = true;
        break;
    default:
        noConfigItem = true;
        break;
    }
    ui->lblStatus->setText(nameOfFunc + argVals + QString(" [Error = %1]").arg(err));

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

    if (!err==ERR_NO_ERROR) {
        funcStr = nameOfFunc + funcArgs + argVals;
        errStr = funcStr + "\n(ConfigItem = " + showItem + ")";
        ErrorDialog errDlg;
        errDlg.setModal(true);
        errDlg.setError(err, errStr);
        errDlg.exec();
    } else {
        textToAdd = QString("%1 = %2")
                .arg(showItem)
                .arg(configValue);
        if(showIndex) textToAdd += QString(" (%1)").arg(index);
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

    tmrStatus = TMRS_IDLE;
    nameOfFunc = ui->cmbInfoType->currentText();

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
    if(!err==ERR_NO_ERROR)
    {
        QString errStr;

        funcStr = nameOfFunc + funcArgs + "Arg vals: " + argVals;
        errStr = funcStr;// + "\n(ConfigItem = " + showItem + ")";
        ErrorDialog errDlg;
        errDlg.setModal(true);
        errDlg.setError(err, errStr);
        errDlg.exec();
    }
    else
    {
        funcStr = nameOfFunc + argVals;
        ui->lblStatus->setText(funcStr);
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
    ui->lblStatus->setText(funcStr);
}
