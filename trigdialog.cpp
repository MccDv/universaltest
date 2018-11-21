#include "trigdialog.h"
#include "ui_trigdialog.h"

TrigDialog::TrigDialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::TrigDialog)
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

    ui->frame->setFont(font);
    ui->fraChanDesc->setFont(font);
    ui->frame_2->setFont(font);
    ui->cmbChanType->addItem("Standard", DaqInChanType(0));
    ui->cmbChanType->addItem("AnalogDiff", DAQI_ANALOG_DIFF);
    ui->cmbChanType->addItem("AnalogSE", DAQI_ANALOG_SE);
    ui->cmbChanType->addItem("Digital", DAQI_DIGITAL);
    ui->cmbChanType->addItem("Counter16", DAQI_CTR16);
    ui->cmbChanType->addItem("Counter32", DAQI_CTR32);
    ui->cmbChanType->addItem("Counter48", DAQI_CTR48);

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

    connect(this, SIGNAL(accepted()), this, SLOT(setParams()));
    connect(this, SIGNAL(trigChannelChanged(int)), this, SLOT(updateTrigChan()));
    connect(this, SIGNAL(trigLevelChanged(double)), this, SLOT(updateTrigLevel()));
    connect(this, SIGNAL(trigVarianceChanged(double)), this, SLOT(updateTrigVariance()));
    connect(this, SIGNAL(retrigCountChanged(uint)), this, SLOT(updateRetrigCount()));
    connect(this, SIGNAL(trigChanTypeChanged(int)), this, SLOT());
    connect(this, SIGNAL(trigRangeChanged(Range)), this, SLOT());
}

TrigDialog::~TrigDialog()
{
    delete ui;
}

void TrigDialog::setParams()
{
    mTrigChannel = ui->spnTrigChan->value();
    mTrigLevel = ui->leTrigLevel->text().toDouble();
    mTrigVariance = ui->leTrigVariance->text().toDouble();
    mRetrigCount = ui->leRetrigCount->text().toUInt();
    mTrigChanType = ui->cmbChanType->currentData().toInt();
    mTrigRange = (Range)ui->cmbRange->currentData().toInt();
}

void TrigDialog::updateTrigChan()
{
    ui->spnTrigChan->setValue(mTrigChannel);
}

void TrigDialog::updateTrigChanType()
{
    int listIndex = (int)mTrigChanType;
    ui->cmbChanType->setCurrentIndex(listIndex);
}

void TrigDialog::updateTrigRange()
{
    int listIndex = (int)mTrigRange - 4;
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

void TrigDialog::updateTrigLevel()
{
    ui->leTrigLevel->setText(QString("%1").arg(mTrigLevel, 2, 'f', 5, '0'));
}

void TrigDialog::updateTrigVariance()
{
    ui->leTrigVariance->setText(QString("%1").arg(mTrigVariance, 2, 'f', 5, '0'));
}

void TrigDialog::updateRetrigCount()
{
    ui->leRetrigCount->setText(QString("%1").arg(mRetrigCount));
}
