#ifndef UNITEST_H
#define UNITEST_H

typedef enum
{
    TYPE_UL_INFO = 0,
    TYPE_DEV_INFO = 1,
    TYPE_AI_INFO = 2,
    TYPE_AO_INFO = 3,
    TYPE_DIO_INFO = 4,
    TYPE_CTR_INFO = 5,
    TYPE_TMR_INFO = 6,
    TYPE_DAQI_INFO = 7,
    TYPE_DAQO_INFO = 8,
    TYPE_MEM_INFO = 9
}UtInfoTypes;

typedef enum
{
    FUNC_GROUP_DISC = 0,
    FUNC_GROUP_AIN = 1,
    FUNC_GROUP_AOUT = 2,
    FUNC_GROUP_DIN = 3,
    FUNC_GROUP_DOUT = 4,
    FUNC_GROUP_CTR = 5,
    FUNC_GROUP_MISC = 6,
    FUNC_GROUP_CONFIG = 7,
    FUNC_GROUP_DAQSCAN = 8,
    FUNC_GROUP_MEM = 9
}UtFunctionGroup;

typedef enum
{
    UL_AIN = 1,
    UL_AINSCAN = 2,
    UL_AIN_LOAD_QUEUE = 3,
    UL_AIN_SET_TRIGGER = 4,
    UL_DAQ_INSCAN = 5,
    UL_TIN = 6,
    UL_TINARRAY = 7
}UtAiFunctions;

typedef enum
{
    UL_AOUT = 1,
    UL_AOUT_SCAN = 2,
    UL_DAQ_OUTSCAN = 3,
    UL_AOUT_SET_TRIGGER = 5,
    UL_AOUTARRAY = 6
}UtAoFunctions;

typedef enum
{
    UL_D_CONFIG_PORT = 1,
    UL_D_CONFIG_BIT = 2,
    UL_D_IN = 3,
    UL_D_OUT = 4,
    UL_D_BIT_IN = 5,
    UL_D_BIT_OUT = 6,
    UL_D_INSCAN = 7,
    UL_D_OUTSCAN = 8,
    UL_D_INARRAY = 9,
    UL_D_OUTARRAY = 10
}UtDioFunctions;

typedef enum
{
    UL_C_SELECT = 1,
    UL_C_IN = 2,
    UL_C_LOAD = 3,
    UL_C_CLEAR = 4,
    UL_C_CONFIG_SCAN = 5,
    UL_C_INSCAN = 6,
    UL_TMR_OUT = 7
}UtCtrFunctions;

typedef enum
{
    UL_GET_INFO = 1,
    UL_GET_CONFIG = 2,
    UL_SET_CONFIG = 3
}UtInfoFunctions;

typedef enum
{
    UL_DISC = 0,
    UL_FLASH_LED = 1,
    UL_GET_ERR_MSG = 2,
    UL_GET_STATUS = 3,
    UL_SCAN_STOP = 4,
    UL_MEM_READ = 5,
    UL_MEM_WRITE = 6
}UtMiscFunctions;

#endif // UNITEST_H
