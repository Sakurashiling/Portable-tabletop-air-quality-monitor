/**
 *\*\file pmswitch.c
 *\*\author Sakurashiling
 *\*\HardWareSite https://oshwhub.com/lin_xiandi/desk-aqm
 *\*\License Apache 2.0
 *\*\version v1.0
 **/

#include "pmswitch.h"

void PMS_Init()
{
    /* Define a structure of type GPIO_InitType */
    GPIO_InitType GPIO_InitStructure;
    /* Enable PMSWITCH related GPIO peripheral clock */
    RCC_AHB_Peripheral_Clock_Enable(PMSWITCH_CLK);
    /* Assign default value to GPIO_InitStructure structure */
    GPIO_Structure_Initialize(&GPIO_InitStructure);
    /* Select the GPIO pin to control */
    GPIO_InitStructure.Pin = PMSWITCH_PIN;
    /* Set pin mode to general push-pull output */
    GPIO_InitStructure.GPIO_Mode = GPIO_MODE_OUT_PP;
    /* Set the pin drive current to 4MA*/
    GPIO_InitStructure.GPIO_Current = GPIO_DS_2MA;
    /* Initialize GPIO */
    GPIO_Peripheral_Initialize(PMSWITCH_PORT, &GPIO_InitStructure);
    /* Turn off PMS SENSOR */
    PMSWITCH_OFF;
}

void PMSWITCH_Toggle(void)
{
    GPIO_Pin_Toggle(PMSWITCH_PORT, PMSWITCH_PIN);
}

void PMSWITCH_On(void)
{
    GPIO_Pins_Set(PMSWITCH_PORT, PMSWITCH_PIN);
}

void PMSWITCH_Off(void)
{
    GPIO_Pins_Reset(PMSWITCH_PORT, PMSWITCH_PIN);
}
