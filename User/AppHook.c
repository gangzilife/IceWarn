#include "FreeRTOS.h"
#include "task.h"

#include "dhcp.h"
#include "stm32f4xx_hal.h"

void vApplicationTickHook( void )
{
    static uint32_t tick = 0;
    HAL_IncTick();
    tick++;
    if(tick >= 1000)
    {
        tick = 0;
        DHCP_time_handler();
    }

    
}

void vApplicationStackOverflowHook( TaskHandle_t xTask, char *pcTaskName )
{
    (void)xTask;
    (void)pcTaskName;
}





