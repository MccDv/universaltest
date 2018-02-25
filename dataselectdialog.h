#ifndef DATASELECTDIALOG_H
#define DATASELECTDIALOG_H

#include <QDialog>
#include "datamanager.h"

namespace Ui {
class DataSelectDialog;
}

class DataSelectDialog : public QDialog
{
    Q_OBJECT
    Q_PROPERTY(QHash<int, int> waveType READ waveType WRITE setWaveType NOTIFY waveTypeChanged)
    Q_PROPERTY(QHash<int, int> numCycles READ numCycles WRITE setNumCycles NOTIFY numCyclesChanged)
    Q_PROPERTY(QHash<int, double> offset READ offset WRITE setOffset NOTIFY offsetChanged)
    Q_PROPERTY(QHash<int, double> amplitude READ amplitude WRITE setAmplitude NOTIFY amplitudeChanged)
    Q_PROPERTY(QHash<int, int> resolution READ resolution WRITE setResolution NOTIFY resolutionChanged)
    Q_PROPERTY(QHash<int, int> dataScale READ dataScale WRITE setDataScale NOTIFY dataScaleChanged)
    Q_PROPERTY(QHash<int, double> fsRange READ fsRange WRITE setFsRange NOTIFY fsRangeChanged)
    Q_PROPERTY(QHash<int, bool> bipolar READ bipolar WRITE setBipolar NOTIFY bipolarChanged)

public:
    explicit DataSelectDialog(QWidget *parent = 0);
    ~DataSelectDialog();

    void setWaveType(QHash<int, int> waveType) {
        mWaveType = waveType;
        emit waveTypeChanged(waveType);
    }

    void setNumCycles(QHash<int, int> numCycles) {
        mNumCycles = numCycles;
        emit numCyclesChanged(numCycles);
    }

    void setAmplitude(QHash<int, double> amplitude) {
        mAmplitude = amplitude;
        emit amplitudeChanged(amplitude);
    }

    void setOffset(QHash<int, double> offset) {
        mOffset = offset;
        emit offsetChanged(offset);
    }

    void setResolution(QHash<int, int> resolution) {
        mResolution = resolution;
        emit resolutionChanged(resolution);
    }

    void setDataScale(QHash<int, int> dataScale) {
        mDataScale = dataScale;
        emit dataScaleChanged(dataScale);
    }

    void setFsRange(QHash<int, double> fsRange) {
        mFSRange = fsRange;
        emit fsRangeChanged(fsRange);
    }

    void setBipolar(QHash<int, bool> bipolar) {
        mBipolar = bipolar;
        emit bipolarChanged(bipolar);
    }

    QHash<int, int> waveType() { return mWaveType; }
    QHash<int, int> numCycles() { return mNumCycles; }
    QHash<int, double> amplitude() { return mAmplitude; }
    QHash<int, double> offset() { return mOffset; }
    QHash<int, int> resolution() { return mResolution; }
    QHash<int, int> dataScale() { return mDataScale; }
    QHash<int, double> fsRange() { return mFSRange; }
    QHash<int, bool> bipolar() { return mBipolar; }

private slots:
    void setParameters();
    void setCurElement();
    void setMinCycles();
    void deleteElement();
    void syncControls();
    void updateSelections();

private:
    Ui::DataSelectDialog *ui;
    QHash<int, int> mWaveType;
    QHash<int, int> mNumCycles;
    QHash<int, double> mOffset;
    QHash<int, double> mAmplitude;
    QHash<int, int> mResolution;
    QHash<int, double> mFSRange;
    QHash<int, bool> mBipolar;
    QHash<int, int> mDataScale;

signals:
    void waveTypeChanged(QHash<int, int>);
    void numCyclesChanged(QHash<int, int>);
    void offsetChanged(QHash<int, double>);
    void amplitudeChanged(QHash<int, double>);
    void resolutionChanged(QHash<int, int>);
    void fsRangeChanged(QHash<int, double>);
    void bipolarChanged(QHash<int, bool>);
    void dataScaleChanged(QHash<int, int>);
};

#endif // DATASELECTDIALOG_H
