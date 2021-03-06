

#ifndef  __BSP_CLOCK_H__
#define  __BSP_CLOCK_H__

#include <stdint.h>
#include <assert.h>

#ifdef __cplusplus
extern  "C" {                                  /* See Note #1.                                         */
#endif

                                                                /* System clock IDs.                                    */
typedef  enum  bsp_clk_id {
    BSP_CLK_ID_SYSCLK,
    BSP_CLK_ID_HCLK,
    BSP_CLK_ID_PCLK1,
    BSP_CLK_ID_PCLK2,
} BSP_CLK_ID;


/*
*********************************************************************************************************
*                                          GLOBAL VARIABLES
*********************************************************************************************************
*/


/*
*********************************************************************************************************
*                                         FUNCTION PROTOTYPES
*********************************************************************************************************
*/

void        BSP_SystemClkCfg(void);
uint32_t    BSP_ClkFreqGet  (BSP_CLK_ID  clk_id);


/*
*********************************************************************************************************
*                                   EXTERNAL C LANGUAGE LINKAGE END
*********************************************************************************************************
*/

#ifdef __cplusplus
}                                              /* End of 'extern'al C lang linkage.                    */
#endif


/*
*********************************************************************************************************
*                                          CONFIGURATION ERRORS
*********************************************************************************************************
*/


/*
*********************************************************************************************************
*                                              MODULE END
*********************************************************************************************************
*/

#endif                                                          /* End of module include.                               */
