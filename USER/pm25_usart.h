
#ifndef __PM25_USART_H__
#define __PM25_USART_H__

#include "n32g430.h"

typedef struct
{
	uint16_t Buffer_Len;
	uint16_t PM1_0_CF;
	uint16_t PM2_5_CF;
	uint16_t PM10_CF;
	uint16_t PM1_0;
	uint16_t PM2_5;
	uint16_t PM10;
	uint16_t Count0_3nm;
	uint16_t Count0_5nm;
	uint16_t Count1_0nm;
	uint16_t Count2_5nm;
	uint16_t Count5_0nm;
	uint16_t Count10nm;
}PM_Sensor_DataStruct;

void PMS7003_sysTick(void);

PM_Sensor_DataStruct * read_PMS7003_data(void);

#endif
