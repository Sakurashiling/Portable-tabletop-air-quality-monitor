#include "sgp30.h"

void sgp30_iic_write(uint8_t addr, const uint8_t *buf, uint32_t len)
{
    int i;
    IIC_Start();
    IIC_Send_Byte(addr);
    IIC_Wait_Ack();
    for (i = 0; i < len; i++)
    {
        IIC_Send_Byte(buf[i]);
        IIC_Wait_Ack();
    }
    IIC_Stop();
}

void sgp30_iic_read(uint8_t addr, uint8_t *buf, uint32_t len)
{
    int i;
    IIC_Start();
    IIC_Send_Byte(addr);
    IIC_Wait_Ack();
    for (i = 0; i < len - 1; i++)
    {
        buf[i] = IIC_Read_Byte(1);
    }
    buf[i] = IIC_Read_Byte(0); // SGP30接收数据时候的最后一个字节不需要等待ACK
    IIC_Stop();
}

//////////////////////////////////////////////////////////////////////////////////////////
///
///       SGP30驱动代码
///
//////////////////////////////////////////////////////////////////////////////////////////
static uint8_t sgp30_checksum(const uint8_t *buf, uint32_t len);

// int sgp30_get_serial_id(uint8_t id[6])
// {
//     uint8_t buf[32];
//     uint8_t crc[3];

//     buf[0] = (SGP30_CMD_GET_SERIAL_ID & 0XFF00) >> 8;
//     buf[1] = (SGP30_CMD_GET_SERIAL_ID & 0X00FF);

//     if (sgp30_iic_write(SGP30_ADDR_WRITE, buf, 2) < 0)
//         return -1;

//     if (sgp30_iic_read(SGP30_ADDR_READ, buf, 9) < 0)
//         return -2;

//     crc[0] = buf[2];
//     crc[1] = buf[5];
//     crc[2] = buf[8];

//     id[0] = buf[0];
//     id[1] = buf[1];
//     id[2] = buf[3];
//     id[3] = buf[4];
//     id[4] = buf[6];
//     id[5] = buf[7];

//     if (
//         sgp30_checksum(&id[0], 2) != crc[0] ||
//         sgp30_checksum(&id[2], 2) != crc[1] ||
//         sgp30_checksum(&id[4], 2) != crc[2])
//         return -3;

//     return 0;
// }

uint8_t sgp30_checksum(const uint8_t *buf, uint32_t len)
{
    const uint8_t Polynomial = 0x31;
    uint8_t Initialization = 0XFF;
    uint8_t i = 0, k = 0;
    while (i < len)
    {
        Initialization ^= buf[i++];
        for (k = 0; k < 8; k++)
        {
            if (Initialization & 0X80)
                Initialization = (Initialization << 1) ^ Polynomial;
            else
                Initialization = (Initialization << 1);
        }
    }
    return Initialization;
}

void sgp30_soft_reset(void)
{
    uint8_t cmd = 0X06;
    sgp30_iic_write(0X00, &cmd, 1);
}

void sgp30_init(void)
{
    uint8_t buf[2];
    // 软件复位
    sgp30_soft_reset();
    // 等待复位完成
    delay_ms(50);
    buf[0] = (SGP30_CMD_INIT_AIR_QUALITY & 0XFF00) >> 8;
    buf[1] = (SGP30_CMD_INIT_AIR_QUALITY & 0X00FF);
    // 初始化控制测量参数
    sgp30_iic_write(SGP30_ADDR_WRITE, buf, 2);
}
// SGP30传感器读取函数，读取成功返回0，读取失败返回-1
int sgp30_read(uint16_t *CO2, uint16_t *TVOC)
{
    uint8_t buf[8] = {0};
    buf[0] = (SGP30_CMD_MEASURE_AIR_QUALITY & 0XFF00) >> 8;
    buf[1] = (SGP30_CMD_MEASURE_AIR_QUALITY & 0X00FF);
    // 启动空气质量测量
    sgp30_iic_write(SGP30_ADDR_WRITE, buf, 2);
    // 等待测量完成(10ms-12ms)
    delay_ms(12);
    // 读取收到的数据
    sgp30_iic_read(SGP30_ADDR_READ, buf, 6);
    // 校验CRC
    if (sgp30_checksum(&buf[3], 2) != buf[5])
        return -1;
    if (CO2 != NULL)
        *CO2 = (buf[0] << 8) | buf[1];
    if (TVOC != NULL)
        *TVOC = (buf[3] << 8) | buf[4];
    return 0;
}
