#include "n32g430.h"
#include "oled.h"
#include <stdio.h>
#include <string.h>
#include "i2c_drv.h"
#include "oledfont.h"

uint8_t OLED_GRAM[128][8] = {0};

/**
 *\*\name   Out_Oled.
 *\*\fun    write data or command function.
 *\*\param   cmd:  MOC_Command or MOC_Data. write data or write command
            data: data needed to write
 *\*\return none.
**/
void Out_Oled(uint8_t cmd, uint8_t data)
{
    uint8_t buffer[2];
    buffer[0] = cmd;
    buffer[1] = data;

    i2c_master_send(buffer, 2);
}
/**
 *\*\name   oled_init.
 *\*\fun    oled configure initilization function.
 *\*\param   none.
 *\*\return none.
 **/
void oled_init(void)
{
    Out_Oled(MOC_Command, 0xAE); //�ر���ʾ
    Out_Oled(MOC_Command, 0xD5); //����ʱ�ӷ�Ƶ����,��Ƶ��
    Out_Oled(MOC_Command, 80);   //[3:0],��Ƶ����;[7:4],��Ƶ��

    Out_Oled(MOC_Command, 0xA8); //��������·��
    Out_Oled(MOC_Command, 0X3F); //Ĭ��0X3F(1/64)

    Out_Oled(MOC_Command, 0xD3); //������ʾƫ��
    Out_Oled(MOC_Command, 0X00); //Ĭ��Ϊ0

    Out_Oled(MOC_Command, 0x40); //������ʾ��ʼ�� [5:0],����.

    Out_Oled(MOC_Command, 0x8D); //��ɱ�����
    Out_Oled(MOC_Command, 0x14); // bit2������/�ر�
    Out_Oled(MOC_Command, 0x20); //�����ڴ��ַģʽ
    Out_Oled(MOC_Command, 0x02); //[1:0],00���е�ַģʽ;01���е�ַģʽ;10,ҳ��ַģʽ;Ĭ��10;
    Out_Oled(MOC_Command, 0xA1); //���ض�������,bit0:0,0->0;1,0->127;
    Out_Oled(MOC_Command, 0xC0); //����COMɨ�跽��;bit3:0,��ͨģʽ;1,�ض���ģʽ COM[N-1]->COM0;N:����·��

    Out_Oled(MOC_Command, 0xDA); //����COMӲ����������
    Out_Oled(MOC_Command, 0x12); //[5:4]����

    Out_Oled(MOC_Command, 0x81); //�Աȶ�����
    Out_Oled(MOC_Command, 0xEF); // 1~255;Ĭ��0X7F (��������,Խ��Խ��)
    Out_Oled(MOC_Command, 0xD9); //����Ԥ�������
    Out_Oled(MOC_Command, 0xf1); //[3:0],PHASE 1;[7:4],PHASE 2;
    Out_Oled(MOC_Command, 0xDB); //����VCOMH ��ѹ����
    Out_Oled(MOC_Command, 0x30); //[6:4] 000,0.65*vcc;001,0.77*vcc;011,0.83*vcc;

    Out_Oled(MOC_Command, 0xA4); //ȫ����ʾ����;bit0:1,����;0,�ر�;(����/����)
    Out_Oled(MOC_Command, 0xA6); //������ʾ��ʽ;bit0:1,������ʾ;0,������ʾ
    Out_Oled(MOC_Command, 0xAF); //������ʾ
}
void OLED_Brightness(int value)
{
    u8 i;
    for (i = 0; i < 8; i++)
    {
        Out_Oled(0X80 + value, 0xEF); // 1~255;Ĭ��0X7F (��������,Խ��Խ��)
    }
}
/**
 *\*\name   OLED_Refresh_Gram.
 *\*\fun    Reresh Oled Gram function,and display on the oled.
 *\*\param   none.
 *\*\return none.
 **/
void OLED_Refresh_Gram(void)
{
    u8 i, n;
    for (i = 0; i < 8; i++)
    {
        Out_Oled(MOC_Command, 0xb0 + i);
        Out_Oled(MOC_Command, 0x00);
        Out_Oled(MOC_Command, 0x10);
        for (n = 0; n < 128; n++)
            Out_Oled(MOC_Data, OLED_GRAM[n][i]);
    }
}
/**
 *\*\name   OLED_Refresh_Gram.
 *\*\fun    Clear Oled Gram function,but it will not display on the oled immediately.
 *\*\param   none.
 *\*\return none.
 **/
void OLED_Clear(void)
{
    u8 i, n;
    for (i = 0; i < 8; i++)
        for (n = 0; n < 128; n++)
            OLED_GRAM[n][i] = 0X00;
}

/**
 *\*\name   OLED_DrawPoint.
 *\*\fun    Draw a point on the Oled,but it will not display on the oled immediately.
 *\*\param   x: The X-axis ,for 128x64 oled ,it ranges 0-127
            y: The Y-axis ,for 128x64 oled ,it ranges 0-63
            t: 1 drow point. 0,drow point without
 *\*\return none.
**/
void OLED_DrawPoint(u8 x, u8 y, u8 t)
{
    u8 pos, bx, temp = 0;
    if (x > 127 || y > 63)
        return; //?????.
    pos = 7 - y / 8;
    bx = y % 8;
    temp = 1 << (7 - bx);
    if (t)
        OLED_GRAM[x][pos] |= temp;
    else
        OLED_GRAM[x][pos] &= ~temp;
}
/**
 *\*\name   OLED_ShowChar.
 *\*\fun    Display a character.
 *\*\param   x: The X-axis ,for 128x64 oled ,it ranges 0-127
             y: The Y-axis ,for 128x64 oled ,it ranges 0-63
           chr: the character needed to display, for esample 'x','1' etc..
          size: the front size, support 12,16,20.
          mode: 1 show a character. 0,Display a character without.
 *\*\return none.
**/
void OLED_ShowChar(u8 x, u8 y, u8 chr, u8 size, u8 mode)
{
    u8 temp, t, t1;
    u8 y0 = y;
    u8 csize = (size / 8 + ((size % 8) ? 1 : 0)) * (size / 2);
    chr = chr - ' ';
    for (t = 0; t < csize; t++)
    {
        if (size == 12)
            temp = asc2_1206[chr][t];
        else if (size == 16)
            temp = asc2_1608[chr][t];
        else if (size == 24)
            temp = asc2_2412[chr][t];
        else
            return;
        for (t1 = 0; t1 < 8; t1++)
        {
            if (temp & 0x80)
                OLED_DrawPoint(x, y, mode);
            else
                OLED_DrawPoint(x, y, !mode);
            temp <<= 1;
            y++;
            if ((y - y0) == size)
            {
                y = y0;
                x++;
                break;
            }
        }
    }
}
void OLED_ShowCNChar(u8 x, u8 y, u8 chr, u8 size, u8 num, u8 mode)
{
    u8 temp, t, t1;
    u8 y0 = y;
    u8 csize = (size / 8 + ((size % 8) ? 1 : 0)) * size;
    chr = chr - '0';
    for (t = 0; t < csize; t++)
    {
        if (size == 12)
        {
            if (num == 1)
                temp = A1_1212[chr][t];
            else if (num == 2)
                temp = A2_1212[chr][t];
            else if (num == 3)
                temp = A3_1212[chr][t];
            // else if (num == 4)
            //     temp = A4_1212[chr][t];
        }
        else if (size == 16)
        {
            if (num == 1)
                temp = A1_1616[chr][t];
        }
        else if (size == 24)
        {
            if (num == 1)
                temp = A1_2424[chr][t];
            // else if (num == 2)
            //     temp = A2_2424[chr][t];
        }
        for (t1 = 0; t1 < 8; t1++)
        {
            if (temp & 0x80)
                OLED_DrawPoint(x, y, mode);
            else
                OLED_DrawPoint(x, y, !mode);
            temp <<= 1;
            y++;
            if ((y - y0) == size)
            {
                y = y0;
                x++;
                break;
            }
        }
    }
}
/**
 *\*\name   OLED_ShowString.
 *\*\fun    Display a string.
 *\*\param   x: The X-axis ,for 128x64 oled ,it ranges 0-127
             y: The Y-axis ,for 128x64 oled ,it ranges 0-63
           *p : the string needed to display, for esample 'x123','1asaasd' etc..
          size: the front size, support 12,16,20.
 *\*\return none.
**/
void OLED_ShowString(u8 x, u8 y, const u8 *p, u8 size)
{
    while ((*p <= '~') && (*p >= ' ')) //?????????!
    {
        if (x > (128 - (size / 2)))
        {
            x = 0;
            y += size;
        }
        if (y > (64 - size))
        {
            y = x = 0;
            OLED_Clear();
        }
        OLED_ShowChar(x, y, *p, size, 1);
        x += size / 2;
        p++;
    }
}
void OLED_ShowCNString(u8 x, u8 y, const u8 *p, u8 size, u8 num)
{
    while ((*p <= '~') && (*p >= ' ')) //?????????!
    {
        if (x > (128 - size))
        {
            x = 0;
            y += size;
        }
        if (y > (64 - size))
        {
            y = x = 0;
            OLED_Clear();
        }
        OLED_ShowCNChar(x, y, *p, size, num, 1);
        x += size;
        p++;
    }
}
// m^n����
u32 OLED_pow(u8 m, u8 n)
{
    u32 result = 1;
    while (n--)
        result *= m;
    return result;
}
//��ʾ2������
// x,y :�������
// len :���ֵ�λ��
// size:�����С
// mode:ģʽ	0,���ģʽ;1,����ģʽ
// num:��ֵ(0~4294967295);
void OLED_ShowUnNum(u8 x, u8 y, u32 num, u8 len, u8 size)
{
    u8 t, temp;
    u8 enshow = 0;
    for (t = 0; t < len; t++)
    {
        temp = (num / OLED_pow(10, len - t - 1)) % 10;
        if (enshow == 0 && t < (len - 1))
        {
            if (temp == 0)
            {
                // OLED_ShowChar(x+(size/2)*t,y,' ',size,1);
                continue;
            }
            else
                enshow = 1;
        }
        OLED_ShowChar(x + (size / 2) * t, y, temp + '0', size, 1);
    }
}
/*
�������ܣ���ʾ�޷���num����
������xy���꣬num��ʾ���֣�len�����ֳ��ȣ�size���ִ�Сģʽ
����ֵ����
*/
void OLED_ShowNum(u8 x, u8 y, int num, u8 len, u8 size)
{
    if (num >= 0)
    {
        // OLED_ShowChar(x,y,' ',size,1);
        OLED_ShowUnNum(x + (size / 2), y, num, len, size);
        return;
    }

    OLED_ShowChar(x, y, '-', size, 1);
    OLED_ShowUnNum(x + (size / 2), y, -num, len, size);
}
/*
//�������ܣ���ʾ�޷���float����
//������xy���꣬num��ʾ���֣�precise��ʾ���ֵľ��ȣ�size���ִ�Сģʽ
//����ֵ����
*/
void OLED_ShowUnFloat(u8 x, u8 y, double num, u8 precisenum, u8 precisefloat, u8 size)
{
    u8 i = 0;

    u32 integer;
    double decimal;

    integer = (int)num;                //��������
    decimal = (double)(num - integer); //С������

    OLED_ShowNum(x, y, integer, precisenum, size);                     //��ʾ��������
    OLED_ShowChar(x + (size / 2) * (precisenum + 1), y, '.', size, 1); //��ʾС����

    x = x + (size / 2) * (precisenum + 2);
    while (precisefloat) //��ʾ��λС��
    {
        decimal = decimal * 10;
        integer = (int)decimal; //ȡ��һλС��
        decimal = (double)(decimal - integer);
        OLED_ShowChar(x + (size / 2) * i, y, integer + '0', size, 1); //ѭ����ʾÿλ�����ַ����Ӹ�λ��ʾ
        i++;
        precisefloat--;
    }
}

/*��ʾfloat������,������*/
void OLED_ShowFloat(u8 x, u8 y, double num, u8 precisefloat, u8 size)
{
    u32 integer = 0;

    if (num < 0)
    {
        OLED_ShowChar(x, y, '-', size, 1);
        num = -num;
        integer = (int)(-num); //��������
        if (integer / 100)
            OLED_ShowUnFloat(x + (size / 2), y, num, 3, precisefloat, size);
        else if (integer / 10)
            OLED_ShowUnFloat(x + (size / 2), y, num, 2, precisefloat, size);
        else
            OLED_ShowUnFloat(x + (size / 2), y, num, 1, precisefloat, size);
        return;
    }
    integer = (int)num; //��������
    if (integer / 100)
        OLED_ShowUnFloat(x + (size / 2), y, num, 3, precisefloat, size);
    else if (integer / 10)
        OLED_ShowUnFloat(x + (size / 2), y, num, 2, precisefloat, size);
    else
        OLED_ShowUnFloat(x + (size / 2), y, num, 1, precisefloat, size);
}

void draw_line(unsigned char x1, unsigned char y1,
               unsigned char x2, unsigned char y2,
               unsigned char mode)
{
    unsigned char length, xTmp, yTmp, i, y, yAlt;
    int m;

    if (x1 == x2)
    { // vertical line
        // x1|y1 must be the upper point
        if (y1 > y2)
        {
            xTmp = x1;
            yTmp = y1;
            x1 = x2;
            y1 = y2;
            x2 = xTmp;
            y2 = yTmp;
        }

        length = y2 - y1;
        for (i = 0; i <= length; i++)
        {
            OLED_DrawPoint(x1, y1 + i, mode);
        }
    }
    else if (y1 == y2)
    { // horizontal line
        // x1|y1 must be the left point
        if (x1 > x2)
        {
            xTmp = x1;
            yTmp = y1;
            x1 = x2;
            y1 = y2;
            x2 = xTmp;
            y2 = yTmp;
        }

        length = x2 - x1;
        for (i = 0; i <= length; i++)
        {
            OLED_DrawPoint(x1 + i, y1, mode);
        }
    }
    else
    {
        // x1 must be smaller than x2
        if (x1 > x2)
        {
            xTmp = x1;
            yTmp = y1;
            x1 = x2;
            y1 = y2;
            x2 = xTmp;
            y2 = yTmp;
        }

        if ((y2 - y1) >= (x2 - x1) || (y1 - y2) >= (x2 - x1))
        { // angle larger or equal 45?			length = x2-x1;								// not really the length :)
            m = ((y2 - y1) * 200) / length;
            yAlt = y1;
            for (i = 0; i <= length; i++)
            {
                y = ((m * i) / 200) + y1;

                if ((m * i) % 200 >= 100)
                    y++;
                else if ((m * i) % 200 <= -100)
                    y--;

                draw_line(x1 + i, yAlt, x1 + i, y, mode); /* wuff wuff recurs. */
                if (length <= (y2 - y1) && y1 < y2)
                    yAlt = y + 1;
                else if (length <= (y1 - y2) && y1 > y2)
                    yAlt = y - 1;
                else
                    yAlt = y;
            }
        }
        else
        { // angle smaller 45?			// y1 must be smaller than y2
            if (y1 > y2)
            {
                xTmp = x1;
                yTmp = y1;
                x1 = x2;
                y1 = y2;
                x2 = xTmp;
                y2 = yTmp;
            }
            length = y2 - y1;
            m = ((x2 - x1) * 200) / length;
            yAlt = x1;
            for (i = 0; i <= length; i++)
            {
                y = ((m * i) / 200) + x1;

                if ((m * i) % 200 >= 100)
                    y++;
                else if ((m * i) % 200 <= -100)
                    y--;

                draw_line(yAlt, y1 + i, y, y1 + i, mode);
                if (length <= (x2 - x1) && x1 < x2)
                    yAlt = y + 1;
                else if (length <= (x1 - x2) && x1 > x2)
                    yAlt = y - 1;
                else
                    yAlt = y;
            }
        }
    }
}
void OLED_DrawLine(unsigned char x1, unsigned char y1,
                   unsigned char x2, unsigned char y2,
                   unsigned char mode)
{
    unsigned char length, xTmp, yTmp, i, y, yAlt;
    int m;

    if (x1 == x2)
    { // vertical line
        // x1|y1 must be the upper point
        if (y1 > y2)
        {
            xTmp = x1;
            yTmp = y1;
            x1 = x2;
            y1 = y2;
            x2 = xTmp;
            y2 = yTmp;
        }

        length = y2 - y1;
        for (i = 0; i <= length; i++)
        {
            OLED_DrawPoint(x1, y1 + i, mode);
        }
    }
    else if (y1 == y2)
    { // horizontal line
        // x1|y1 must be the left point
        if (x1 > x2)
        {
            xTmp = x1;
            yTmp = y1;
            x1 = x2;
            y1 = y2;
            x2 = xTmp;
            y2 = yTmp;
        }

        length = x2 - x1;
        for (i = 0; i <= length; i++)
        {
            OLED_DrawPoint(x1 + i, y1, mode);
        }
    }
    else
    {
        // x1 must be smaller than x2
        if (x1 > x2)
        {
            xTmp = x1;
            yTmp = y1;
            x1 = x2;
            y1 = y2;
            x2 = xTmp;
            y2 = yTmp;
        }
        length = x2 - x1;

        if ((y2 - y1) >= (x2 - x1) || (y1 - y2) >= (x2 - x1))
        { // angle larger or equal 45?			length = x2-x1;								// not really the length :)
            m = ((y2 - y1) * 200) / length;
            yAlt = y1;
            for (i = 0; i <= length; i++)
            {
                y = ((m * i) / 200) + y1;

                if ((m * i) % 200 >= 100)
                    y++;
                else if ((m * i) % 200 <= -100)
                    y--;

                draw_line(x1 + i, yAlt, x1 + i, y, mode); /* wuff wuff recurs. */
                if (length <= (y2 - y1) && y1 < y2)
                    yAlt = y + 1;
                else if (length <= (y1 - y2) && y1 > y2)
                    yAlt = y - 1;
                else
                    yAlt = y;
            }
        }
        else
        { // angle smaller 45?			// y1 must be smaller than y2
            if (y1 > y2)
            {
                xTmp = x1;
                yTmp = y1;
                x1 = x2;
                y1 = y2;
                x2 = xTmp;
                y2 = yTmp;
            }
            length = y2 - y1;
            m = ((x2 - x1) * 200) / length;
            yAlt = x1;
            for (i = 0; i <= length; i++)
            {
                y = ((m * i) / 200) + x1;

                if ((m * i) % 200 >= 100)
                    y++;
                else if ((m * i) % 200 <= -100)
                    y--;

                draw_line(yAlt, y1 + i, y, y1 + i, mode);
                if (length <= (x2 - x1) && x1 < x2)
                    yAlt = y + 1;
                else if (length <= (x1 - x2) && x1 > x2)
                    yAlt = y - 1;
                else
                    yAlt = y;
            }
        }
    }
}
