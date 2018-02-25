#include "queuedialog.h"
#include "ui_queuedialog.h"

QueueDialog::QueueDialog(QWidget *parent, UtFunctionGroup utFuncGroup) :
    QDialog(parent),
    ui(new Ui::QueueDialog)
{
    ui->setupUi(this);

    ui->lstQueue->setStyleSheet("QListWidget { background-color : white; color : blue; }" );
    inputFunctions = (utFuncGroup == FUNC_GROUP_AIN);
    ui->cmbRange->addItem("BIP20VOLTS", BIP20VOLTS);
    ui->cmbRange->addItem("BIP10VOLTS", BIP10VOLTS);
    ui->cmbRange->addItem("BIPPT5VOLTS", BIP5VOLTS);
    ui->cmbRange->addItem("BIP4VOLTS", BIP4VOLTS);
    ui->cmbRange->addItem("BIP2PT5VOLTS", BIP2PT5VOLTS);
    ui->cmbRange->addItem("BIP2VOLTS", BIP2VOLTS);
    ui->cmbRange->addItem("BIP1PT25VOLTS", BIP1PT25VOLTS);
    ui->cmbRange->addItem("BIP1VOLTS", BIP1VOLTS);
    ui->cmbRange->addItem("BIPPT078VOLTS", BIPPT078VOLTS);
    ui->cmbRange->addItem("UNI10VOLTS", UNI10VOLTS);
    ui->cmbRange->addItem("UNI5VOLTS", UNI5VOLTS);

    if (inputFunctions) {
        ui->cmbChanType->addItem("Standard", DaqInChanType(0));
        ui->cmbChanType->addItem("AnalogDiff", DAQI_ANALOG_DIFF);
        ui->cmbChanType->addItem("AnalogSE", DAQI_ANALOG_SE);
        ui->cmbChanType->addItem("Digital", DAQI_DIGITAL);
        ui->cmbChanType->addItem("Counter16", DAQI_CTR16);
        ui->cmbChanType->addItem("Counter32", DAQI_CTR32);
        ui->cmbChanType->addItem("Counter48", DAQI_CTR48);

        ui->cmbMode->setVisible(true);
        ui->cmbMode->addItem("Differential", AI_DIFFERENTIAL);
        ui->cmbMode->addItem("Single Ended", AI_SINGLE_ENDED);
    } else {
        ui->cmbChanType->addItem("AnalogOut", DAQO_ANALOG);
        ui->cmbChanType->addItem("DigitalOut", DAQO_DIGITAL);
        ui->cmbMode->setVisible(false);
    }

    connect(ui->spnQElements, SIGNAL(valueChanged(int)), this, SLOT(updateNumElements(int)));
    connect(ui->lstQueue, SIGNAL(currentRowChanged(int)), this, SLOT(lstQueueRowSelected(int)));
    connect(ui->cmdLoad, SIGNAL(clicked(bool)), this, SLOT(onLoadQueue()));
    connect(this, SIGNAL(numQueueElementsChanged(int)), this, SLOT(updateNumElements(int)));
    connect(ui->cmbChanType, SIGNAL(currentIndexChanged(int)), this, SLOT(chanTypeSelection(int)));
    connect(ui->cmdAutoFill, SIGNAL(clicked(bool)), this, SLOT(autoFill()));
    connect(ui->cmdDelElement, SIGNAL(clicked(bool)), this, SLOT(deleteElement()));
    connect(ui->cmdSave, SIGNAL(clicked(QAbstractButton*)), this, SLOT(done()));

    ui->cmdLoad->setEnabled(false);
    ui->cmdDelElement->setEnabled(false);
    ui->cmdAutoFill->setEnabled(false);
}

QueueDialog::~QueueDialog()
{
    delete ui;
}

void QueueDialog::updateNumElements(int newNumElements)
{
    int elementsShown = ui->spnQElements->value();
    if (!elementsShown) {
        ui->cmbChanType->setEnabled(true);
        ui->cmbMode->setEnabled(true);
        ui->cmdLoad->setEnabled(false);
    }
    if (!(elementsShown == newNumElements))
        ui->spnQElements->setValue(newNumElements);
    updateQueueList(newNumElements);
    ui->cmdAutoFill->setEnabled(newNumElements > mRangeList.count());
    mNumQueueElements = newNumElements;
}

void QueueDialog::chanTypeSelection(int selection)
{
    ui->cmbMode->setEnabled(!selection);
}

void QueueDialog::updateQueueList(int newNumElements)
{
    int numElements = mChanList.count();
    bool daqParams, itemsRemoved;
    QString elementDescription, modeArg;

    itemsRemoved = (newNumElements < numElements);
    daqParams = true;
    if (inputFunctions)
        daqParams = (!(ui->cmbChanType->currentData()).toInt());
    ui->lstQueue->clear();
    for (int i = 0; i < newNumElements; i++) {
        if (i < numElements) {
            if (mChanList.value(i) < 0)
                elementDescription = QString("%1: chan -1, mode, range").arg(i);
            else {
                if (inputFunctions)
                    modeArg = getAiInputModeName(mModeList.value(i));
                else if (daqParams) {
                    if (inputFunctions)
                        modeArg = getChanTypeNames(mChanTypeList.value(i));
                    else
                        modeArg = getChanOTypeNames(mChanOutTypeList.value(i));
                    ui->cmbMode->setEnabled(false);
                } else {
                    ui->cmbChanType->setEnabled(false);
                }
                elementDescription = QString("%1: chan %2, %3, %4")
                    .arg(i)
                    .arg(mChanList.value(i))
                    .arg(modeArg)
                    .arg(getRangeName(mRangeList.value(i)));
            }
            ui->lstQueue->addItem(elementDescription);
        } else {
            elementDescription = QString("%1: chan -1, mode, range").arg(i);
            ui->lstQueue->addItem(elementDescription);
            mChanList.insert(i, -1);
        }
    }
    for (int i = newNumElements; i < numElements; i++) {
        mChanList.remove(i);
        mModeList.remove(i);
        mRangeList.remove(i);
    }
    if (itemsRemoved && newNumElements) {
        mNumQueueElements = newNumElements;
        ui->lstQueue->setCurrentRow(newNumElements - 1);
        mCurElement = newNumElements - 1;
    }
    //removed elements with key of "-1"
    mChanList.remove(-1);
    mChanTypeList.remove(-1);
    mChanOutTypeList.remove(-1);
    mModeList.remove(-1);
    mRangeList.remove(-1);
}

void QueueDialog::lstQueueRowSelected(int rowSelected)
{
    mCurElement = rowSelected;
    ui->cmdLoad->setEnabled(!(rowSelected < 0));
    if (!isAutoSet) {
        if (mChanList.count()) {
            if (!(mChanList[mCurElement]<0)) {
                ui->spnQChan->setValue(mChanList[mCurElement]);
                ui->cmbMode->setCurrentIndex(mModeList[mCurElement]-1);
                int listIndex = (int)mRangeList[mCurElement] - 4;
                if (listIndex < 16)
                    ui->cmbRange->setCurrentIndex(listIndex);
                else {
                    switch (listIndex) {
                    case 15:
                        //BIPPT078VOLTS
                        ui->cmbRange->setCurrentIndex(8);
                        break;
                    case 1002:
                        //UNI10VOLTS
                        ui->cmbRange->setCurrentIndex(9);
                    case 1003:
                        ui->cmbRange->setCurrentIndex(10);
                    default:
                        break;
                    }
                }
            }
        }
    }
}

void QueueDialog::onLoadQueue()
{
    AiInputMode inputMode;
    Range elementRange;
    DaqInChanType chanType;
    DaqOutChanType chanOutType;
    QString curMode;
    bool daqParams;

    elementRange = static_cast<Range>(
                ui->cmbRange->currentData(Qt::UserRole).toInt());
    if (inputFunctions) {
        chanType = static_cast<DaqInChanType>(
                    ui->cmbChanType->currentData(Qt::UserRole).toInt());
        inputMode = static_cast<AiInputMode>(
                    ui->cmbMode->currentData(Qt::UserRole).toInt());
    } else {
        chanOutType = static_cast<DaqOutChanType>(
                    ui->cmbChanType->currentData(Qt::UserRole).toInt());;
    }
    int chanVal = ui->spnQChan->value();

    daqParams = true;
    if (inputFunctions) {
        daqParams = (ui->cmbChanType->currentIndex());
        ui->cmbChanType->setEnabled(daqParams);
        curMode = getChanTypeNames(chanType);
        if (!daqParams)
            curMode = getAiInputModeName(inputMode);
    } else {
        ui->cmbChanType->setEnabled(true);
        ui->cmbMode->setEnabled(false);
        curMode = getChanOTypeNames(chanOutType);
    }
    QString curRange = getRangeName(elementRange);
    QString curElementStr = QString("%1: chan %2, %3, %4")
            .arg(mCurElement)
            .arg(chanVal)
            .arg(curMode)
            .arg(curRange);
    ui->lstQueue->currentItem()->setText(curElementStr);
    mChanList.insert(mCurElement, chanVal);
    mChanTypeList.insert(mCurElement, chanType);
    mChanOutTypeList.insert(mCurElement, chanOutType);
    mModeList.insert(mCurElement, inputMode);
    mRangeList.insert(mCurElement, elementRange);
}

void QueueDialog::autoFill()
{
    int curQSize, curElements, startChan;
    int chanIndex;

    isAutoSet = true;
    int increment = ui->spnAutoInc->value();
    chanIndex = 0;
    curQSize = mRangeList.size();
    //if (curQSize < 0)
    //    curQSize = 0;
    curElements = ui->spnQElements->value();
    startChan = ui->spnQChan->value();
    for (int element = curQSize; element < curElements; element++) {
        ui->spnQChan->setValue(startChan + chanIndex);
        ui->lstQueue->setCurrentRow(element);
        onLoadQueue();
        chanIndex += increment;
    }
    isAutoSet = false;
    ui->cmdAutoFill->setEnabled(false);
}

void QueueDialog::deleteElement()
{
    int selElement;
    int elementCount;

    elementCount = ui->spnQElements->value();
    selElement = ui->lstQueue->currentIndex().row();
    if (!(selElement<0)) {
        mChanList.remove(selElement);
        mModeList.remove(selElement);
        mRangeList.remove(selElement);
        mChanTypeList.remove(selElement);
        mChanOutTypeList.remove(selElement);
    }
    elementCount -= 1;
    updateNumElements(elementCount);
}

void QueueDialog::done()
{
    int numSet = ui->spnQElements->value();
    mNumQueueElements = numSet;
    //for (int i = numSet; i < mChanList.count(); i++)
    mChanList.remove(-1);
    mChanTypeList.remove(-1);
    mChanOutTypeList.remove(-1);
    mModeList.remove(-1);
    mRangeList.remove(-1);
}
