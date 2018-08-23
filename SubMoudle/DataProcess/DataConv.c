#include "DataConv.h"
#include "DataPub.h"
/*通过查表法查找每个ID的处理方法*/
/*函数void(func11)(DataType_t c[], int d)为命令ID的处理接口，需要添加新的ID时，直接在表中添加相应的ID和处理方法即可*/
/*两个参数DataType_t c[]数组为该命令ID下对于的参数项的数组，每一项是一个ID对应一个ID值，int d为数组项的个数*/



static int alarmarray[18];


SprayConf_t  SprayConfig = { 0 };
AlarmConf_t  AlarmConfig = { 0 };
SysConf_t    SysConfig = { 0 };

typedef struct
{
	int para_id;
	int para_data;
}DataType_t;

typedef void(*FunType)(DataType_t c[],int d);

typedef struct
{
	int command;
	int max_para;
	FunType Func;
}ParseMSG_t;

/*为什么“多此一举”写这个方法，为了把传递下来的负数变为负数，因为之前的数据解析所有的数都会当成int类型解析，
但一些char类型的负数仍然会是正数，所以“多此一举”*/
static void async_alarmconfig(void)
{
	AlarmConfig.LowLiquidMask = alarmarray[0];
	AlarmConfig.LowLiquidWarn = alarmarray[1];
	AlarmConfig.LowLiquidRestore = alarmarray[2];
	AlarmConfig.HighPressMask = alarmarray[3];
	AlarmConfig.HighPressWarn = alarmarray[4];
	AlarmConfig.HighPressRestore = alarmarray[5];
	AlarmConfig.GasTempLowMask = alarmarray[6];
	AlarmConfig.GasTempLowWarn = alarmarray[7];
	AlarmConfig.GasTempLowRestore = alarmarray[8];
	AlarmConfig.RoadTempMask = alarmarray[9];
	AlarmConfig.RoadTempWarn = alarmarray[10];
	AlarmConfig.RoadTempRestore = alarmarray[11];
	AlarmConfig.RoadIceMask = alarmarray[12];
	AlarmConfig.RoadIceWarn = alarmarray[13];
	AlarmConfig.RoadIceRestore = alarmarray[14];
	AlarmConfig.TrafficErrMask = alarmarray[15];
	AlarmConfig.TrafficErrWarn = alarmarray[16];
	AlarmConfig.TrafficErrRestore = alarmarray[17];
}

/*SetOperPasswd*/
static void(func11)(DataType_t c[], int d)
{
	if ((c[0].para_id != 1) || (c[0].para_data > 999999))
		return;
	SysConfig.operationpasswd = c[0].para_data;
	//publish
    generalACK();
	return;
}

/*GetOperPasswd*/
static void(func12)(DataType_t c[], int d)
{
	//publish
    publish_passwd();
	return;
}

/*SetAutoRun*/
static void(func13)(DataType_t c[], int d)
{
	if (c[0].para_id != 1 )
		return;
	SysConfig.autorun = c[0].para_data;
	//publish
    generalACK();
	return;
}

/*GetAutoRun*/
static void(func14)(DataType_t c[], int d)
{
	//publish
    publish_autorun();
	return;
}

/*ManualOneTime*/
static void(func15)(DataType_t c[], int d)
{
	//手动操作一次
    generalACK();
	return;
}

/*IceWarnON*/
static void(func16)(DataType_t c[], int d)
{
	if (c[0].para_id != 1)
		return;
	SysConfig.icewarnON = c[0].para_data;
	//publish
    generalACK();
	return;
}

/*GetIceWarn*/
static void(func17)(DataType_t c[], int d)
{
	//publish
    publish_icewarn();
	return;
}

/*SetSprayPara*/
static void(func18)(DataType_t c[], int d)
{
	int* p = (int*)&SprayConfig;
	printf("func18 the num of para = %d\n",d);
	for (int i = 0; i < d; i++)
	{
		printf("id = 0x%x , data = 0x%x\n", c[i].para_id, c[i].para_data);
		if (c[i].para_id > sizeof(SprayConfig) / sizeof(int))  //ID超出参数ID的范围，如果强制赋值，会访问非法内存
			continue;
		p[c[i].para_id - 1] = c[i].para_data;
	}
    generalACK();
	return;
}

/*SetAlarmPara*/
static void(func19)(DataType_t c[], int d)
{
	printf("func19 the num of para = %d\n", d);
	for (int i = 0; i < d; i++)
	{
		printf("id = 0x%x , data = 0x%x\n", c[i].para_id, c[i].para_data);
		if (c[i].para_id > sizeof(alarmarray) / sizeof(int))  //ID超出参数ID的范围，如果强制赋值，会访问非法内存
			continue;
		alarmarray[c[i].para_id - 1] = c[i].para_data;
	}
	async_alarmconfig();
    generalACK();
	return;
}

/*GetDevicePara*/
static void(func20)(DataType_t c[], int d)
{
	//publish
    publish_devicepara();
	return;
}

/*GetDeviceStatus*/
static void(func21)(DataType_t c[], int d)
{
	//publish
	return;
}


const static ParseMSG_t FuncArray[] =
{
	{ 11,1 ,func11 },
	{ 12,1 ,func12 },
	{ 13,1 ,func13 },
	{ 14,1 ,func14 },
	{ 15,1 ,func15 },
	{ 16,1 ,func16 },
	{ 17,1 ,func17 },
	{ 18,7 ,func18 },
	{ 19,18,func19 },
	{ 20,1 ,func20 },
	{ 21,1 ,func21 },
};

static uint8_t get_paranum(int command_id)
{
	for (int i = 0; i < sizeof(FuncArray) / sizeof(ParseMSG_t); i++)
	{
		if (FuncArray[i].command == command_id)
			return FuncArray[i].max_para;
	}
	return 1;
}

int device_id = 60568;
int packhead_len = 6;


static uint8_t get_datalen(uint8_t type)
{
	uint8_t ret = 0;
	switch (type)
	{
	case 1: ret = 1; break;
	case 2: ret = 2; break;
	case 3: ret = 4; break;
	case 4: ret = 4; break;
	case 5: ret = 4; break;
	case 6: ret = 8; break;
	case 7:
	case 8:
	case 9:
	default: break;
	}
	return ret;
}


static void dealMSG(int command_id, DataType_t para[] , int para_num)
{
	for (int i = 0; i < sizeof(FuncArray) / sizeof(ParseMSG_t); i++)
	{
		if (FuncArray[i].command == command_id)
		{
			if (para_num > FuncArray[i].max_para)
				return;
			FuncArray[i].Func(para, para_num);
		}	
	}
}

void parseNetMSG(uint8_t* buf, uint16_t len)
{
	/*用于存放解析出来的参数的数组*/
	DataType_t data[20] = { 0 };
	int index = 0;

	uint8_t* pbuf = buf;
	int id = pbuf[2] | pbuf[3] << 8 | pbuf[4] << 16 | pbuf[5] << 24;
	if (id != device_id)
		return;
	pbuf += packhead_len;

	uint8_t command_len = get_datalen(pbuf[0]);
	int command_id = 0;
	int parameter_id = 0;
	int parameter_data = 0;
	for (int i = 0; i < command_len; i++)
	{
		command_id |= pbuf[1 + i] << 8 * i;
	}
	pbuf += (command_len + 1);
	uint8_t num_para = get_paranum(command_id);
	//printf("command = %d , the command have %d parameters\n", command_id, num_para);

	/*循环解析命令ID后的参数，并把相关参数ID和对于的参数值放在数组中*/
	for (index = 0; index < num_para; index++)
	{
		/*解析参数ID*/
		command_len = get_datalen(pbuf[0]);
		parameter_id = 0;
		for (int i = 0; i < command_len; i++)
		{
			parameter_id |= pbuf[1 + i] << 8 * i;
		}
		pbuf += (command_len + 1);
		data[index].para_id = parameter_id;

		/*解析参数值*/
		command_len = get_datalen(pbuf[0]);
		parameter_data = 0;
		for (int j = 0; j < command_len; j++)
		{
			parameter_data |= pbuf[1 + j] << 8 * j;
		}
		pbuf += (command_len + 1);
		data[index].para_data = parameter_data;

		
		if ((buf + len - pbuf) < 4)
		{
			index++;
			break;
		}				
	}
	dealMSG(command_id, data, index);
}

///*下发的float数据是内存值，解析时把内存的数据转换成了一个int数据，现根据int转换成float*/
//typedef union
//{
//	float float_data;
//	uint8_t byte[4];
//}Fdata_t;
//static float floatmemTofloat(int mem)
//{
//	Fdata_t data = {0};
//	data.byte[0] = mem & 0xFF;
//	data.byte[1] = (mem >> 8) & 0xFF;
//	data.byte[2] = (mem >> 16) & 0xFF;
//	data.byte[3] = (mem >> 24) & 0xFF;
//	return data.float_data;
//}

