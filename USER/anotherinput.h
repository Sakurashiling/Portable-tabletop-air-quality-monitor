/**
 *\*\file anotherinput.h
 *\*\author Sakurashiling
 *\*\HardWareSite https://oshwhub.com/lin_xiandi/desk-aqm
 *\*\License Apache 2.0
 *\*\version v1.0
 **/

#ifndef __ANOTHERINPUT_H__
#define __ANOTHERINPUT_H__

#ifdef __cplusplus
extern "C"
{
#endif

#include "n32g430.h"

/** 按键引脚 **/
#define anotherinput_PORT GPIOB                 /* GPIO port */
#define anotherinput_CLK RCC_AHB_PERIPH_GPIOB   /* GPIO port clock */
#define ESPinput_PIN GPIO_PIN_0                 /* GPIO connected to the SCL clock line */
#define CHARGEinput_PIN GPIO_PIN_1              /* GPIO connected to the SCL clock line */

void anotherinput_Init(void);
void anotherinput_Read(void);

#ifdef __cplusplus
}
#endif

#endif /* __ANOTHERINPUT_H__ */
