#ifndef TESTUTILITIES_H
#define TESTUTILITIES_H

#include <QtCore>
#include "unitest.h"
#include "uldaq.h"
//#include "aisubwidget.h"

//void eventCallback(DaqDeviceHandle, DaqEventType, unsigned long long, void*);

void setEventsByHandle(DaqDeviceHandle devHandle, DaqEventType eventType, bool enableEvent);
void removeEventsByHandle(DaqDeviceHandle devHandle, DaqEventType eventType);
void setEventParameterByHandle(DaqDeviceHandle devHandle, unsigned long long eventParameter);

void removeEndInScanEvent(DaqDeviceHandle devHandle);
void removeEndOutScanEvent(DaqDeviceHandle devHandle);
void removeInScanErrorEvent(DaqDeviceHandle devHandle);
void removeOutScanErrorEvent(DaqDeviceHandle devHandle);
void removeDataAvailableEvent(DaqDeviceHandle devHandle);
void removeEventParameter(DaqDeviceHandle devHandle);

bool getEndInScanEvent(DaqDeviceHandle devHandle, bool &eventValue);
bool getEndOutScanEvent(DaqDeviceHandle devHandle, bool &eventValue);
bool getInScanErrorEvent(DaqDeviceHandle devHandle, bool &eventValue);
bool getOutScanErrorEvent(DaqDeviceHandle devHandle, bool &eventValue);
bool getDataAvailableEvent(DaqDeviceHandle devHandle, bool &eventValue);
bool getEventParameter(DaqDeviceHandle devHandle, unsigned long long &paramValue);

double getRangeVolts(Range rangeVal);
double getVoltsFromCounts(long long Resolution, Range curRange, int counts);
double getTickValue(CounterTickSize tickSize);

int getRangeIndex(Range rangeCode);
int getDIChanTypeIndex(DaqInChanType chanType);
QString getOptionNames(ScanOption curOptions);
QString getInfoDescription(int infoType, int infoItem, long long infoValue);
QString getEventNames(long long eventMask);
QString getAiInputModeName(AiInputMode inMode);
QString getAiChanTypeName(AiChanType chanType);
QString getAiCalTableTypeName(AiCalTableType tableType);
QString getAiRejFreqName(AiRejectFreqType rejFreq);
QString getChanCouplingModeName(CouplingMode cplMode);
QString getAoSyncModeName(AOutSyncMode syncMode);
QString getAoSenseModeName(int senseMode);
QString getTcTypeName(TcType tcType);
QString getTempUnitName(TempUnit tempUnit);
QString getDigitalDirection(DigitalDirection digDir);
QString getDioPortTypeName(DigitalPortType typeNum);
QString getDPortIoTypeName(DigitalPortIoType ioType);
QString getCtrMeasTypeNames(CounterMeasurementType typeNum);
QString getCtrMeasModeNames(CounterMeasurementMode modeNum);
QString getCtrRegTypeNames(CounterRegisterType regType);
QString getTmrTypeName(TimerType tmrType);
QString getChanTypeNames(DaqInChanType typeNum);
QString getChanTypeNames(AiChanType typeNum);
QString getSensorConnectNames(SensorConnectionType connType);
QString getChanOTypeNames(DaqOutChanType typeNum);
QString getTrigTypeNames(TriggerType typeNum);
QString getQueueTypeNames(AiQueueType typeNum);
QString getQueueLimitNames(AiChanQueueLimitation typeNum);
QString getRangeName(Range rangeVal);
QString getRangeNomo(Range rangeVal);
QString getAccessTypes(MemAccessType memAccess);
QString getRegionNames(MemRegion memRegion);
QString getEnableDisableName(int value);
CounterMeasurementMode getMeasModeFromListIndex(int listIndex);
#endif // TESTUTILITIES_H
