#include "algorithm.h"
#include <stdio.h>
#include <math.h>

float Get_Dewpoint(float temp,float humi)
{
    float X = (float)humi / 100;
    float Y = 0;


    float A = 273.16 / (273.15 + temp);
    float B = (273.15 + temp) / 273.16;


    Y = 10.79574 * (1 - A) - 5.02800 * log10(B) + 1.50475 * pow(10, -4) * (1 - pow(10, (-8.2969*(B - 1)))) + 0.42873 * pow(10, -3)*(pow(10, 4.76955 *(1 - A)) - 1) + 0.78614;

    float top = 243.92* (log10(X) + Y - 0.78588);
    float low = 7.69 - (log10(X) + Y - 0.78588);

    //printf("温度:%f  湿度:%f  下的露点温度为%f\n",temp,humi, top / low);   
    return (top / low);
}

typedef enum
{
	ERR_NONE,
	ERR_MALLOC,
	ERR_NULL
}LIST_STATUS_t;


typedef struct Node
{
	float val;
	struct Node* pNext;
}NODE, *PNODE;

/******************************************************************************/
static volatile int  datainterval = 1;//数据的间隔，默认情况下，1分钟一条数据
static volatile int  dataCount = 60;  //数据的数量，默认情况下有60条数据
static PNODE pList = NULL;            //用来计算温度变化率的链表
/******************************************************************************/
static PNODE pRoad = NULL;            //用来计算路面状态的链表
static volatile int  dataCount_RoadCond = 20;  //数据的数量，默认情况下有60条数据
/******************************************************************************/

static PNODE Create_List(void)
{
	PNODE pHead;
	pHead = (PNODE)malloc(sizeof(NODE));
	if (pHead != NULL)
		pHead->pNext = NULL;		
	return pHead;
}

/*
static void List_List(PNODE pList)
{
	PNODE p = pList->pNext;
	while (p != NULL)
	{
		printf("%f\n", p->val);
		p = p->pNext;
	}
}*/

static int InsertListFromTail(PNODE pList, float val)
{

	PNODE p = pList;

	while (p->pNext)
	{
		p = p->pNext;
	}

	PNODE pNew = (PNODE)malloc(sizeof(NODE));
	if (pNew == NULL)
		return ERR_MALLOC;
	pNew->val = val;
	pNew->pNext = p->pNext;
	p->pNext = pNew;
	return ERR_NONE;
}


static void DeleteListFromHead(PNODE pList)
{
	if (pList->pNext == NULL)
		return;
	PNODE pDelete = pList->pNext;
	pList->pNext = pDelete->pNext;
	free(pDelete);
	pDelete = NULL;
}


static int  Num_List(PNODE pList)
{
	int ret = 0;
	PNODE p = pList;
	while (p->pNext != NULL)
	{
		ret++;
		p = p->pNext;
	}
	return ret;
}

/*
static bool Is_Empty(PNODE pList)
{
	PNODE p = pList;
	if (p->pNext == NULL)
		return true;
	else
		return false;
}*/

static int add_to_list(PNODE pList, float val)
{
	if (InsertListFromTail(pList, val) != ERR_NONE)
		return 1;
	int num = Num_List(pList);
	if (num > dataCount)
	{
		DeleteListFromHead(pList);
	}
	return 0;
}

static float get_x_average(int num)
{
	float x_average = 0;
	float x_sum = 0;
	for (int i = 0; i < num; i++)
	{
		x_sum += i * datainterval;
	}
	x_average = x_sum / num;
	return x_average;
}

/*链表的功能实现两个模块，此处为路面状态模块*/
/*************************************************************************/

int InitRoadCondModule(void)
{
	pRoad = Create_List();
	if (pRoad == NULL)
		return 1;
	else
		return 0;
}

int AddDataToRoadCond(float value)
{
	return add_to_list(pRoad, value);
}

void SetDataNum_RoadCond(int num)
{
	dataCount_RoadCond = num;
}

int GetDataNum_RoadCond(void)
{
	return dataCount_RoadCond ;
}
/****************************************************************************
10 干
15 潮
20 湿
25 潮含融雪剂 潮含融雪剂 潮含融雪剂 潮含融雪剂
30 湿含融雪剂 湿含融雪剂 湿含融雪剂 湿含融雪剂
35 冰
40 雪
45 霜
***************************************************************************/
unsigned char RoadCond_Ice(void)
{    
	int num = Num_List(pRoad);  //判断当前数据的数量，如果数量不够，则直接输出0,表示温度没有变化
    //printf("num = %d",num);
	if (num < dataCount_RoadCond - 5)  //数据的数量不够
		return 0;  
    
    int times = 0;
    while(pRoad->pNext != NULL)
    {
        pRoad = pRoad->pNext;
        if(pRoad->val > 35)              //如果路面状态已经结冰，则times++，计算所有结冰的次数
            times++;
    }
    if(times > (dataCount_RoadCond - 5)/2)
        return 1;
    else
        return 0;
}


/*************************************************************************/
/*链表的功能实现两个模块，此处为结冰预警模块*/
int InitIcewarnModule(void)
{
	pList = Create_List();
	if (pList == NULL)
		return 1;
	else
		return 0;
}

int AddDataToModule(float value)
{
	return add_to_list(pList, value);
}

void SetDataNum(int num)
{
	dataCount = num;
}

int GetDataNum(void)
{
	return dataCount ;
}

void SetDataInterval(int interval)
{
	datainterval = interval;
}

int GetDataInterval(void)
{
	return datainterval;
}

float get_u(void)
{
	int num = Num_List(pList);  //判断当前数据的数量，如果数量不够，则直接输出0,表示温度没有变化
    //printf("num = %d",num);
	if (num < dataCount)
		return 0;
	float x_average = get_x_average(num);
	float y_average = 0;
	float y_sum = 0;

	PNODE p = pList;
	while (p->pNext)
	{
		p = p->pNext;
		y_sum += p->val;
	}
	y_average = y_sum / num;

	p = pList;
	int count = 0;
	float molecular = 0;
	float denominator = 0;
	while (p->pNext)
	{
		p = p->pNext;
		molecular += (count - x_average)*(p->val - y_average);
		denominator += (count - x_average)*(count - x_average);
		count += datainterval;
	}
	return  molecular / denominator;
}
