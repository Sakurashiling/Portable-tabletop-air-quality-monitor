#include "sht20.h"
// SHT20采用的是CRC8校验、模型为X8 + X5 +X4 +1,即CRC_MODEL = 0x131。
#define CRC_MODEL 0x131

u8 CRC_Check(u8 *ptr, u8 len, u8 checksum)
{
    u8 i;
    u8 crc = 0x00; //计算的初始crc值

    while (len--)
    {
        crc ^= *ptr++; //每次先与需要计算的数据异或,计算完指向下一数据

        for (i = 8; i > 0; --i) //下面这段计算过程与计算一个字节crc一样
        {
            if (crc & 0x80)
            {
                crc = (crc << 1) ^ CRC_MODEL;
            }
            else
                crc = (crc << 1);
        }
    }

    if (checksum == crc)
    {
        return 0;
    }
    else
        return 1;
}

int SHT20_Get_Data(u8 Cmd, uint16_t *temperature, uint16_t *humidity)
{
    uint16_t data;
    u8 addr, checksum;
    u8 buf[2];

    addr = SHT20_ADDRESS << 1; // IIC地址是7bit，这里需要左移1位，bit0：1-读	0-写

    IIC_Start(); //起始信号

    IIC_Send_Byte(addr); //发送设备地址(写)
    IIC_Wait_Ack();      //等待应答

    IIC_Send_Byte(Cmd); //发送命令
    IIC_Wait_Ack();

    IIC_Start(); //重启信号

    IIC_Send_Byte(addr + 1); //发送设备地址(读)
    IIC_Wait_Ack();

    delay_ms(70); //等待测量

    buf[0] = IIC_Read_Byte(1); //数据高位
    // IIC_Ack();                //产生ACK应答
    buf[1] = IIC_Read_Byte(1); //数据低位
    // IIC_Ack();                //产生ACK应答  如果这里不应答  则不传送校验位

    checksum = IIC_Read_Byte(0); //校验位
    // IIC_NAck();                 //不应答

    IIC_Stop(); //结束信号

    data = (buf[0] << 8) + buf[1];

    if (CRC_Check(buf, 2, checksum) == 0) //校验
    {
        if (Cmd == SHT20_Measurement_T_HM)
        {
            *temperature = (175.72 * data / 65536 - 46.85) * 10; //温度计算公式
        }
        else
            *humidity = (125.0 * data / 65536 - 6.0) * 10; //湿度计算公式

        return 0;
    }
    else
        return -1; //校验不通过返回-1
}
