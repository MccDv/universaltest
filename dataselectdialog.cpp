#include "dataselectdialog.h"
#include "ui_dataselectdialog.h"

DataSelectDialog::DataSelectDialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::DataSelectDialog)
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

    ui->fraAmplitude->setFont(font);
    ui->fraDataConfig->setFont(font);
    ui->fraShowConfig->setFont(font);
    ui->cmbWaveForm->addItem("Flat line", DMgr::flatLine);
    ui->cmbWaveForm->addItem("Square wave", DMgr::squareWave);
    ui->cmbWaveForm->addItem("Sine wave", DMgr::sineWave);
    ui->cmbWaveForm->addItem("Positive ramp", DMgr::posRamp);
    ui->cmbWaveForm->addItem("Negative ramp", DMgr::negRamp);
    ui->cmbWaveForm->addItem("Triangle wave", DMgr::triWave);
    ui->cmbWaveForm->setCurrentIndex(2);


    connect(ui->spnElement, SIGNAL(valueChanged(int)), this, SLOT(syncControls()));
    connect(ui->cmbWaveForm, SIGNAL(currentIndexChanged(int)), this, SLOT(setMinCycles()));
    connect(ui->cmdSet, SIGNAL(clicked(bool)), this, SLOT(setParameters()));
    connect(ui->cmdDelete, SIGNAL(clicked(bool)), this, SLOT(deleteElement()));
    connect(this, SIGNAL(amplitudeChanged(QHash<int,double>)), this, SLOT(updateSelections()));
    connect(this, SIGNAL(offsetChanged(QHash<int,double>)), this, SLOT(updateSelections()));
    connect(this, SIGNAL(waveTypeChanged(QHash<int,int>)), this, SLOT(updateSelections()));
    connect(this, SIGNAL(numCyclesChanged(QHash<int,int>)), this, SLOT(updateSelections()));
    connect(ui->lwShowConfig, SIGNAL(itemSelectionChanged()), this, SLOT(setCurElement()));

    //set defaults
    double defaultOffset = 0;
    double defaultAmplitude = 0;
    int defaultScale = mDataScale.value(0);
    int defaultResolution = mResolution.value(0);
    double defaultFSRange = mFSRange.value(0);
    bool isBipolar = mBipolar.value(0);

    switch (defaultScale) {
    case DMgr::counts:
        defaultOffset = qPow(2, defaultResolution) / 2;
        defaultAmplitude = qPow(2, defaultResolution) / 2;
        break;
    case DMgr::volts:
        if (!isBipolar)
            defaultOffset = defaultFSRange / 2;
        defaultAmplitude = defaultFSRange / 2;
        break;
    default:
        break;
    }
    ui->leAmplitude->setText(QString("%1").arg(defaultAmplitude));
    ui->leOffset->setText(QString("%1").arg(defaultOffset));
    ui->spnElement->setValue(0);
    syncControls();
}

DataSelectDialog::~DataSelectDialog()
{
    delete ui;
}

void DataSelectDialog::syncControls()
{
    int selElement = ui->spnElement->value();

    if (mWaveType.contains(selElement)) {
        ui->cmbWaveForm->setCurrentIndex(mWaveType.value(selElement));
        ui->spnCycles->setValue(mNumCycles.value(selElement));
        ui->leAmplitude->setText(QString("%1").arg(mAmplitude.value(selElement)));
        ui->leOffset->setText(QString("%1").arg(mOffset.value(selElement)));
    }
}

void DataSelectDialog::setMinCycles()
{
    ui->spnCycles->setMinimum(0);
    if (!(ui->cmbWaveForm->currentData().toInt() == (int)DMgr::squareWave))
        ui->spnCycles->setMinimum(1);
}

void DataSelectDialog::deleteElement()
{
    int curElement = ui->spnElement->value();

    if (mWaveType.contains(curElement))
        mWaveType.remove(curElement);
    updateSelections();
}

void DataSelectDialog::setCurElement()
{
    int curElement = ui->lwShowConfig->currentRow();
    ui->spnElement->setValue(curElement);
}

void DataSelectDialog::setParameters()
{
    int curElement = ui->spnElement->value();
    QVariant waveSelected = ui->cmbWaveForm->currentData();
    int curWave = waveSelected.toInt();
    int curCycles = ui->spnCycles->value();
    double curOffset = ui->leOffset->text().toDouble();
    double curAmplitude = ui->leAmplitude->text().toDouble();

    if (mWaveType.contains(curElement)) {
        mWaveType[curElement] = curWave;
        mNumCycles[curElement] = curCycles;
        mOffset[curElement] = curOffset;
        mAmplitude[curElement] = curAmplitude;
    } else {
        mWaveType.insert(curElement, curWave);
        mNumCycles.insert(curElement, curCycles);
        mOffset.insert(curElement, curOffset);
        mAmplitude.insert(curElement, curAmplitude);
    }
    updateSelections();
}

void DataSelectDialog::updateSelections()
{
    QString waveDescription;
    //QHashIterator<int, int> i(mWaveType);
    int numWaves = mWaveType.count();
    ui->lwShowConfig->clear();
    int waveForm;

    for (int i = 0; i < numWaves; i++) {
        //i.next();
        waveForm = mWaveType.value(i);
        switch (waveForm) {
        case 0:
            waveDescription = QString("flat line, at level at %2")
                    .arg(mAmplitude.value(i));
            break;
        case 1:
            if (mNumCycles.value(i) == 0) {
                waveDescription = QString("Max rate square wave at %1, ± %2")
                        .arg(mOffset.value(i))
                        .arg(mAmplitude.value(i) / 2);
            } else {
                waveDescription = QString("%1 cycle square wave at %2, ± %3")
                        .arg(mNumCycles.value(i))
                        .arg(mOffset.value(i))
                        .arg(mAmplitude.value(i) / 2);
            }
            break;
        case 2:
            waveDescription = QString("%1 cycle sine wave at %2, ± %3")
                    .arg(mNumCycles.value(i))
                    .arg(mOffset.value(i))
                    .arg(mAmplitude.value(i) / 2);
            break;
        case 3:
            waveDescription = QString("%1 cycle + ramp at %2, ± %3")
                    .arg(mNumCycles.value(i))
                    .arg(mOffset.value(i))
                    .arg(mAmplitude.value(i) / 2);
            break;
        case 4:
            waveDescription = QString("%1 cycle - ramp at %2, ± %3")
                    .arg(mNumCycles.value(i))
                    .arg(mOffset.value(i))
                    .arg(mAmplitude.value(i) / 2);
            break;
        case 5:
            waveDescription = QString("%1 cycle triangle wave at %2, ± %3")
                    .arg(mNumCycles.value(i))
                    .arg(mOffset.value(i))
                    .arg(mAmplitude.value(i) / 2);
            break;
        default:
            break;
        }
        //ui->lwShowConfig->addItem(QString("Ch %1: " + waveDescription).arg(i));
        ui->lwShowConfig->insertItem(i, QString("Ch %1: " + waveDescription).arg(i));
        //ui->teShowConfig->append(QString("Ch %1: " + waveDescription).arg(i));
    }
    syncControls();
}
