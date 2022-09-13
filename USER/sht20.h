#ifndef __SHT20_H
#define __SHT20_H

#include "n32g430.h"
#include "myiic.h"
#include "bsp_delay.h"

/*SHT20*/
#define SHT20_ADDRESS 0X40            // SHT20 IIC地址
#define SHT20_Measurement_RH_HM 0XE5  // 读取湿度保持主机
#define SHT20_Measurement_T_HM 0XE3   // 读取温度保持主机
#define SHT20_Measurement_RH_NHM 0XF5 // 读取湿度非保持主机
#define SHT20_Measurement_T_NHM 0XF3  // 读取温度非保持主机
#define SHT20_READ_REG 0XE7           // 读用户寄存器
#define SHT20_WRITE_REG 0XE6          // 写用户寄存器
#define SHT20_SOFT_RESET 0XFE         // 软复位

int SHT20_Get_Data(u8 Cmd, uint16_t *temperature, uint16_t *humidity);

#endif
