#include "FreeRTOS.h"
#include "task.h"


void vApplicationTickHook( void )
{
    
}

void vApplicationStackOverflowHook( TaskHandle_t xTask, char *pcTaskName )
{
    (void)xTask;
    (void)pcTaskName;
}





