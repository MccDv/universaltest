#include "datamanager.h"

DataManager::DataManager()
{

}

void DataManager::getChannelData(double offset, double amplitude, DMgr::WaveType waveType,
                                 int cycles, QVector<double> &floatData, bool truncate)
{
    int togglePoint, element;
    int direction = 1;
    int dataSize = floatData.size();
    double numCycles = (double)cycles;
    double numSamples = (double)dataSize;

    togglePoint = 0;
    if (!cycles == 0)
        togglePoint = ((dataSize / cycles) / 2) - 1;

    element = 0;
    switch (waveType) {
    case DMgr::flatLine:
        for (int i = 0; i < dataSize; i++)
            if (truncate)
                floatData[i] = trunc(amplitude);
            else
                floatData[i] = amplitude;
        break;
    case DMgr::squareWave:
        for (int i = 0; i < dataSize; i++) {
            if (truncate)
                floatData[i] = trunc(offset + ((amplitude / 2) * direction));
            else
                floatData[i] = offset + ((amplitude / 2) * direction);
            element = element + 1;
            if (element > togglePoint) {
                element = 0;
                direction *= -1;
            }
        }
        break;
    case DMgr::sineWave:
        for (int i = 0; i < dataSize; i++) {
            if (truncate)
                floatData[i] = trunc(offset + qSin(i * cycles / (dataSize / 6.28318530718)) * amplitude / 2);
            else
                floatData[i] = offset + qSin(i * cycles / (dataSize / 6.28318530718)) * amplitude / 2;
        }
        break;
    case DMgr::posRamp:
        element = 0;
        for (int i = 0; i < dataSize; i++) {
            if (truncate)
                floatData[i] = trunc((offset - (amplitude / 2))
                    + ((element * (numCycles / numSamples)) * amplitude)
                    + ((amplitude / numSamples) / 2));
            else
                floatData[i] = (offset - (amplitude / 2))
                        + ((element * (numCycles / numSamples)) * amplitude)
                        + ((amplitude / numSamples) / 2);
            element++;
            if (!((element * cycles) < dataSize))
                element = 0;
        }
        break;
    case DMgr::negRamp:
        element = 0;
        for (int i = 0; i < dataSize; i++) {
            if (truncate)
                floatData[i] = trunc((offset + (amplitude / 2))
                    - ((element * (numCycles / numSamples)) * amplitude)
                    - ((amplitude / numSamples) / 2));
            else
                floatData[i] = (offset + (amplitude / 2))
                    - ((element * (numCycles / numSamples)) * amplitude)
                    - ((amplitude / numSamples) / 2);
            element++;
            if (!((element * cycles) < dataSize))
                element = 0;
        }
        break;
    case DMgr::triWave:
        for (int i = 0; i < dataSize; i++) {
            if (i == 0)
                element = dataSize / 2;
            if (truncate)
                floatData[i] = trunc(offset - (amplitude / 2)
                    + (element / numSamples) * amplitude);
            else
                floatData[i] = offset - (amplitude / 2)
                    + (element / numSamples) * amplitude;
            if (element + (cycles * 2 * direction) > (dataSize - 1))
                direction = -1;
            if ((element + (cycles * 2 * direction)) < 0)
                direction = 1;
            element += (cycles * 2 * direction);
        }
        break;
    default:
        break;
    }
}

/*void DataManager::getOutputData(double offset, double amplitude, DMgr::WaveType waveType, int cycles, double* countData, int dataSize)
{
    int x, togglePoint;
    int direction = 1;
    //int dataSize = countData.size();
    togglePoint = 0;
    if (!cycles == 0)
        togglePoint = ((dataSize / cycles) / 2) - 1;
    switch (waveType) {
    case DMgr::flatLine:
        for (int i = 0; i < dataSize; i++)
            countData[i] = amplitude;
        break;
    case DMgr::squareWave:
        x = 1;
        int element;
        for (int i = 0; i < dataSize; i++) {
            countData[i] = offset + ((amplitude / 2) * direction);
            element = element + 1;
            if (element > togglePoint) {
                element = 0;
                direction *= -1;
            }
        }
        break;
    case DMgr::sineWave:
        for (int i = 0; i < dataSize; i++) {
            countData[i] = offset + qSin(i * cycles / (dataSize / 6.28318530718)) * amplitude / 2;
        }
        break;
    case DMgr::posRamp:
        //needs work
        element = 0;
        for (int i = 0; i < dataSize; i++) {
            countData[i] = (offset - (amplitude / 2) + element * cycles / dataSize
                            * amplitude) + ((amplitude / dataSize) / 2);
            element = element + 1;
            if (element * cycles > dataSize - 1)
               element = 0;
        }
        break;
    case DMgr::negRamp:
        //needs work
        for (int i = 0; i < dataSize; i++)
            countData[i] = offset - (amplitude / 2) + ((i / dataSize) * amplitude);
        break;
    case DMgr::triWave:
        for (int i = 0; i < dataSize; i++) {
            int element = dataSize / 2;
            int direction = 1;
            countData[i] = offset - (amplitude / 2) + element / dataSize * amplitude;
            if (element + (cycles * 2 * direction) > (dataSize - 1))
                direction = -1;
        }
        break;
    default:
        break;
    }
}*/

