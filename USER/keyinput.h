/**
 *\*\file keyinput.h
 *\*\author Sakurashiling
 *\*\HardWareSite https://oshwhub.com/lin_xiandi/desk-aqm
 *\*\License Apache 2.0
 *\*\version v1.0
 **/

#ifndef __KEYINPUT_H__
#define __KEYINPUT_H__

#ifdef __cplusplus
extern "C"
{
#endif

#include "n32g430.h"

/** 按键引脚 **/
#define keyinput_PORT GPIOA               /* GPIO port */
#define keyinput_CLK RCC_AHB_PERIPH_GPIOA /* GPIO port clock */
#define keyinput_PINL GPIO_PIN_7         /* GPIO connected to the SCL clock line */
#define keyinput_PINR GPIO_PIN_6         /* GPIO connected to the SCL clock line */

/** 按钮控制宏 **/
#define keyinput_TOGGLE {keyinput_PORT->POD ^= keyinput_PIN;}

void KeyInput_Init(void);

#ifdef __cplusplus
}
#endif

#endif /* __KEYINPUT_H__ */
