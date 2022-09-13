/**
 *\*\file bsp_delay.h
 *\*\author Sakurashiling
 *\*\HardWareSite https://oshwhub.com/lin_xiandi/desk-aqm
 *\*\License Apache 2.0
 *\*\version v1.0
 **/

#ifndef __BSP_DELAY_H__
#define __BSP_DELAY_H__

#ifdef __cplusplus
extern "C" {
#endif

#include "n32g430.h"

//void SysTick_Delay_Us( __IO uint32_t us);
//void SysTick_Delay_Ms( __IO uint32_t ms);


void delay_init(u8 SYSCLK);

void delay_us(u32 nus);
void delay_ms(u32 nms);
void delay_xms(u32 nms);


#ifdef __cplusplus
}
#endif

#endif /* __BSP_DELAY_H__ */

