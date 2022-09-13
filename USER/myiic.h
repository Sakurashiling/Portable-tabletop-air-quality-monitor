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

// //IO方向设置
#define SDA_IN()  {GPIOB->PMODE&=~(3<<(14*2));GPIOB->PMODE|=0<<2*14;}	//PB9输入模式
#define SDA_OUT() {GPIOB->PMODE&=~(3<<(14*2));GPIOB->PMODE|=1<<2*14;} //PB9输出模式
//IO操作函数	 
#define IIC_SCL_SET()					do{GPIO_Pins_Set(IIC_SCL_PORT,IIC_SCL_PIN);}while(0);
#define IIC_SCL_RESET() 			do{GPIO_Pins_Reset(IIC_SCL_PORT,IIC_SCL_PIN);}while(0);
#define IIC_SDA_SET()					do{GPIO_Pins_Set(IIC_SDA_PORT,IIC_SDA_PIN);}while(0);
#define IIC_SDA_RESET()				do{GPIO_Pins_Reset(IIC_SDA_PORT,IIC_SDA_PIN);}while(0);
#define READ_SDA()						GPIO_Input_Pin_Data_Get(IIC_SDA_PORT,IIC_SDA_PIN)


//IIC所有操作函数
void IIC_Init(void);                //初始化IIC的IO口				 
void IIC_Start(void);				//发送IIC开始信号
void IIC_Stop(void);	  			//发送IIC停止信号
void IIC_Send_Byte(u8 txd);			//IIC发送一个字节
u8 IIC_Read_Byte(unsigned char ack);//IIC读取一个字节
u8 IIC_Wait_Ack(void); 				//IIC等待ACK信号
void IIC_Ack(void);					//IIC发送ACK信号
void IIC_NAck(void);				//IIC不发送ACK信号

void IIC_Write_One_Byte(u8 daddr,u8 addr,u8 data);
u8 IIC_Read_One_Byte(u8 daddr,u8 addr);	  
#endif
















