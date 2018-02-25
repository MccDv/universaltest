#include "trigdialog.h"
#include "ui_trigdialog.h"

TrigDialog::TrigDialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::TrigDialog)
{
    ui->setupUi(this);

    connect(this, SIGNAL(accepted()), this, SLOT(setParams()));
    connect(this, SIGNAL(trigChannelChanged(int)), this, SLOT(updateTrigChan()));
    connect(this, SIGNAL(trigLevelChanged(double)), this, SLOT(updateTrigLevel()));
    connect(this, SIGNAL(trigVarianceChanged(double)), this, SLOT(updateTrigVariance()));
    connect(this, SIGNAL(retrigCountChanged(uint)), this, SLOT(updateRetrigCount()));
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
}

void TrigDialog::updateTrigChan()
{
    ui->spnTrigChan->setValue(mTrigChannel);
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
