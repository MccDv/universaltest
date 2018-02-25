#ifndef DATAMANAGER_H
#define DATAMANAGER_H

#include <QVector>
#include <QHash>
#include "qmath.h"
#include "tgmath.h"

namespace DMgr {

enum WaveType {
    flatLine    = 0,
    squareWave  = 1,
    sineWave    = 2,
    posRamp     = 3,
    negRamp     = 4,
    triWave     = 5
};

enum DataScale {
    counts      = 0,
    volts       = 1
};

}

class DataManager
{

public:
    DataManager();

    //void getOutputData(double offset, double amplitude, DMgr::WaveType waveType, int cycles, double* floatData, int dataSize);
    void getChannelData(double offset, double amplitude, DMgr::WaveType waveType, int cycles, QVector<double> &floatData, bool truncate);

private:

};

#endif // DATAMANAGER_H
