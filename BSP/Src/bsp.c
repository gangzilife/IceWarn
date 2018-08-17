

#include  "bsp_clock.h"
#include  "bsp_led.h"
#include  "bsp_can.h"
#include  "bsp_usart.h"
#include  "bsp_spi.h"
#include  "stm32f4xx_hal.h"




/*
*********************************************************************************************************
*                                               BSP_Init()
*
* Description : Initialize the Board Support Package (BSP).
*
* Argument(s) : none.
*
* Return(s)   : none.
*
* Caller(s)   : Application.
*
* Note(s)     : (1) This function SHOULD be called before any other BSP function is called.
*********************************************************************************************************
*/

void  BSP_Init (void)
{
    BSP_LED_Init();                                             /* Init LEDs.                                           */
    BSP_CAN_Init();
    BSP_USART_Init();
    BSP_SPI_Init();
}
