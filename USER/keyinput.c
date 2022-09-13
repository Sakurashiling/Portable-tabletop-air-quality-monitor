/**
 *\*\file keyinput.c
 *\*\author Sakurashiling
 *\*\HardWareSite https://oshwhub.com/lin_xiandi/desk-aqm
 *\*\License Apache 2.0
 *\*\version v1.0
 **/

#include "keyinput.h"
#include "bsp_delay.h"
#include "n32g430_gpio.h"
#include "FreeRTOS.h"

void KeyInput_Init()
{
    /* Define a structure of type GPIO_InitType */
    GPIO_InitType GPIO_InitStructure;
    /* Enable PMSWITCH related GPIO peripheral clock */
    RCC_AHB_Peripheral_Clock_Enable(keyinput_CLK);
    /* Assign default value to GPIO_InitStructure structure */
    GPIO_Structure_Initialize(&GPIO_InitStructure);
    /* Select the GPIO pin to control */
    GPIO_InitStructure.Pin = keyinput_PINL | keyinput_PINR;
    /* Set pin mode to general push-pull output */
    GPIO_InitStructure.GPIO_Mode = GPIO_MODE_INPUT;
    /* Initialize GPIO */
    GPIO_Peripheral_Initialize(keyinput_PORT, &GPIO_InitStructure);
}
