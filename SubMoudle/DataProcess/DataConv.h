#ifndef _DATACONVERT_H__
#define _DATACONVERT_H__

#include "stdint.h"
#include "stdio.h"


/*喷洒参数配置结构体*/
typedef struct
{
	int TimeOfIce;
	int	TimeOfIceWarn;
	int	TimeOfManual;
	int	SprayInterval;
	int	JudgInterval;
	int	TimeOf10g;
	int	SnowLimit;
}SprayConf_t;

/*报警参数配置结构体*/
typedef struct
{
	signed char LowLiquidMask;
	signed char LowLiquidWarn;
	signed char LowLiquidRestore;
	signed char HighPressMask;
	signed char HighPressWarn;
	signed char HighPressRestore;
	signed char GasTempLowMask;
	signed char GasTempLowWarn;
	signed char GasTempLowRestore;
	signed char RoadTempMask;
	signed char RoadTempWarn;
	signed char RoadTempRestore;
	signed char RoadIceMask;
	signed char RoadIceWarn;
	signed char RoadIceRestore;
	signed char TrafficErrMask;
	signed char TrafficErrWarn;
	signed char TrafficErrRestore;
}AlarmConf_t;

typedef struct
{
	int operationpasswd;
	int autorun;
	int icewarnON;
}SysConf_t;

extern SprayConf_t  SprayConfig;
extern AlarmConf_t  AlarmConfig;
extern SysConf_t    SysConfig;

void parseNetMSG(uint8_t* buf,uint16_t len);

#endif