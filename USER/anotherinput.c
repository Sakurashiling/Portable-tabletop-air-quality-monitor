/**
 *\*\file anotherinput.c
 *\*\author Sakurashiling
 *\*\HardWareSite https://oshwhub.com/lin_xiandi/desk-aqm
 *\*\License Apache 2.0
 *\*\version v1.0
 **/
#include "anotherinput.h"
#include "bsp_delay.h"
#include "n32g430_gpio.h"
#include "FreeRTOS.h"

bool ESPIN_flag = false;
bool Charging_State = false;

void anotherinput_Init(void)
{
    /* Define a structure of type GPIO_InitType */
    GPIO_InitType GPIO_InitStructure;
    /* Enable PMSWITCH related GPIO peripheral clock */
    RCC_AHB_Peripheral_Clock_Enable(anotherinput_CLK);
    /* Assign default value to GPIO_InitStructure structure */
    GPIO_Structure_Initialize(&GPIO_InitStructure);
    /* Select the GPIO pin to control */
    GPIO_InitStructure.Pin = ESPinput_PIN | CHARGEinput_PIN;
    /* Set pin mode to general push-pull output */
    GPIO_InitStructure.GPIO_Mode = GPIO_MODE_INPUT;
    /* Initialize GPIO */
    GPIO_Peripheral_Initialize(anotherinput_PORT, &GPIO_InitStructure);
}

void anotherinput_Read(void)
{
    if (GPIO_Input_Pin_Data_Get(anotherinput_PORT, ESPinput_PIN) == 1)
    {
        ESPIN_flag = true;
    }
    if (GPIO_Input_Pin_Data_Get(anotherinput_PORT, CHARGEinput_PIN) == 1)
    {
        Charging_State = true;
    }
    else
    {
        Charging_State = false;
    }
}
