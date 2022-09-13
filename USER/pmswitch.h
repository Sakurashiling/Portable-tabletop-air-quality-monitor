/**
 *\*\file pmswitch.h
 *\*\author Sakurashiling
 *\*\HardWareSite https://oshwhub.com/lin_xiandi/desk-aqm
 *\*\License Apache 2.0
 *\*\version v1.0
 **/

#ifndef __PMSWITCH_H__
#define __PMSWITCH_H__

#ifdef __cplusplus
extern "C" {
#endif

#include "n32g430.h"

/** PMS7003空气质量传感器引脚 **/
#define PMSWITCH_PORT    	GPIOB	    /* GPIO port */
#define PMSWITCH_CLK    	RCC_AHB_PERIPH_GPIOB	    /* GPIO port */
#define PMSWITCH_PIN		GPIO_PIN_12	                /* GPIO connected to the SCL clock line */

/** PMS7003控制宏 **/
#define PMSWITCH_ON					{PMSWITCH_PORT->PBSC = PMSWITCH_PIN;}
#define PMSWITCH_OFF				{PMSWITCH_PORT->PBC = PMSWITCH_PIN;}

void PMS_Init(void);
void PMSWITCH_Toggle(void);

#ifdef __cplusplus
}
#endif

#endif /* __PMSWITCH_H__ */


