#include "testutilities.h"
#include <QtMath>
#include "aisubwidget.h"

/*CounterMeasurementMode CMM_LATCH_ON_INDEX = static_cast<CounterMeasurementMode>(1 << 26);
CounterMeasurementMode CMM_PHB_CONTROLS_DIR = static_cast<CounterMeasurementMode>(1 << 27);
CounterMeasurementMode CMM_DECREMENT_ON = static_cast<CounterMeasurementMode>(1 << 28);*/

QHash<long long, bool> endInScanEventList;
QHash<long long, bool> inScanErrorEventList;
QHash<long long, bool> outScanErrorEventList;
QHash<long long, bool> endOutScanEventList;
QHash<long long, bool> dataAvailableEventList;
QHash<long long, unsigned long long> eventParamList;

/*void eventCallback(DaqDeviceHandle devHandle, DaqEventType eventType, unsigned long long eventData, void* userData)
{
    AiSubWidget *aiChild = (AiSubWidget *)userData;
    aiChild->callbackHandler(eventType, eventData);
}*/

void setEventsByHandle(DaqDeviceHandle devHandle, DaqEventType eventType, bool enableEvent)
{
    if (eventType & DE_ON_DATA_AVAILABLE) {
        dataAvailableEventList[devHandle] = enableEvent;
    }
    if (eventType & DE_ON_INPUT_SCAN_ERROR) {
        inScanErrorEventList[devHandle] = enableEvent;
    }
    if (eventType & DE_ON_END_OF_INPUT_SCAN) {
        endInScanEventList[devHandle] = enableEvent;
    }
    if (eventType & DE_ON_OUTPUT_SCAN_ERROR) {
        outScanErrorEventList[devHandle] = enableEvent;
    }
    if (eventType & DE_ON_END_OF_OUTPUT_SCAN) {
        endOutScanEventList[devHandle] = enableEvent;
    }
}

void removeEventsByHandle(DaqDeviceHandle devHandle, DaqEventType eventType)
{
    if (eventType & DE_ON_DATA_AVAILABLE) {
        dataAvailableEventList.remove(devHandle);
    }
    if (eventType & DE_ON_INPUT_SCAN_ERROR) {
        inScanErrorEventList.remove(devHandle);
    }
    if (eventType & DE_ON_END_OF_INPUT_SCAN) {
        endInScanEventList.remove(devHandle);
    }
    if (eventType & DE_ON_OUTPUT_SCAN_ERROR) {
        dataAvailableEventList.remove(devHandle);
    }
    if (eventType & DE_ON_END_OF_OUTPUT_SCAN) {
        endOutScanEventList.remove(devHandle);
    }
}

void setEventParameterByHandle(DaqDeviceHandle devHandle, unsigned long long eventParameter)
{
    eventParamList[devHandle] = eventParameter;
}

void removeEndInScanEvent(DaqDeviceHandle devHandle)
{
    endInScanEventList.remove(devHandle);
}

void removeEndOutScanEvent(DaqDeviceHandle devHandle)
{
    endOutScanEventList.remove(devHandle);
}

void removeInScanErrorEvent(DaqDeviceHandle devHandle)
{
    inScanErrorEventList.remove(devHandle);
}

void removeOutScanErrorEvent(DaqDeviceHandle devHandle)
{
    outScanErrorEventList.remove(devHandle);
}

void removeDataAvailableEvent(DaqDeviceHandle devHandle)
{
    dataAvailableEventList.remove(devHandle);
}

void removeAllEventsByHandle(DaqDeviceHandle devHandle)
{
    endInScanEventList.remove(devHandle);
    inScanErrorEventList.remove(devHandle);
    outScanErrorEventList.remove(devHandle);
    endOutScanEventList.remove(devHandle);
    dataAvailableEventList.remove(devHandle);
    eventParamList.remove(devHandle);
}

bool getEndInScanEvent(DaqDeviceHandle devHandle, bool &eventValue)
{
    eventValue = endInScanEventList.value(devHandle);
    return endInScanEventList.contains(devHandle);
}

bool getEndOutScanEvent(DaqDeviceHandle devHandle, bool &eventValue)
{
    eventValue = endOutScanEventList.value(devHandle);
    return endOutScanEventList.contains(devHandle);
}

bool getInScanErrorEvent(DaqDeviceHandle devHandle, bool &eventValue)
{
    eventValue = inScanErrorEventList.value(devHandle);
    return inScanErrorEventList.contains(devHandle);
}

bool getOutScanErrorEvent(DaqDeviceHandle devHandle, bool &eventValue)
{
    eventValue = outScanErrorEventList.value(devHandle);
    return outScanErrorEventList.contains(devHandle);
}

bool getDataAvailableEvent(DaqDeviceHandle devHandle, bool &eventValue)
{
    eventValue = dataAvailableEventList.value(devHandle);
    return dataAvailableEventList.contains(devHandle);
}

bool getEventParameter(DaqDeviceHandle devHandle, unsigned long long &paramValue)
{
    paramValue = eventParamList.value(devHandle);
    return eventParamList.contains(devHandle);
}

int getDIChanTypeIndex(DaqInChanType chanType)
{
    switch (chanType) {
    case DAQI_ANALOG_DIFF:
        return 1;
        break;
    case DAQI_ANALOG_SE:
        return 2;
        break;
    case DAQI_DIGITAL:
        return 3;
        break;
    case DAQI_CTR16:
        return 4;
        break;
    case DAQI_CTR32:
        return 5;
        break;
    case DAQI_CTR48:
        return 6;
        break;
    case DAQI_DAC:
        return 7;
        break;
    default:
        return 0;
        break;
    }
}

int getRangeIndex(Range rangeCode)
{
    int trueIndex,  indexResult;
    int currentOffset;  //changes as menus are modified

    currentOffset = -1;
    switch (rangeCode) {
    case BIP60VOLTS:
        trueIndex = 0;
        break;
    case BIP30VOLTS:
        trueIndex = 1;
        break;
    case BIP20VOLTS:
        trueIndex = 2;
        currentOffset = 2;
        break;
    case BIP15VOLTS:
        trueIndex = 3;
        break;
    case BIP10VOLTS:
        trueIndex = 4;
        currentOffset = 3;
        break;
    case BIP5VOLTS:
        trueIndex = 5;
        currentOffset = 3;
        break;
    case BIP4VOLTS:
        trueIndex = 6;
        currentOffset = 3;
        break;
    case BIP3VOLTS:
        trueIndex = 7;
        currentOffset = 3;
        break;
    case BIP2PT5VOLTS:
        trueIndex = 8;
        currentOffset = 3;
        break;
    case BIP2VOLTS:
        trueIndex = 9;
        currentOffset = 3;
        break;
    case BIP1PT25VOLTS:
        trueIndex = 10;
        currentOffset = 3;
        break;
    case BIP1VOLTS:
        trueIndex = 11;
        currentOffset = 3;
        break;
    case BIPPT625VOLTS:
        trueIndex = 12;
        currentOffset = 3;
        break;
    case BIPPT5VOLTS:
        trueIndex = 13;
        break;
    case BIPPT312VOLTS:
        trueIndex = 14;
        currentOffset = 4;
        break;
    case BIPPT156VOLTS:
        trueIndex = 15;
        currentOffset = 4;
        break;
    case BIPPT2VOLTS:
        trueIndex = 16;
        break;
    case BIPPT125VOLTS:
        trueIndex = 17;
        break;
    case BIPPT1VOLTS:
        trueIndex = 18;
        break;
    case BIPPT078VOLTS:
        trueIndex = 19;
        currentOffset = 7;
        break;
    case BIPPT05VOLTS:
        trueIndex = 20;
        break;
    case BIPPT01VOLTS:
        trueIndex = 21;
        break;
    case BIPPT005VOLTS:
        trueIndex = 22;
        break;
    case UNI60VOLTS:
        trueIndex = 23;
        break;
    case UNI30VOLTS:
        trueIndex = 24;
        break;
    case UNI20VOLTS:
        trueIndex = 25;
        break;
    case UNI15VOLTS:
        trueIndex = 26;
        break;
    case UNI10VOLTS:
        trueIndex = 27;
        currentOffset = 14;
        break;
    case UNI5VOLTS:
        trueIndex = 28;
        currentOffset = 14;
        break;
    default:
        break;
    }

    /*
    UNI4VOLTS       = 1007,
    UNI2PT5VOLTS    = 1008,
    UNI2VOLTS       = 1009,
    UNI1PT25VOLTS   = 1010,
    UNI1VOLTS       = 1011,
    UNIPT625VOLTS   = 1012,
    UNIPT5VOLTS     = 1013,
    UNIPT25VOLTS    = 1014,
    UNIPT125VOLTS   = 1015,
    UNIPT2VOLTS     = 1016,
    UNIPT1VOLTS     = 1017,
    UNIPT078VOLTS   = 1018,
    UNIPT05VOLTS    = 1019,
    UNIPT01VOLTS    = 1020,
    UNIPT005VOLTS   = 1021,
    MA0TO20 = 2000*/

    if(currentOffset < 0)
        trueIndex = 2;
    indexResult = trueIndex - currentOffset;
    return indexResult;
}

double getRangeVolts(Range rangeVal)
{
    double rangeVolts;

    //rangeInt = static_cast<Range>(rangeVal);
    switch (rangeVal) {
    case BIP60VOLTS:
        rangeVolts = 120;
        break;
    case BIP30VOLTS:
        rangeVolts = 60;
        break;
    case BIP15VOLTS:
        rangeVolts = 30;
        break;
    case BIP20VOLTS:
        rangeVolts = 40;
        break;
    case BIP10VOLTS:
        rangeVolts = 20;
        break;
    case BIP5VOLTS:
        rangeVolts = 10;
        break;
    case BIP4VOLTS:
        rangeVolts = 8;
        break;
    case BIP3VOLTS:
        rangeVolts = 6;
        break;
    case BIP2PT5VOLTS:
        rangeVolts = 5;
        break;
    case BIP2VOLTS:
        rangeVolts = 4;
        break;
    case BIP1PT25VOLTS:
        rangeVolts = 2.5;
        break;
    case BIP1VOLTS:
        rangeVolts = 2;
        break;
    case BIPPT625VOLTS:
        rangeVolts = 1.25;
        break;
    case BIPPT5VOLTS:
        rangeVolts = 1;
        break;
    case BIPPT312VOLTS:
        rangeVolts = 0.624;
        break;
    case BIPPT25VOLTS:
        rangeVolts = 0.5;
        break;
    case BIPPT125VOLTS:
        rangeVolts = 0.25;
        break;
    case BIPPT2VOLTS:
        rangeVolts = 0.4;
        break;
    case BIPPT156VOLTS:
        rangeVolts = 0.312;
        break;
    case BIPPT1VOLTS:
        rangeVolts = 0.2;
        break;
    case BIPPT078VOLTS:
        rangeVolts = 0.156;
        break;
    case BIPPT05VOLTS:
        rangeVolts = 0.1;
        break;
    case BIPPT01VOLTS:
        rangeVolts = 0.02;
        break;
    case BIPPT005VOLTS:
        rangeVolts = 0.01;
        break;
    case UNI60VOLTS:
        rangeVolts = 60;
        break;
    case UNI30VOLTS:
        rangeVolts = 30;
        break;
    case UNI15VOLTS:
        rangeVolts = 15;
        break;
    case UNI20VOLTS:
        rangeVolts = 20;
        break;
    case UNI10VOLTS:
        rangeVolts = 10;
        break;
    case UNI5VOLTS:
        rangeVolts = 5;
        break;
    case MA0TO20:
        rangeVolts = 0.02;
        break;
    default:
        rangeVolts = 0;
        break;
    }

    return rangeVolts;

}

double getVoltsFromCounts(long long Resolution, Range curRange, int counts)
{
    double LSB, bipOffset, calcVolts;

    double FSR = getRangeVolts(curRange);
    bipOffset = 0;
    LSB = FSR / qPow(2, Resolution);
    if (curRange < 100)
        bipOffset = FSR / 2;
    calcVolts = counts * LSB;
    return calcVolts - bipOffset;
}

double getTickValue(CounterTickSize tickSize)
{
    switch (tickSize) {
    case CTS_TICK_20PT83ns:
        return .00000002083;
    case CTS_TICK_208PT3ns:
        return .0000002083;
    case CTS_TICK_2083PT3ns:
        return .000002083;
    case CTS_TICK_20833PT3ns:
        return .00002083;
    case CTS_TICK_20ns:
        return .000000020;
    case CTS_TICK_200ns:
        return .00000020;
    case CTS_TICK_2000ns:
        return .0000020;
    case CTS_TICK_20000ns:
        return .000020;
    default:
        return 1;
    }
}

QString getOptionNames(ScanOption curOptions)
{
    QString optString;

    if (curOptions == 0)
        optString = "Default";
    else {
        if (curOptions & SO_SINGLEIO)
            optString = "SingleIO, ";
        if (curOptions & SO_BLOCKIO)
            optString += "BlockIO, ";
        if (curOptions & SO_BURSTIO)
            optString += "BurstIO, ";
        if (curOptions & SO_CONTINUOUS)
            optString += "Continuous, ";
        if (curOptions & SO_EXTCLOCK)
            optString += "ExtClock, ";
        if (curOptions & SO_EXTTRIGGER)
            optString += "ExtTrigger, ";
        if (curOptions & SO_RETRIGGER)
            optString += "ReTrigger, ";
        if (curOptions & SO_BURSTMODE)
            optString += "BurstMode, ";
        if (curOptions & SO_PACEROUT)
            optString += "PacerOut, ";
        if (curOptions & SO_EXTTIMEBASE)
            optString += "ExtTimebase, ";
        if (curOptions & SO_TIMEBASEOUT)
            optString += "TimebaseOut, ";
        optString = optString.left(optString.length() - 2);
    }

    return optString;
}

QString getInfoDescription(int infoType, int infoItem, long long infoValue)
{

    switch (infoType) {
    case TYPE_DEV_INFO:
        if (infoItem == DEV_INFO_DAQ_EVENT_TYPES)
            return getEventNames(infoValue);
        if (infoItem == DEV_INFO_MEM_REGIONS)
            return getRegionNames((MemRegion)infoValue);
        break;
    case TYPE_AI_INFO:
        if (infoItem == AI_INFO_CHAN_TYPES)
            return getChanTypeNames((AiChanType)infoValue);
        if (infoItem == AI_INFO_SCAN_OPTIONS)
            return getOptionNames((ScanOption)infoValue);
        if ((infoItem == AI_INFO_DIFF_RANGE) | (infoItem == AI_INFO_SE_RANGE))
            return getRangeName((Range)infoValue);
        if (infoItem == AI_INFO_TRIG_TYPES)
            return getTrigTypeNames((TriggerType)infoValue);
        if (infoItem == AI_INFO_QUEUE_TYPES)
            return getQueueTypeNames((AiQueueType)infoValue);
        if (infoItem == AI_INFO_QUEUE_LIMITS)
            return getQueueLimitNames((AiChanQueueLimitation)infoValue);
        break;
    case TYPE_AO_INFO:
        if (infoItem == AO_INFO_SCAN_OPTIONS)
            return getOptionNames((ScanOption)infoValue);
        if (infoItem == AO_INFO_RANGE)
            return getRangeName((Range)infoValue);
        if (infoItem == AO_INFO_TRIG_TYPES)
            return getTrigTypeNames((TriggerType)infoValue);
        break;
    case TYPE_DIO_INFO:
        if (infoItem == DIO_INFO_PORT_TYPE)
            return getDioPortTypeName((DigitalPortType)infoValue);
        if (infoItem == DIO_INFO_PORT_IO_TYPE)
            return getDPortIoTypeName((DigitalPortIoType)infoValue);
        if (infoItem == DIO_INFO_TRIG_TYPES)
            return getTrigTypeNames((TriggerType)infoValue);
        if (infoItem == DIO_INFO_SCAN_OPTIONS)
            return getOptionNames((ScanOption)infoValue);
        break;
    case TYPE_CTR_INFO:
        if (infoItem == CTR_INFO_MEASUREMENT_TYPES)
            return getCtrMeasTypeNames((CounterMeasurementType)infoValue);
        if (infoItem == CTR_INFO_MEASUREMENT_MODES)
            return getCtrMeasModeNames((CounterMeasurementMode)infoValue);
        if (infoItem == CTR_INFO_REGISTER_TYPES)
            return getCtrRegTypeNames((CounterRegisterType)infoValue);
        if (infoItem == CTR_INFO_SCAN_OPTIONS)
            return getOptionNames((ScanOption)infoValue);
        if (infoItem == CTR_INFO_TRIG_TYPES)
            return getTrigTypeNames((TriggerType)infoValue);
        break;
    case TYPE_TMR_INFO:
        if (infoItem == TMR_INFO_TYPE)
            return getTmrTypeName((TimerType)infoValue);
        break;
    case TYPE_DAQI_INFO:
        if (infoItem == DAQI_INFO_CHAN_TYPES)
            return getChanTypeNames((DaqInChanType)infoValue);
        if (infoItem == DAQI_INFO_TRIG_TYPES)
            return getTrigTypeNames((TriggerType)infoValue);
        if (infoItem == DAQI_INFO_SCAN_OPTIONS)
            return getOptionNames((ScanOption)infoValue);
        break;
    case TYPE_DAQO_INFO:
        if (infoItem == DAQO_INFO_CHAN_TYPES)
            return getChanOTypeNames((DaqOutChanType)infoValue);
        if (infoItem == DAQO_INFO_TRIG_TYPES)
            return getTrigTypeNames((TriggerType)infoValue);
        if (infoItem == DAQO_INFO_SCAN_OPTIONS)
            return getOptionNames((ScanOption)infoValue);
        break;
    case TYPE_MEM_INFO:
        return getAccessTypes((MemAccessType)infoValue);
        break;
    default:
        break;
    }
    return "";
}

QString getQueueTypeNames(AiQueueType typeNum)
{
    int mask;
    AiQueueType maskedVal;
    QString chanName;

    maskedVal = (AiQueueType)0;
    for (int i = 0; i < 3; i++) {
        mask = pow(2, i);
        maskedVal = (AiQueueType)(typeNum & (AiQueueType)mask);
        switch (maskedVal) {
        case CHAN_QUEUE:
            chanName += "ChanQueue, ";
            break;
        case GAIN_QUEUE:
            chanName += "GainQueue, ";
            break;
        case MODE_QUEUE:
            chanName += "ModeQueue, ";
            break;
        default:
            //chanName += "Invalid QType, ";
            break;
        }
    }
    int loc = chanName.lastIndexOf(",");
    return chanName.left(loc);
}

QString getQueueLimitNames(AiChanQueueLimitation typeNum)
{
    int mask;
    AiChanQueueLimitation maskedVal;
    QString chanName;

    maskedVal = (AiChanQueueLimitation)0;
    for (int i = 0; i < 2; i++) {
        mask = pow(2, i);
        maskedVal = (AiChanQueueLimitation)(typeNum & (AiChanQueueLimitation)mask);
        switch (maskedVal) {
        case UNIQUE_CHAN:
            chanName += "Unique, ";
            break;
        case ASCENDING_CHAN:
            chanName += "Ascending, ";
            break;
        default:
            //chanName += "Invalid QLim, ";
            break;
        }
    }
    int loc = chanName.lastIndexOf(",");
    return chanName.left(loc);
}

QString getTrigTypeNames(TriggerType typeNum)
{
    int mask;
    TriggerType maskedVal;
    QString chanName;

    maskedVal = (TriggerType)0;
    for (int i = 0; i < 19; i++) {
        mask = pow(2, i);
        maskedVal = (TriggerType)(typeNum & (TriggerType)mask);
        switch (maskedVal) {
        case TRIG_POS_EDGE:
            chanName += "Positive Edge, ";
            break;
        case TRIG_NEG_EDGE:
            chanName += "Negative Edge, ";
            break;
        case TRIG_HIGH:
            chanName += "TrigHigh, ";
            break;
        case TRIG_LOW:
            chanName += "TrigLow, ";
            break;
        case GATE_HIGH:
            chanName += "GateHigh, ";
            break;
        case GATE_LOW:
            chanName += "GateLow, ";
            break;
        case TRIG_RISING:
            chanName += "TrigRising, ";
            break;
        case TRIG_FALLING:
            chanName += "TrigFalling, ";
            break;
        case TRIG_ABOVE:
            chanName += "TrigAbove, ";
            break;
        case TRIG_BELOW:
            chanName += "TrigBelow, ";
            break;
        case GATE_ABOVE:
            chanName += "GateAbove, ";
            break;
        case GATE_BELOW:
            chanName += "GateBelow, ";
            break;
        case GATE_IN_WINDOW:
            chanName += "GateInWindow, ";
            break;
        case GATE_OUT_WINDOW:
            chanName += "GateOutWindow, ";
            break;
        case TRIG_PATTERN_EQ:
            chanName += "TrigPatternEq, ";
            break;
        case TRIG_PATTERN_NE:
            chanName += "TrigPatternNE, ";
            break;
        case TRIG_PATTERN_ABOVE:
            chanName += "TrigPatternAbove, ";
            break;
        case TRIG_PATTERN_BELOW:
            chanName += "TrigPatternBelow, ";
            break;
        default:
            break;
        }
    }
    int loc = chanName.lastIndexOf(",");
    return chanName.left(loc);
}

QString getEventNames(long long eventMask)
{
    QString eventString;

    if (eventMask == 0)
        return "No Events";
    else {
        if (eventMask & DE_ON_DATA_AVAILABLE)
            eventString = "OnDataAvailable, ";
        if (eventMask & DE_ON_INPUT_SCAN_ERROR)
            eventString += "OnInScanError, ";
        if (eventMask & DE_ON_END_OF_INPUT_SCAN)
            eventString += "OnEndOfInScan, ";
        if (eventMask & DE_ON_OUTPUT_SCAN_ERROR)
            eventString += "OnOutScanError, ";
        if (eventMask & DE_ON_END_OF_OUTPUT_SCAN)
            eventString += "OnEndOfOutScan, ";
        if (eventString.length())
            return eventString.left(eventString.length() - 2);
        return "Invalid EventType";
    }
}

QString getAiInputModeName(AiInputMode inMode)
{
    //itemInt = static_cast<AiInputMode>(inMode);
    switch (inMode) {
    case AI_DIFFERENTIAL:
        return "Diff";
    case AI_SINGLE_ENDED:
        return "SE";
    case AI_PSEUDO_DIFFERENTIAL:
        return "Pseudo Diff";
    default:
        return "Invalid mode";
    }
}

QString getAiChanTypeName(AiChanType chanType)
{
    QString chanTypeString;

    if (chanType == 0)
        return "Invalid Type";
    else {
        if (chanType & AI_VOLTAGE)
            chanTypeString = "Volts, ";
        if (chanType & AI_TC)
            chanTypeString += "TC, ";
        if (chanType & AI_RTD)
            chanTypeString += "RTD, ";
        if (chanType & AI_THERMISTOR)
            chanTypeString += "Tmstr, ";
        if (chanType & AI_SEMICONDUCTOR)
            chanTypeString += "SemiCdr, ";
        if (chanType & AI_DISABLED)
            chanTypeString += "Disabled, ";
        if (chanTypeString.length())
            return chanTypeString.left(chanTypeString.length() - 2);
        return "Invalid Type";
    }
}

QString getAiCalTableTypeName(AiCalTableType tableType)
{
    switch(tableType) {
    case AI_CTT_FACTORY:
        return "Factory Cal";
    case AI_CTT_FIELD:
        return "Field Cal";
    default:
        return "Invalid Cal Table";
    }
}

QString getAiRejFreqName(AiRejectFreqType rejFreq)
{
    switch (rejFreq) {
    case AI_RFT_50HZ:
        return "50Hz Rejection";
    case AI_RFT_60HZ:
        return "60Hz Rejection";
    default:
        return "Invalid rejFreq";
    }
}

QString getChanTypeNames(AiChanType typeNum)
{
    int mask;
    AiChanType maskedVal;
    QString chanName;

    maskedVal = (AiChanType)0;
    for (int i = 0; i < 6; i++) {
        mask = pow(2, i);
        if (i == 5)
            mask = pow(2, 30);
        maskedVal = (AiChanType)(typeNum & (AiChanType)mask);
        switch (maskedVal) {
        case AI_VOLTAGE:
            chanName += "Voltage, ";
            break;
        case AI_TC:
            chanName += "TC, ";
            break;
        case AI_RTD:
            chanName += "RTD, ";
            break;
        case AI_THERMISTOR:
            chanName += "Thermistor, ";
            break;
        case AI_SEMICONDUCTOR:
            chanName += "Semiconductor, ";
            break;
        case AI_DISABLED:
            chanName += "Disabled, ";
            break;
        default:
            //chanName += "Invalid Type";
            break;
        }
    }
    int loc = chanName.lastIndexOf(",");
    return chanName.left(loc);
}

QString getAoSyncModeName(AOutSyncMode syncMode)
{
    switch (syncMode) {
    case AOSM_SLAVE:
        return "Slave";
        break;
    case AOSM_MASTER:
        return "Master";
        break;
    default:
        return "Invalid";
        break;
    }
}

QString getAoSenseModeName(int senseMode)
{
    //change int to AOutSenseMode when header is updated
    switch (senseMode) {
    case 1:
        //AOSM_DISABLED
        return "Disabled";
        break;
    case 2:
        //AOSM_ENABLED
        return "Enabled";
        break;
    default:
        return "Invalid";
        break;
    }
}

QString getTcTypeName(TcType tcType)
{
    switch (tcType) {
    case TC_J:
        return "J";
        break;
    case TC_K:
        return "K";
        break;
    case TC_T:
        return "T";
        break;
    case TC_E:
        return "E";
        break;
    case TC_R:
        return "R";
        break;
    case TC_S:
        return "S";
        break;
    case TC_B:
        return "B";
        break;
    case TC_N:
        return "N";
        break;
    default:
        return "Invalid Type";
        break;
    }
}

QString getTempUnitName(TempUnit tempUnit)
{
    switch (tempUnit) {
    case TU_CELSIUS:
        return "Celsius";
    case TU_FAHRENHEIT:
        return "Fahrenheit";
    case TU_KELVIN:
        return "Kelvin";
    default:
        return "Invalid temp unit";
    }
}

QString getChanCouplingModeName(CouplingMode cplMode)
{
    switch (cplMode) {
    case CM_DC:
        return "DC Coupling";
    case CM_AC:
        return "AC Coupling";
    default:
        return "Invalid Coupling Mode";
    }
}

QString getEnableDisableName(int value)
{
    switch (value) {
    case 1:
        return "Disabled";
    case 2:
        return "Enabled";
    default:
        return "Invalid Value";
    }
}

QString getSensorConnectNames(SensorConnectionType connType)
{
    switch (connType) {
    case SCT_2_WIRE_1:
        return "Two-wire, 1 sensor";
        break;
    case SCT_2_WIRE_2:
        return "Two-wire, 2 sensor";
        break;
    case SCT_3_WIRE:
        return "Three-wire";
        break;
    case SCT_4_WIRE:
        return "Four-wire";
        break;
    default:
        return "Invalid Type";
        break;
    }
}

QString getDigitalDirection(DigitalDirection digDir)
{
    switch (digDir) {
    case DD_INPUT:
        return "Input";
        break;
    case DD_OUTPUT:
        return "Output";
        break;
    default:
        return "InvalidDir";
        break;
    }
}

QString getDioPortTypeName(DigitalPortType typeNum)
{
    switch (typeNum) {
    case AUXPORT0:
        return "Auxport0";
    case AUXPORT1:
        return "Auxport1";
    case AUXPORT2:
        return "Auxport2";
    case FIRSTPORTA:
        return "FirstPortA";
    case FIRSTPORTB:
        return "FirstPortB";
    case FIRSTPORTCL:
        return "FirstPortCL";
    case FIRSTPORTCH:
        return "FirstPortCH";
    case SECONDPORTA:
        return "SecondPortA";
    case SECONDPORTB:
        return "SecondPortB";
    case SECONDPORTCL:
        return "SecondPortCL";
    case SECONDPORTCH:
        return "SecondPortCH";
    case THIRDPORTA:
        return "ThirdPortA";
    case THIRDPORTB:
        return "ThirdPortB";
    case THIRDPORTCL:
        return "ThirdPortCL";
    case THIRDPORTCH:
        return "ThirdPortCH";
    case FOURTHPORTA:
        return "FourthPortA";
    case FOURTHPORTB:
        return "FourthPortB";
    case FOURTHPORTCL:
        return "FourthPortCL";
    case FOURTHPORTCH:
        return "FourthPortCH";
    case FIFTHPORTA:
        return "FifthPortA";
    case FIFTHPORTB:
        return "FifthPortB";
    case FIFTHPORTCL:
        return "FifthPortCL";
    case FIFTHPORTCH:
        return "FifthPortCH";
    case SIXTHPORTA:
        return "SixthPortA";
    case SIXTHPORTB:
        return "SixthPortB";
    case SIXTHPORTCL:
        return "SixthPortCL";
    case SIXTHPORTCH:
        return "SixthPortCH";
    case SEVENTHPORTA:
        return "SeventhPortA";
    case SEVENTHPORTB:
        return "SeventhPortB";
    case SEVENTHPORTCL:
        return "SeventhPortCL";
    case SEVENTHPORTCH:
        return "SeventhPortCH";
    case EIGHTHPORTA:
        return "EighthPortA";
    case EIGHTHPORTB:
        return "EighthPortB";
    case EIGHTHPORTCL:
        return "EighthPortCL";
    case EIGHTHPORTCH:
        return "EighthPortCH";
    default:
        return "Invalid Type";
        break;
    }
}

QString getDPortIoTypeName(DigitalPortIoType ioType)
{
    switch (ioType) {
    case DPIOT_IN:
        return "Fixed input";
        break;
    case DPIOT_OUT:
        return "Fixed output";
        break;
    case DPIOT_IO:
        return "IO Port-Config";
        break;
    case DPIOT_BITIO:
        return "IO Bit-Config";
        break;
    case DPIOT_NONCONFIG:
        return "IO Non-Config";
        break;
    default:
        return "Invalid Type";
        break;
    }
}

QString getCtrMeasTypeNames(CounterMeasurementType typeNum)
{
    int mask;
    CounterMeasurementType maskedVal;
    QString chanName;

    maskedVal = (CounterMeasurementType)0;
    for (int i = 0; i < 6; i++) {
        mask = pow(2, i);
        maskedVal = (CounterMeasurementType)(typeNum & (CounterMeasurementType)mask);
        switch (maskedVal) {
        case CMT_COUNT:
            chanName += "Count, ";
            break;
        case CMT_PERIOD:
            chanName += "Period, ";
            break;
        case CMT_PULSE_WIDTH:
            chanName += "PulseWidth, ";
            break;
        case CMT_TIMING:
            chanName += "Timing, ";
            break;
        case CMT_ENCODER:
            chanName += "Encoder, ";
            break;
        default:
            //chanName += "Invalid CtrType, ";
            break;
        }
    }
    int loc = chanName.lastIndexOf(",");
    return chanName.left(loc);
}

QString getCtrRegTypeNames(CounterRegisterType regType)
{
    int mask;
    CounterRegisterType maskedVal;
    QString regName;

    maskedVal = (CounterRegisterType)0;
    for (int i = 0; i < 6; i++) {
        mask = pow(2, i);
        maskedVal = (CounterRegisterType)(regType & (CounterRegisterType)mask);
        switch (maskedVal) {
        case CRT_COUNT:
            regName += "Count, ";
            break;
        case CRT_LOAD:
            regName += "Load, ";
            break;
        case CRT_MIN_LIMIT:
            regName += "MinLimit, ";
            break;
        case CRT_MAX_LIMIT:
            regName += "MaxLimit, ";
            break;
        case CRT_OUTPUT_VAL0:
            regName += "Output0, ";
            break;
        case CRT_OUTPUT_VAL1:
            regName += "Output1, ";
            break;
        default:
            //regName += "Invalid RegType, ";
            break;
        }
    }
    int loc = regName.lastIndexOf(",");
    return regName.left(loc);
}

QString getCtrMeasModeNames(CounterMeasurementMode modeNum)
{
    int mask;
    CounterMeasurementMode maskedVal;
    QString modeName;

    if (modeNum == 0)
        return "Default";

    maskedVal = (CounterMeasurementMode)0;
    for (int i = 0; i < 26; i++) {
        mask = pow(2, i);
        maskedVal = (CounterMeasurementMode)(modeNum & (CounterMeasurementMode)mask);
        switch (maskedVal) {
        case CMM_CLEAR_ON_READ:
            modeName += "ClearOnRead, ";
            break;
        case CMM_COUNT_DOWN:
            modeName += "CountDown, ";
            break;
        case CMM_GATE_CONTROLS_DIR:
            modeName += "GateDir, ";
            break;
        case CMM_GATE_CLEARS_CTR:
            modeName += "GateClear, ";
            break;
        case CMM_GATE_TRIG_SRC:
            modeName += "GateTrig, ";
            break;
        case CMM_OUTPUT_ON:
            modeName += "OutputOn, ";
            break;
        case CMM_OUTPUT_INITIAL_STATE_HIGH:
            modeName += "InitHigh, ";
            break;
        case CMM_NO_RECYCLE:
            modeName += "NoRecycle, ";
            break;
        case CMM_RANGE_LIMIT_ON:
            modeName += "Rangelimit, ";
            break;
        case CMM_GATING_ON:
            modeName += "GateOn, ";
            break;
        case CMM_INVERT_GATE:
            modeName += "InvGate, ";
            break;
        case CMM_PERIOD_X10:
            modeName += "Per X10, ";
            break;
        case CMM_PERIOD_X100:
            modeName += "Per X100, ";
            break;
        case CMM_PERIOD_X1000:
            modeName += "Per X1000, ";
            break;
        case CMM_PERIOD_GATING_ON:
            modeName += "Per GateOn, ";
            break;
        case CMM_PERIOD_INVERT_GATE:
            modeName += "Per InvGate, ";
            break;
        case CMM_PULSE_WIDTH_GATING_ON:
            modeName += "Pw GateOn, ";
            break;
        case CMM_PULSE_WIDTH_INVERT_GATE:
            modeName += "Pw InvGate, ";
            break;
        case CMM_TIMING_MODE_INVERT_GATE:
            modeName += "T InvGate, ";
            break;
        case CMM_ENCODER_X2:
            modeName += "E X2, ";
            break;
        case CMM_ENCODER_X4:
            modeName += "E X4, ";
            break;
        case CMM_ENCODER_LATCH_ON_Z:
            modeName += "E LatchZ, ";
            break;
        case CMM_ENCODER_CLEAR_ON_Z:
            modeName += "E ClearZ, ";
            break;
        case CMM_ENCODER_NO_RECYCLE:
            modeName += "E NoRecycle, ";
            break;
        case CMM_ENCODER_RANGE_LIMIT_ON:
            modeName += "E Rangelimit, ";
            break;
        case CMM_ENCODER_Z_ACTIVE_EDGE:
            modeName += "Z Edge, ";
            break;
        default:
            //modeName += "Invalid RegType, ";
            break;
        }
    }
    int loc = modeName.lastIndexOf(",");
    return modeName.left(loc);
}

QString getTmrTypeName(TimerType tmrType)
{
    switch (tmrType) {
    case TMR_STANDARD:
        return "Standard";
        break;
    case TMR_ADVANCED:
        return "Advanced";
        break;
    default:
        return "Invalid Type";
        break;
    }
}

QString getChanTypeNames(DaqInChanType typeNum)
{
    int mask;
    DaqInChanType maskedVal;
    QString chanName;

    maskedVal = (DaqInChanType)0;
    for (int i = 0; i < 8; i++) {
        mask = pow(2, i);
        maskedVal = (DaqInChanType)(typeNum & (DaqInChanType)mask);
        switch (maskedVal) {
        case DAQI_ANALOG_DIFF:
            chanName += "AnalogDiff, ";
            break;
        case DAQI_ANALOG_SE:
            chanName += "AnalogSE, ";
            break;
        case DAQI_DIGITAL:
            chanName += "Digital, ";
            break;
        case DAQI_CTR16:
            chanName += "Counter16, ";
            break;
        case DAQI_CTR32:
            chanName += "Counter32, ";
            break;
        case DAQI_CTR48:
            chanName += "Counter48, ";
            break;
        case DAQI_DAC:
            chanName += "DAC, ";
            break;
        default:
            //chanName += "Invalid ChanType, ";
            break;
        }
    }
    int loc = chanName.lastIndexOf(",");
    return chanName.left(loc);
}

QString getChanOTypeNames(DaqOutChanType typeNum)
{
    int mask;
    DaqOutChanType maskedVal;
    QString chanName;

    maskedVal = (DaqOutChanType)0;
    for (int i = 0; i < 2; i++) {
        mask = pow(2, i);
        maskedVal = (DaqOutChanType)(typeNum & (DaqOutChanType)mask);
        switch (maskedVal) {
        case DAQO_ANALOG:
            chanName += "AnalogOut, ";
            break;
        case DAQO_DIGITAL:
            chanName += "DigitalOut, ";
            break;
        default:
            //chanName += "Invalid ChanType, ";
            break;
        }
    }
    int loc = chanName.lastIndexOf(",");
    return chanName.left(loc);
}

QString getRangeName(Range rangeVal)
{
    //rangeInt = static_cast<Range>(rangeVal);
    switch (rangeVal) {
    case BIP60VOLTS:
        return "Bip60Volts";
    case BIP30VOLTS:
        return "Bip30Volts";
    case BIP15VOLTS:
        return "Bip15Volts";
    case BIP20VOLTS:
        return "Bip20Volts";
    case BIP10VOLTS:
        return "Bip10Volts";
    case BIP5VOLTS:
        return "Bip5Volts";
    case BIP4VOLTS:
        return "Bip4Volts";
    case BIP2PT5VOLTS:
        return "Bip2Pt5Volts";
    case BIP2VOLTS:
        return "Bip2Volts";
    case BIP1PT25VOLTS:
        return "Bip1Pt25Volts";
    case BIP1VOLTS:
        return "Bip1Volts";
    case BIPPT625VOLTS:
        return "BipPt625Volts";
    case BIPPT5VOLTS:
        return "BipPt5Volts";
    case BIPPT25VOLTS:
        return "BipPt25Volts";
    case BIPPT125VOLTS:
        return "BipPt125Volts";
    case BIPPT2VOLTS:
        return "BipPt2Volts";
    case BIPPT1VOLTS:
        return "BipPt1Volts";
    case BIPPT078VOLTS:
        return "BipPt078Volts";
    case BIPPT05VOLTS:
        return "BipPt01Volts";
    case BIPPT01VOLTS:
        return "BipPt005Volts";
    case BIPPT005VOLTS:
        return "Bip1Pt67Volts";
    case BIP3VOLTS:
        return "Bip3Volts";
    case BIPPT312VOLTS:
        return "BipPt312Volts";
    case BIPPT156VOLTS:
        return "BipPt156Volts";
    case UNI60VOLTS:
        return "Uni60Volts";
    case UNI30VOLTS:
        return "Uni30Volts";
    case UNI15VOLTS:
        return "Uni15Volts";
    case UNI20VOLTS:
        return "Uni20Volts";
    case UNI10VOLTS:
        return "Uni10Volts";
    case UNI5VOLTS:
        return "Uni5Volts";
    case UNI4VOLTS:
        return "Uni4Volts";
    case UNI2PT5VOLTS:
        return "Uni2Pt5Volts";
    case UNI2VOLTS:
        return "Uni2Volts";
    case UNI1PT25VOLTS:
        return "Uni1Pt25Volts";
    case UNI1VOLTS:
        return "Uni1Volts";
    case MA0TO20:
        return "0to20ma";
    default:
        return "Invalid Range";
    }
}

QString getRangeNomo(Range rangeVal)
{
    double curRangeVolts;

    switch (rangeVal) {
    case BIP60VOLTS:
        curRangeVolts = 60;
        break;
    case BIP30VOLTS:
        curRangeVolts = 30;
        break;
    case BIP15VOLTS:
        curRangeVolts = 15;
        break;
    case BIP20VOLTS:
        curRangeVolts = 20;
        break;
    case BIP10VOLTS:
        curRangeVolts = 10;
        break;
    case BIP5VOLTS:
        curRangeVolts = 5;
        break;
    case BIP4VOLTS:
        curRangeVolts = 4;
        break;
    case BIP3VOLTS:
        curRangeVolts = 3;
        break;
    case BIP2PT5VOLTS:
        curRangeVolts = 2.5;
        break;
    case BIP2VOLTS:
        curRangeVolts = 2;
        break;
    case BIP1PT25VOLTS:
        curRangeVolts = 1.25;
        break;
    case BIP1VOLTS:
        curRangeVolts = 1;
        break;
    case BIPPT625VOLTS:
        curRangeVolts = 0.625;
        break;
    case BIPPT5VOLTS:
        curRangeVolts = 0.5;
        break;
    case BIPPT312VOLTS:
        curRangeVolts = 0.312;
        break;
    case BIPPT25VOLTS:
        curRangeVolts = 0.25;
        break;
    case BIPPT125VOLTS:
        curRangeVolts = 0.125;
        break;
    case BIPPT2VOLTS:
        curRangeVolts = 0.2;
        break;
    case BIPPT156VOLTS:
        curRangeVolts = 0.156;
        break;
    case BIPPT1VOLTS:
        curRangeVolts = 0.1;
        break;
    case BIPPT078VOLTS:
        curRangeVolts = 0.078;
        break;
    case BIPPT05VOLTS:
        curRangeVolts = 0.05;
        break;
    case BIPPT01VOLTS:
        curRangeVolts = 0.01;
        break;
    case BIPPT005VOLTS:
        curRangeVolts = 0.005;
        break;
    case UNI60VOLTS:
        curRangeVolts = 60;
        break;
    case UNI30VOLTS:
        curRangeVolts = 30;
        break;
    case UNI15VOLTS:
        curRangeVolts = 15;
        break;
    case UNI20VOLTS:
        curRangeVolts = 20;
        break;
    case UNI10VOLTS:
        curRangeVolts = 10;
        break;
    case UNI5VOLTS:
        curRangeVolts = 5;
        break;
    case MA0TO20:
        curRangeVolts = 0.02;
        break;
    default:
        curRangeVolts = 0;
        break;
    }
    const QChar plusMinus = 0x00B1;
    if (rangeVal < 100)
        return QString("%1%2V").arg(plusMinus).arg(curRangeVolts);
    else
        return QString("%1V").arg(curRangeVolts);
}

QString getAccessTypes(MemAccessType memAccess)
{
    QString accessDesc;

    if (memAccess == 0)
        return "None";
    else {
        if (memAccess & MA_READ)
            accessDesc = "Read, ";
        if (memAccess & MA_WRITE)
            accessDesc += "Write, ";
        if (accessDesc.length())
            return accessDesc.left(accessDesc.length() - 2);
        return "Invalid Access Type";
    }
}

QString getRegionNames(MemRegion memRegion)
{
    QString regionNames;

    if (memRegion == 0)
        return "None";
    else {
        if (memRegion & MR_CAL)
            regionNames = "Cal, ";
        if (memRegion & MR_USER)
            regionNames += "User, ";
        if (memRegion & MR_SETTINGS)
            regionNames += "Settings, ";
        if (memRegion & MR_RESERVED0)
            regionNames += "Reserved0, ";
        if (regionNames.length())
            return regionNames.left(regionNames.length() - 2);
        return "Invalid Region";
    }
}

CounterMeasurementMode getMeasModeFromListIndex(int listIndex)
{
    switch (listIndex) {
    case 0:
        return CMM_DEFAULT;
    case 1:
        return CMM_CLEAR_ON_READ;
    case 2:
        return CMM_COUNT_DOWN;
    case 3:
        return CMM_GATE_CONTROLS_DIR;
    case 4:
        return CMM_GATE_CLEARS_CTR;
    case 5:
        return CMM_GATE_TRIG_SRC;
    case 6:
        return CMM_OUTPUT_ON;
    case 7:
        return CMM_OUTPUT_INITIAL_STATE_HIGH;
    case 8:
        return CMM_NO_RECYCLE;
    case 9:
        return CMM_RANGE_LIMIT_ON;
    case 10:
        return CMM_GATING_ON;
    case 11:
        return CMM_INVERT_GATE;
    case 12:
        return CMM_LATCH_ON_INDEX;
    case 13:
        return CMM_PHB_CONTROLS_DIR;
    case 14:
        return CMM_DECREMENT_ON;
    case 15:
        return CMM_PERIOD_X10;
    case 16:
        return CMM_PERIOD_X100;
    case 17:
        return CMM_PERIOD_X1000;
    case 18:
        return CMM_PERIOD_GATING_ON;
    case 19:
        return CMM_PERIOD_INVERT_GATE;
    case 20:
        return CMM_PULSE_WIDTH_GATING_ON;
    case 21:
        return CMM_PULSE_WIDTH_INVERT_GATE;
    case 22:
        return CMM_TIMING_MODE_INVERT_GATE;
    case 23:
        return CMM_ENCODER_X2;
    case 24:
        return CMM_ENCODER_X4;
    case 25:
        return CMM_ENCODER_LATCH_ON_Z;
    case 26:
        return CMM_ENCODER_CLEAR_ON_Z;
    case 27:
        return CMM_ENCODER_NO_RECYCLE;
    case 28:
        return CMM_ENCODER_RANGE_LIMIT_ON;
    case 29:
        return CMM_ENCODER_Z_ACTIVE_EDGE;
     default:
        return CMM_DEFAULT;
    }
}
