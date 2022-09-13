/**
 *\*\file myiic.h
 *\*\author Sakurashiling
 *\*\HardWareSite https://oshwhub.com/lin_xiandi/desk-aqm
 *\*\License Apache 2.0
 *\*\version v1.0
 **/

#ifndef __MYIIC_H
#define __MYIIC_H
#include "n32g430_gpio.h" 

//IIC Config

#define IIC_SCL_PORT			GPIOB
#define IIC_SCL_PIN				GPIO_PIN_13

#define IIC_SDA_PORT			GPIOB	
#define IIC_SDA_PIN				GPIO_PIN_14	

// //IO��������
#define SDA_IN()  {GPIOB->PMODE&=~(3<<(14*2));GPIOB->PMODE|=0<<2*14;}	//PB9����ģʽ
#define SDA_OUT() {GPIOB->PMODE&=~(3<<(14*2));GPIOB->PMODE|=1<<2*14;} //PB9���ģʽ
//IO��������	 
#define IIC_SCL_SET()					do{GPIO_Pins_Set(IIC_SCL_PORT,IIC_SCL_PIN);}while(0);
#define IIC_SCL_RESET() 			do{GPIO_Pins_Reset(IIC_SCL_PORT,IIC_SCL_PIN);}while(0);
#define IIC_SDA_SET()					do{GPIO_Pins_Set(IIC_SDA_PORT,IIC_SDA_PIN);}while(0);
#define IIC_SDA_RESET()				do{GPIO_Pins_Reset(IIC_SDA_PORT,IIC_SDA_PIN);}while(0);
#define READ_SDA()						GPIO_Input_Pin_Data_Get(IIC_SDA_PORT,IIC_SDA_PIN)


//IIC���в�������
void IIC_Init(void);                //��ʼ��IIC��IO��				 
void IIC_Start(void);				//����IIC��ʼ�ź�
void IIC_Stop(void);	  			//����IICֹͣ�ź�
void IIC_Send_Byte(u8 txd);			//IIC����һ���ֽ�
u8 IIC_Read_Byte(unsigned char ack);//IIC��ȡһ���ֽ�
u8 IIC_Wait_Ack(void); 				//IIC�ȴ�ACK�ź�
void IIC_Ack(void);					//IIC����ACK�ź�
void IIC_NAck(void);				//IIC������ACK�ź�

void IIC_Write_One_Byte(u8 daddr,u8 addr,u8 data);
u8 IIC_Read_One_Byte(u8 daddr,u8 addr);	  
#endif
















