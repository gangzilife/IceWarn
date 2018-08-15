#ifndef __ALGORITHM_H__
#define __ALGORITHM_H__

#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>

extern int InitIcewarnModule(void);
extern int AddDataToModule(float value);

extern void SetDataNum(int num);
extern int GetDataNum(void);

extern void SetDataInterval(int interval);
extern int GetDataInterval(void);

extern float get_u(void);


extern float Get_Dewpoint(float temp,float humi);

#endif