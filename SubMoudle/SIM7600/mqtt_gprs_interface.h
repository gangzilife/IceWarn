#ifndef __MQTT_GPRS_INTERFACE_H_
#define __MQTT_GPRS_INTERFACE_H_

#include "mqtt_interface.h"

/*
 * @brief MQTT MilliTimer handler
 * @note MUST BE register to your system 1m Tick timer handler
 */
//void MilliTimer_Handler(void);

/*
 * @brief Timer structure
 */
//typedef struct Timer Timer;
//struct Timer {
//	unsigned long systick_period;
//	unsigned long end_time;
//};

/*
 * @brief Network structure
 */

/*
 * @brief Timer function
 */
//void TimerInit(Timer*);
//char TimerIsExpired(Timer*);
//void TimerCountdownMS(Timer*, unsigned int);
//void TimerCountdown(Timer*, unsigned int);
//int TimerLeftMS(Timer*);

/*
 * @brief Network interface porting
 */
int gprs_read(Network*, unsigned char*, int, int);
int gprs_write(Network*, unsigned char*, int, int);
void gprs_disconnect(Network*);
void gprs_NewNetwork(Network* n, int sn);
int gprs_ConnectNetwork(Network*, char*, int);

#endif //__MQTT_INTERFACE_H_
