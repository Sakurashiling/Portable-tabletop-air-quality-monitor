
#include "pm25_usart.h"
#include "n32g430.h"
#include "stdio.h"
#include "string.h"
#include "bsp_delay.h"

#define LOG_USARTx USART1
#define USARTx_IRQn       USART1_IRQn
#define USARTx_IRQHandler USART1_IRQHandler
#define LOG_PERIPH RCC_APB2_PERIPH_USART1
#define LOG_GPIO GPIOA
#define LOG_PERIPH_GPIO RCC_AHB_PERIPH_GPIOA
#define LOG_TX_PIN GPIO_PIN_9
#define LOG_RX_PIN GPIO_PIN_10


PM_Sensor_DataStruct	PM_Sensor_Data;
uint8_t 	PM_Sensor_RxBuffer[50];
uint16_t    PM_Sensor_RxTimeOut = 0;
uint16_t    PM_Sensor_RxCount = 0;

FlagStatus  PM_Sensor_RxFinish = RESET; 

//=============================================================================
//Check_PMSensor_DataValid    //
//=============================================================================
ErrorStatus	Check_PMSensor_DataValid(void)
{
	uint16_t 	Cal_CheckSum;
	uint16_t 	Buffer_CheckSum;
	uint16_t 	Buffer_Len;
	uint8_t 	i;
	ErrorStatus Result = ERROR;

	if((PM_Sensor_RxBuffer[0] == 'B')&&(PM_Sensor_RxBuffer[1] == 'M'))
	{
		Buffer_Len = (uint16_t)((PM_Sensor_RxBuffer[2] << 8) | PM_Sensor_RxBuffer[3]);

		Buffer_CheckSum = (uint16_t)((PM_Sensor_RxBuffer[Buffer_Len + 2] << 8) | PM_Sensor_RxBuffer[Buffer_Len + 3]);

		Cal_CheckSum = 0;
		for(i=0;i<(Buffer_Len + 2);i++)
		{
			Cal_CheckSum += PM_Sensor_RxBuffer[i];
		}

		if(Cal_CheckSum == Buffer_CheckSum)
			Result = SUCCESS;
	}
	return Result;
}
//=============================================================================
//PMSensor_DataReflash    //
//=============================================================================
void PMSensor_DataReflash(void)
{
	uint16_t Buffer_Len;

	memset(&PM_Sensor_Data,0,(sizeof(PM_Sensor_Data) - 2)); //PM_Sensor_Data.PM2_5_Old should not set to zero
	
	Buffer_Len = (uint16_t)((PM_Sensor_RxBuffer[2] << 8) | PM_Sensor_RxBuffer[3]);
	if(Buffer_Len == 28)   //PMS1003/5003
	{
		PM_Sensor_Data.Buffer_Len = 28;
		PM_Sensor_Data.PM1_0_CF = (uint16_t)((PM_Sensor_RxBuffer[4]<<8) | PM_Sensor_RxBuffer[5]);
		PM_Sensor_Data.PM2_5_CF = (uint16_t)((PM_Sensor_RxBuffer[6]<<8) | PM_Sensor_RxBuffer[7]);
		PM_Sensor_Data.PM10_CF 	= (uint16_t)((PM_Sensor_RxBuffer[8]<<8) | PM_Sensor_RxBuffer[9]);
		PM_Sensor_Data.PM1_0 	= (uint16_t)((PM_Sensor_RxBuffer[10]<<8) | PM_Sensor_RxBuffer[11]);
		PM_Sensor_Data.PM2_5 	= (uint16_t)((PM_Sensor_RxBuffer[12]<<8) | PM_Sensor_RxBuffer[13]);
		PM_Sensor_Data.PM10 	= (uint16_t)((PM_Sensor_RxBuffer[14]<<8) | PM_Sensor_RxBuffer[15]);
		PM_Sensor_Data.Count0_3nm = (uint16_t)((PM_Sensor_RxBuffer[16]<<8) | PM_Sensor_RxBuffer[17]);
		PM_Sensor_Data.Count0_5nm = (uint16_t)((PM_Sensor_RxBuffer[18]<<8) | PM_Sensor_RxBuffer[19]);
		PM_Sensor_Data.Count1_0nm = (uint16_t)((PM_Sensor_RxBuffer[20]<<8) | PM_Sensor_RxBuffer[21]);
		PM_Sensor_Data.Count2_5nm = (uint16_t)((PM_Sensor_RxBuffer[22]<<8) | PM_Sensor_RxBuffer[23]);
		PM_Sensor_Data.Count5_0nm = (uint16_t)((PM_Sensor_RxBuffer[24]<<8) | PM_Sensor_RxBuffer[25]);
		PM_Sensor_Data.Count10nm = (uint16_t)((PM_Sensor_RxBuffer[26]<<8) | PM_Sensor_RxBuffer[27]);
		
	}
	else if(Buffer_Len == 20)// PMS3003
	{
		PM_Sensor_Data.Buffer_Len = 20;
		PM_Sensor_Data.PM1_0_CF = (uint16_t)((PM_Sensor_RxBuffer[4]<<8) | PM_Sensor_RxBuffer[5]);
		PM_Sensor_Data.PM2_5_CF = (uint16_t)((PM_Sensor_RxBuffer[6]<<8) | PM_Sensor_RxBuffer[7]);
		PM_Sensor_Data.PM10_CF 	= (uint16_t)((PM_Sensor_RxBuffer[8]<<8) | PM_Sensor_RxBuffer[9]);
		PM_Sensor_Data.PM1_0 	= (uint16_t)((PM_Sensor_RxBuffer[10]<<8) | PM_Sensor_RxBuffer[11]);
		PM_Sensor_Data.PM2_5 	= (uint16_t)((PM_Sensor_RxBuffer[12]<<8) | PM_Sensor_RxBuffer[13]);
		PM_Sensor_Data.PM10 	= (uint16_t)((PM_Sensor_RxBuffer[14]<<8) | PM_Sensor_RxBuffer[15]);
		PM_Sensor_Data.Count0_3nm = 0;
		PM_Sensor_Data.Count0_5nm = 0;
		PM_Sensor_Data.Count1_0nm = 0;
		PM_Sensor_Data.Count2_5nm = 0;
		PM_Sensor_Data.Count5_0nm = 0;
		PM_Sensor_Data.Count10nm = 0;
	}
}

void PMS7003_sysTick(void)
{
	//OS_TimeMS ++;   //   ++1us for os timer
	//==========================================================================
	if(PM_Sensor_RxTimeOut != 0x00) // timeout for PM data receive
	{
		PM_Sensor_RxTimeOut--;
	}
	else
	{
		if((PM_Sensor_RxCount)&&(PM_Sensor_RxBuffer[0] == 'B')&&(PM_Sensor_RxBuffer[1] == 'M'))
		{
			PM_Sensor_RxCount = 0;
			PM_Sensor_RxFinish = SET;
			USART_Interrput_Disable(USART1, USART_INT_RXDNE);
		}
		else
		{
			PM_Sensor_RxCount = 0;
		}
	}
}

void USARTx_IRQHandler(void)
{
	static uint8_t 		USART1_ByteData = 0;
    
    if (USART_Interrupt_Status_Get(USART1, USART_INT_RXDNE) != RESET)
    {
		
		USART1_ByteData = USART_Data_Receive(USART1);

		if(PM_Sensor_RxFinish == RESET)
		{
			PM_Sensor_RxBuffer[PM_Sensor_RxCount++] = USART1_ByteData;
			PM_Sensor_RxTimeOut = 20;
		}
    }
}

//任务里面去读取
PM_Sensor_DataStruct * read_PMS7003_data(void)
{
	if(PM_Sensor_RxFinish == SET)
	{
		if(Check_PMSensor_DataValid())
		{
			PMSensor_DataReflash();
			//LCD_Display(PM_Sensor_Data.PM2_5); // display PM2.5 on 	LCD
		}
		PM_Sensor_RxFinish = RESET;
		/* enable uart */
		USART_Interrput_Enable(LOG_USARTx, USART_INT_RXDNE);
		return &PM_Sensor_Data;
	}
	return NULL;
	
}

