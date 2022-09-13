#include "sht20.h"
// SHT20���õ���CRC8У�顢ģ��ΪX8 + X5 +X4 +1,��CRC_MODEL = 0x131��
#define CRC_MODEL 0x131

u8 CRC_Check(u8 *ptr, u8 len, u8 checksum)
{
    u8 i;
    u8 crc = 0x00; //����ĳ�ʼcrcֵ

    while (len--)
    {
        crc ^= *ptr++; //ÿ��������Ҫ������������,������ָ����һ����

        for (i = 8; i > 0; --i) //������μ�����������һ���ֽ�crcһ��
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

    addr = SHT20_ADDRESS << 1; // IIC��ַ��7bit��������Ҫ����1λ��bit0��1-��	0-д

    IIC_Start(); //��ʼ�ź�

    IIC_Send_Byte(addr); //�����豸��ַ(д)
    IIC_Wait_Ack();      //�ȴ�Ӧ��

    IIC_Send_Byte(Cmd); //��������
    IIC_Wait_Ack();

    IIC_Start(); //�����ź�

    IIC_Send_Byte(addr + 1); //�����豸��ַ(��)
    IIC_Wait_Ack();

    delay_ms(70); //�ȴ�����

    buf[0] = IIC_Read_Byte(1); //���ݸ�λ
    // IIC_Ack();                //����ACKӦ��
    buf[1] = IIC_Read_Byte(1); //���ݵ�λ
    // IIC_Ack();                //����ACKӦ��  ������ﲻӦ��  �򲻴���У��λ

    checksum = IIC_Read_Byte(0); //У��λ
    // IIC_NAck();                 //��Ӧ��

    IIC_Stop(); //�����ź�

    data = (buf[0] << 8) + buf[1];

    if (CRC_Check(buf, 2, checksum) == 0) //У��
    {
        if (Cmd == SHT20_Measurement_T_HM)
        {
            *temperature = (175.72 * data / 65536 - 46.85) * 10; //�¶ȼ��㹫ʽ
        }
        else
            *humidity = (125.0 * data / 65536 - 6.0) * 10; //ʪ�ȼ��㹫ʽ

        return 0;
    }
    else
        return -1; //У�鲻ͨ������-1
}
