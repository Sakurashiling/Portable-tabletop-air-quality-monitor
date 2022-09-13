/**
 *\*\file ADCinput.h
 *\*\author Sakurashiling
 *\*\HardWareSite https://oshwhub.com/lin_xiandi/desk-aqm
 *\*\License Apache 2.0
 *\*\version v1.0
 **/

#ifndef __ADCINPUT_H__
#define __ADCINPUT_H__

#ifdef __cplusplus
extern "C"
{
#endif

#include "n32g430.h"

#include "string.h"
#include "stdlib.h"
#include "stddef.h"
#include "stdbool.h"
#include "stdio.h"

#define VOLTAGE_FACTOR       1.07069F                 // 实际电压与表显电压的比值
#define VOLTAGE_ADC_CHANNEL  ADC_Channel_02_PA1   // 电压采样通道编号

void BT_ADC_Init(void);
void BT_ADC_Read(void);

#ifdef __cplusplus
}
#endif

#endif /* __ADCINPUT_H__ */
