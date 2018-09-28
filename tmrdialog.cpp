#include "tmrdialog.h"
#include "ui_tmrdialog.h"

TmrDialog::TmrDialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::TmrDialog)
{
    ui->setupUi(this);

    connect(this, SIGNAL(accepted()), this, SLOT(setParams()));
    connect(this, SIGNAL(intervalChanged(int)), this, SLOT(updateInterval()));
    connect(this, SIGNAL(enabledChanged(bool)), this, SLOT(updateEnabling()));
    connect(this, SIGNAL(onePerIntervalChanged(bool)), this, SLOT(updateOnePer()));
    connect(this, SIGNAL(stopOnStartChanged(bool)), this, SLOT(updateStopOnStart()));
}

TmrDialog::~TmrDialog()
{
    delete ui;
}

void TmrDialog::setParams()
{
    mEnabled = ui->chkTmrEnable->isChecked();
    mInterval = ui->leTmrInterval->text().toInt();
    mStopOnStart = ui->chkStopBG->isChecked();
    mOneSamplePer = ui->chkOnePerCall->isChecked();
}

void TmrDialog::updateInterval()
{
    ui->leTmrInterval->setText(QString("%1").arg(mInterval));
}

void TmrDialog::updateEnabling()
{
    ui->chkTmrEnable->setChecked(mEnabled);
}

void TmrDialog::updateOnePer()
{
    ui->chkOnePerCall->setChecked(mOneSamplePer);
}

void TmrDialog::updateStopOnStart()
{
    ui->chkStopBG->setChecked(mStopOnStart);
}
