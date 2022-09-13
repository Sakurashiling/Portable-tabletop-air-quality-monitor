/**
 *\*\file main.c
 *\*\author Sakurashiling
 *\*\HardWareSite https://oshwhub.com/lin_xiandi/desk-aqm
 *\*\License Apache 2.0
 *\*\version v2.2
 **/
#include "main.h"
#include "n32g430.h"
#include "bsp_delay.h"
#include "pmswitch.h"
#include "keyinput.h"
#include "i2c_drv.h"
#include "oled.h"
#include "ADCinput.h"
#include "anotherinput.h"
#include "log.h"
#include "pm25_usart.h"
#include "myiic.h"
#include "sht20.h"
#include "SGP30.h"

#include "FreeRTOS.h"
#include "task.h"
//用户定义--------------------------
extern char str_voltage[8];
extern uint16_t PCT_voltage;
// extern uint8_t RxBuffer1[32];

uint16_t PM1_0 = 0, PM2_5 = 0, PM10 = 0;
uint16_t Count0_3nm = 0, Count0_5nm = 0, Count1_0nm = 0, Count2_5nm = 0, Count5_0nm = 0, Count10nm = 0;
uint16_t temperature = 0, humidity = 0;
// SGP30传感器状态 0：正常 1：初始化失败 2：读取失败
uint16_t SGP30_STA = 1, eCO2 = 400, TVOC = 0;
// 综合空气质量 0：优 1：良 2：轻度污染 3：中度污染 4：重度污染 5：严重污染
uint16_t Air_quality = 0;

extern bool usart_flag = false;
extern bool PMSWITCH_State = true;
extern bool ESPIN_flag;
extern bool Charging_State;

bool IIC_IN_TASK = true; // IIC使用中
bool LButton = false, RButton = false;
bool LButtonLong = false, RButtonLong = false;
bool LBT_PASS = false, RBT_PASS = false;
// OLED当前页-----------------------
// 0级页面 0: 主页页面 1: 设置图标列表 2:设置内容
int16_t OLEDPAGE0 = 0, PAGE0Count = 2;
// 1级页面(主页页面) 0:汇总 1:空气温湿度 2:空气TVOC&eCO2浓度 3:PM浓度 4:PM悬浮颗粒物计数(0.3-1.0nm) 5:PM悬浮颗粒物计数(2.5-10nm)
int16_t OLEDPAGE1 = 0, PAGE1Count = 5;
// 2级页面(设置图标列表) 0: 设置0 1: 设置1
int16_t OLEDPAGE2 = 0, PAGE2Count = 1;
// 3级页面(设置内容) 0: 设置0 1: 设置1
int16_t OLEDPAGE3 = 0, PAGE3Count = 1;

//任务参数--------------------------
//优先级 堆栈大小 任务句柄 任务函数
#define START_TASK_PRIO 1
#define START_STK_SIZE 128
TaskHandle_t StartTask_Handler;
void start_task(void *pvParameters);
//用户任务--------------------------
/*SGP30读取任务*/
#define SGP30_TASK_PRIO 2
#define SGP30_STK_SIZE 128
TaskHandle_t SGP30Task_Handler;
void SGP30_task(void *pvParameters);
/*SHT20读取任务*/
#define SHT20_TASK_PRIO 3
#define SHT20_STK_SIZE 128
TaskHandle_t SHT20Task_Handler;
void SHT20_task(void *pvParameters);
/*串口接收处理任务*/
#define UART_RX_TASK_PRIO 4
#define UART_RX_STK_SIZE 128
TaskHandle_t UART_RXTask_Handler;
void UART_RX_task(void *pvParameters);
/*OLED显示*/
#define OLED_TASK_PRIO 5
#define OLED_STK_SIZE 128
TaskHandle_t OLEDTask_Handler;
void OLED_task(void *pvParameters);
/*按键输入*/
#define Keyinput_TASK_PRIO 6
#define Keyinput_STK_SIZE 128
TaskHandle_t KeyinputTask_Handler;
void Keyinput_task(void *pvParameters);
/*杂七杂八的任务*/
#define another_TASK_PRIO 7
#define another_STK_SIZE 128
TaskHandle_t anotherTask_Handler;
void another_task(void *pvParameters);

int main(void)
{
    //设置系统中断优先级分组4(FreeRTOS中的默认方式！)
    NVIC_Priority_Group_Set(NVIC_PER2_SUB2_PRIORITYGROUP);

    delay_init(108);     //初始化延迟函数
    log_init();          //串口初始化
    delay_ms(10);        //
    PMS_Init();          //初始化PMS控制开关
    delay_ms(5);         //
    KeyInput_Init();     //初始化按键输入
    delay_ms(5);         //
    anotherinput_Init(); //初始化ESP、充电检测输入
    delay_ms(5);         //
    i2c_master_init();   //初始化i2c1(OLED)
    delay_ms(10);        //
    oled_init();         //初始化OLED显示
    delay_ms(10);        //
    BT_ADC_Init();       //初始化电池电压ADC
    delay_ms(5);         //
    IIC_Init();          //初始化i2c2(SHT20、SGP30)
    delay_ms(5);         //

    printf("ALL Init OK!\n");

    //创建开始任务
    xTaskCreate((TaskFunction_t)start_task,          //任务函数
                (const char *)"start_task",          //任务名称
                (uint16_t)START_STK_SIZE,            //任务堆栈大小
                (void *)NULL,                        //传递给任务函数的参数
                (UBaseType_t)START_TASK_PRIO,        //任务优先级
                (TaskHandle_t *)&StartTask_Handler); //任务句柄
    //开启任务调度
    vTaskStartScheduler();
}

//开始任务任务函数
void start_task(void *pvParameters)
{
    taskENTER_CRITICAL(); //进入临界区

    // 创建SGP30读取任务
    xTaskCreate((TaskFunction_t)SGP30_task,
                (const char *)"SGP30_task",
                (uint16_t)SGP30_STK_SIZE,
                (void *)NULL,
                (UBaseType_t)SGP30_TASK_PRIO,
                (TaskHandle_t *)&SGP30Task_Handler);
    // 创建SHT20读取任务
    xTaskCreate((TaskFunction_t)SHT20_task,
                (const char *)"SHT20_task",
                (uint16_t)SHT20_STK_SIZE,
                (void *)NULL,
                (UBaseType_t)SHT20_TASK_PRIO,
                (TaskHandle_t *)&SHT20Task_Handler);
    //创建串口接受处理任务
    xTaskCreate((TaskFunction_t)UART_RX_task,
                (const char *)"UART_RX_task",
                (uint16_t)UART_RX_STK_SIZE,
                (void *)NULL,
                (UBaseType_t)UART_RX_TASK_PRIO,
                (TaskHandle_t *)&UART_RXTask_Handler);
    //创建OLED刷新任务
    xTaskCreate((TaskFunction_t)OLED_task,
                (const char *)"OLED_task",
                (uint16_t)OLED_STK_SIZE,
                (void *)NULL,
                (UBaseType_t)OLED_TASK_PRIO,
                (TaskHandle_t *)&OLEDTask_Handler);
    //创建按键检测任务
    xTaskCreate((TaskFunction_t)Keyinput_task,
                (const char *)"Keyinput_task",
                (uint16_t)Keyinput_STK_SIZE,
                (void *)NULL,
                (UBaseType_t)Keyinput_TASK_PRIO,
                (TaskHandle_t *)&KeyinputTask_Handler);
    //创建打杂专用任务
    xTaskCreate((TaskFunction_t)another_task,
                (const char *)"another_task",
                (uint16_t)another_STK_SIZE,
                (void *)NULL,
                (UBaseType_t)another_TASK_PRIO,
                (TaskHandle_t *)&anotherTask_Handler);

    vTaskDelete(StartTask_Handler); //删除开始任务

    taskEXIT_CRITICAL(); //退出临界区
}

// SGP30读取任务函数
void SGP30_task(void *pvParameters)
{
    sgp30_init();
    delay_ms(1000); // 延时1000ms
    IIC_IN_TASK = false;
    do
    {
        if (sgp30_read(&eCO2, &TVOC) == -1) // 读取并判断
        {
            SGP30_STA = 1; // 初始化时出问题：初始化未完成或传感器有问题
        }
        else
        {
            SGP30_STA = 0; // 读取正常
        }
        delay_ms(1000); // 延时1000ms
    } while (SGP30_STA > 0);
    while (1)
    {
        if (sgp30_read(&eCO2, &TVOC) == -1) // 读取并判断
        {
            SGP30_STA = 2; // 初始化过后出问题：读取失败
        }
        else
        {
            SGP30_STA = 0; // 读取正常
        }
        IIC_IN_TASK = false;
        delay_ms(800); // 延时800ms
        IIC_IN_TASK = true;
        delay_ms(200); // 延时200ms
    }
}

// SHT20读取任务函数
void SHT20_task(void *pvParameters)
{
    while (1)
    {
        if (!IIC_IN_TASK)
        {
            SHT20_Get_Data(SHT20_Measurement_T_HM, &temperature, &humidity);
            delay_ms(100); // 延时100ms
            SHT20_Get_Data(SHT20_Measurement_RH_HM, &temperature, &humidity);
        }
        delay_ms(600); // 延时600ms
    }
}

// 串口接收处理任务函数
void UART_RX_task(void *pvParameters)
{
    PM_Sensor_DataStruct *pData = NULL;
    while (1)
    {
        pData = read_PMS7003_data();
        if (pData != NULL)
        {
            PM1_0 = pData->PM1_0;
            PM2_5 = pData->PM2_5;
            PM10 = pData->PM10;
            Count0_3nm = pData->Count0_3nm;
            Count0_5nm = pData->Count0_5nm;
            Count1_0nm = pData->Count1_0nm;
            Count2_5nm = pData->Count2_5nm;
            Count5_0nm = pData->Count5_0nm;
            Count10nm = pData->Count10nm;
            pData = NULL;
        }
        delay_ms(600); // 延时600ms
    }
}
// UI按键控制
void UITASK_BT()
{
    switch (OLEDPAGE0)
    {
    case 0: //主页页面
    {
        //实现按键短按功能
        if (LButton == true)
        {
            LButton = false;
            if (OLEDPAGE1 <= 0)
            {
                OLEDPAGE1 = PAGE1Count;
            }
            else
            {
                OLEDPAGE1--;
            }
        }
        if (RButton == true)
        {
            RButton = false;
            if (OLEDPAGE1 >= PAGE1Count)
            {
                OLEDPAGE1 = 0;
            }
            else
            {
                OLEDPAGE1++;
            }
        }
        //实现按键长按功能
        if (LButtonLong == true) // 长按左按键
        {
            LButtonLong = false;
            // if (OLEDPAGE1 <= 0)
            // {
            //     OLEDPAGE1 = PAGE1Count;
            // }
            // else
            // {
            //     OLEDPAGE1--;
            // }
        }
        if (RButtonLong == true) // 长按右按键进入设置图标列表
        {
            RButtonLong = false;
            if (OLEDPAGE0 != 1)
            {
                OLEDPAGE0 = 1;
            }
        }
    }
    break;
    case 1: //设置图标列表界面
    {
        //实现按键短按功能
        if (LButton == true)
        {
            LButton = false;
            if (OLEDPAGE2 <= 0)
            {
                OLEDPAGE2 = PAGE2Count;
            }
            else
            {
                OLEDPAGE2--;
            }
        }
        if (RButton == true)
        {
            RButton = false;
            if (OLEDPAGE2 >= PAGE2Count)
            {
                OLEDPAGE2 = 0;
            }
            else
            {
                OLEDPAGE2++;
            }
        }
        //实现按键长按功能
        if (LButtonLong == true) // 长按左按键返回主页页面
        {
            LButtonLong = false;
            if (OLEDPAGE0 != 0)
            {
                OLEDPAGE0 = 0;
            }
        }
        if (RButtonLong == true) // 长按右按键进入设置内容
        {
            RButtonLong = false;
            if (OLEDPAGE0 != 2)
            {
                OLEDPAGE0 = 2;
            }
        }
    }
    break;
    case 2: //设置内容
    {
        //实现按键短按功能
        if (LButton == true)
        {
            LButton = false;
            LBT_PASS = true;
        }
        if (RButton == true)
        {
            RButton = false;
            RBT_PASS = true;
        }
        //实现按键长按功能
        if (LButtonLong == true) // 长按左按键返回设置图标列表页面
        {
            LButtonLong = false;
            if (OLEDPAGE0 != 1)
            {
                OLEDPAGE0 = 1;
            }
        }
        if (RButtonLong == true)
        {
            RButtonLong = false;
            // if (OLEDPAGE0 != 2)
            // {
            //     OLEDPAGE0 = 2;
            // }
        }
    }
    break;
    default:
        break;
    }
}
// 电池图标显示
void BAT_ICON()
{
    if (Charging_State == true)
    {
        OLED_ShowCNString(116, 0, (const u8 *)"6", 12, 1); //充电中
    }
    else if (PCT_voltage <= 50) //二分法提高处理效率
    {
        if (PCT_voltage <= 0)
        {
            OLED_ShowCNString(116, 0, (const u8 *)"7", 12, 1); //电池0%
        }
        else if (PCT_voltage <= 10)
        {
            OLED_ShowCNString(116, 0, (const u8 *)"8", 12, 1); //电池10%
        }
        else if (PCT_voltage <= 20)
        {
            OLED_ShowCNString(116, 0, (const u8 *)"9", 12, 1); //电池20%
        }
        else if (PCT_voltage <= 30)
        {
            OLED_ShowCNString(116, 0, (const u8 *)"0", 12, 2); //电池30%
        }
        else if (PCT_voltage <= 40)
        {
            OLED_ShowCNString(116, 0, (const u8 *)"1", 12, 2); //电池40%
        }
        else
        {
            OLED_ShowCNString(116, 0, (const u8 *)"2", 12, 2); //电池50%
        }
    }
    else if (PCT_voltage <= 60)
    {
        OLED_ShowCNString(116, 0, (const u8 *)"3", 12, 2); //电池60%
    }
    else if (PCT_voltage <= 70)
    {
        OLED_ShowCNString(116, 0, (const u8 *)"4", 12, 2); //电池70%
    }
    else if (PCT_voltage <= 80)
    {
        OLED_ShowCNString(116, 0, (const u8 *)"5", 12, 2); //电池80%
    }
    else if (PCT_voltage <= 90)
    {
        OLED_ShowCNString(116, 0, (const u8 *)"6", 12, 2); //电池90%
    }
    else
    {
        OLED_ShowCNString(116, 0, (const u8 *)"7", 12, 2); //电池100%
    }
}
// 空气质量分析,可以自行添加条件
void airquality_analyse(void)
{
    if (PM2_5 <= 50)
    {
        Air_quality = 0;
    }
    else if (PM2_5 <= 100)
    {
        Air_quality = 1;
    }
    else if (PM2_5 <= 150)
    {
        Air_quality = 2;
    }
    else if (PM2_5 <= 200)
    {
        Air_quality = 3;
    }
    else if (PM2_5 <= 300)
    {
        Air_quality = 4;
    }
    else
        Air_quality = 5;
}
// OLED刷新任务函数
void OLED_task(void *pvParameters)
{
    while (1)
    {
        UITASK_BT();  // UI按键控制函数
        OLED_Clear(); // 仅仅更新OLED的数据，不刷新
        OLED_ShowString(0, 0, (const u8 *)" ", 12);
        switch (OLEDPAGE0)
        {
        case 0: // 主页页面
        {
            switch (OLEDPAGE1)
            {
            case 0: // 汇总

                OLED_ShowCNString(0, 0, (const u8 *)"89", 12, 2);    // 空气
                OLED_ShowCNString(24, 0, (const u8 *)"0123", 12, 1); // 悬浮颗粒数
                airquality_analyse();                                // 空气质量分析
                OLED_ShowCNString(0, 16, (const u8 *)"23", 24, 1);   // 空气
                OLED_DrawLine(52, 14, 52, 64, 1);                    // 分隔竖线
                OLED_ShowCNString(0, 40, (const u8 *)"45", 24, 1);   // 质量
                switch (Air_quality)
                {
                case 0:
                    OLED_ShowCNString(78, 22, (const u8 *)"0", 24, 1);    // 优
                    OLED_ShowString(56, 46, (const u8 *)"excellent", 16); // 湿度符号
                    break;
                case 1:
                    OLED_ShowCNString(78, 22, (const u8 *)"1", 24, 1); // 良
                    OLED_ShowString(76, 46, (const u8 *)"good", 16);   // 湿度符号
                    break;
                case 2:
                    OLED_ShowCNString(60, 24, (const u8 *)"3789", 16, 1); // 轻度污染
                    OLED_ShowString(66, 46, (const u8 *)"Level 3", 16);   // 湿度符号
                    break;
                case 3:
                    OLED_ShowCNString(60, 24, (const u8 *)"4789", 16, 1); // 中度污染
                    OLED_ShowString(66, 46, (const u8 *)"Level 4", 16);   // 湿度符号
                    break;
                case 4:
                    OLED_ShowCNString(60, 24, (const u8 *)"5789", 16, 1); // 重度污染
                    OLED_ShowString(66, 46, (const u8 *)"Level 5", 16);   // 湿度符号
                    break;
                case 5:
                    OLED_ShowCNString(60, 24, (const u8 *)"6589", 16, 1); // 严重污染
                    OLED_ShowString(66, 46, (const u8 *)"Level 6", 16);   // 湿度符号
                    break;
                default:
                    break;
                }

                break;
            case 1: // 空气温湿度

                OLED_ShowCNString(0, 0, (const u8 *)"89", 12, 2);   //空气
                OLED_ShowCNString(24, 0, (const u8 *)"781", 12, 3); //温湿度

                OLED_ShowCNString(0, 16, (const u8 *)"68", 24, 1); //温度
                OLED_ShowString(48, 16, (const u8 *)":", 24);
                OLED_ShowNum(36, 16, (int)(temperature / 10), 3, 24);
                OLED_ShowString(86, 16, (const u8 *)".", 24);
                OLED_ShowNum(86, 16, (int)(temperature % 10), 1, 24);
                OLED_ShowCNString(112, 22, (const u8 *)"2", 16, 1); //温度符号
                OLED_ShowCNString(0, 40, (const u8 *)"78", 24, 1);  //湿度
                OLED_ShowString(48, 40, (const u8 *)":", 24);
                OLED_ShowNum(36, 40, (int)(humidity / 10), 3, 24);
                OLED_ShowString(86, 40, (const u8 *)".", 24);
                OLED_ShowNum(86, 40, (int)(humidity % 10), 1, 24);
                OLED_ShowString(112, 46, (const u8 *)"Rh", 16); //湿度符号
                break;
            case 2: // TVOC&eCO2浓度

                OLED_ShowString(0, 0, (const u8 *)"TVOC&eCO2", 12);
                OLED_ShowCNString(56, 0, (const u8 *)"01", 12, 3); //浓度

                if (SGP30_STA == 0)
                {
                    OLED_ShowString(0, 16, (const u8 *)"TVOC:", 24);
                    OLED_ShowNum(52, 22, (int)TVOC, 5, 16);
                    OLED_ShowString(104, 20, (const u8 *)"ppb", 16);
                    OLED_ShowString(0, 40, (const u8 *)"eCO2:", 24);
                    OLED_ShowNum(52, 46, (int)eCO2, 5, 16);
                    OLED_ShowString(104, 44, (const u8 *)"ppm", 16);
                }
                else if (SGP30_STA == 1)
                {
                    OLED_ShowString(0, 16, (const u8 *)"TVOC:", 24);
                    OLED_ShowString(60, 22, (const u8 *)"Initing", 16);
                    OLED_ShowString(0, 40, (const u8 *)"eCO2:", 24);
                    OLED_ShowString(60, 46, (const u8 *)"Initing", 16);
                }
                else if (SGP30_STA == 2)
                {
                    OLED_ShowString(0, 16, (const u8 *)"TVOC:", 24);
                    OLED_ShowString(60, 22, (const u8 *)"Waiting", 16);
                    OLED_ShowString(0, 40, (const u8 *)"eCO2:", 24);
                    OLED_ShowString(60, 46, (const u8 *)"Waiting", 16);
                }

                break;
            case 3: // PM浓度

                OLED_ShowCNString(0, 0, (const u8 *)"89", 12, 2); //空气
                OLED_ShowString(24, 0, (const u8 *)"PM", 12);
                OLED_ShowCNString(38, 0, (const u8 *)"01", 12, 3); //浓度

                OLED_ShowString(0, 16, (const u8 *)"PM1.0:      g/m3", 16);
                OLED_ShowCNString(84, 20, (const u8 *)"5", 12, 1); //μ
                OLED_ShowNum(40, 16, (int)PM1_0, 4, 16);
                OLED_ShowString(0, 32, (const u8 *)"PM2.5:      g/m3", 16);
                OLED_ShowCNString(84, 36, (const u8 *)"5", 12, 1); //μ
                OLED_ShowNum(40, 32, (int)PM2_5, 4, 16);
                OLED_ShowString(0, 48, (const u8 *)"PM10:       g/m3", 16);
                OLED_ShowCNString(84, 52, (const u8 *)"5", 12, 1); //μ
                OLED_ShowNum(40, 48, (int)PM10, 4, 16);
                break;
            case 4: // PM悬浮颗粒物计数(0.3-1.0nm)

                OLED_ShowCNString(0, 0, (const u8 *)"89", 12, 2);     //空气
                OLED_ShowCNString(24, 0, (const u8 *)"23456", 12, 3); //悬浮颗粒数

                OLED_ShowString(0, 16, (const u8 *)"0.3nm:", 16);
                OLED_ShowString(96, 20, (const u8 *)"/0.1L", 12);
                OLED_ShowNum(44, 20, (int)Count0_3nm, 5, 12);
                OLED_ShowCNString(84, 18, (const u8 *)"4", 12, 1); //个
                OLED_ShowString(0, 32, (const u8 *)"0.5nm:", 16);
                OLED_ShowString(96, 36, (const u8 *)"/0.1L", 12);
                OLED_ShowNum(44, 36, (int)Count0_5nm, 5, 12);
                OLED_ShowCNString(84, 34, (const u8 *)"4", 12, 1); //个
                OLED_ShowString(0, 48, (const u8 *)"1.0nm:", 16);
                OLED_ShowString(96, 52, (const u8 *)"/0.1L", 12);
                OLED_ShowNum(44, 52, (int)Count1_0nm, 5, 12);
                OLED_ShowCNString(84, 50, (const u8 *)"4", 12, 1); //个
                break;
            case 5: // PM悬浮颗粒物计数(2.5-10nm)

                OLED_ShowCNString(0, 0, (const u8 *)"89", 12, 2);     //空气
                OLED_ShowCNString(24, 0, (const u8 *)"23456", 12, 3); //悬浮颗粒数

                OLED_ShowString(0, 16, (const u8 *)"2.5nm:", 16);
                OLED_ShowString(96, 20, (const u8 *)"/0.1L", 12);
                OLED_ShowNum(44, 20, (int)Count2_5nm, 5, 12);
                OLED_ShowCNString(84, 18, (const u8 *)"4", 12, 1); //个
                OLED_ShowString(0, 32, (const u8 *)"5.0nm:", 16);
                OLED_ShowString(96, 36, (const u8 *)"/0.1L", 12);
                OLED_ShowNum(44, 36, (int)Count5_0nm, 5, 12);
                OLED_ShowCNString(84, 34, (const u8 *)"4", 12, 1); //个
                OLED_ShowString(0, 48, (const u8 *)"10nm:", 16);
                OLED_ShowString(96, 52, (const u8 *)"/0.1L", 12);
                OLED_ShowNum(48, 52, (int)Count10nm, 5, 12);
                OLED_ShowCNString(84, 50, (const u8 *)"4", 12, 1); //个
                break;
            default:
                break;
            }
            BAT_ICON(); // 显示电池电量
            OLED_ShowNum(82, 0, (int)OLEDPAGE1, 1, 12);
            OLED_ShowString(96, 0, (const u8 *)"/", 12);
            OLED_ShowNum(98, 0, (int)PAGE1Count, 1, 12);
            OLED_DrawLine(112, 0, 112, 14, 1); // 电池竖线
            OLED_DrawLine(0, 14, 128, 14, 1);  // 分界横线
            OLED_DrawLine(84, 0, 84, 14, 1);   // 页数竖线
        }
        break;
        case 1: // 设置图标列表页面
        {
            OLED_ShowCNString(0, 24, (const u8 *)"0", 16, 1);   //左箭头
            OLED_ShowCNString(112, 24, (const u8 *)"1", 16, 1); //右箭头
            switch (OLEDPAGE2)
            {
            case 0: // 设置0
                OLED_ShowString(96, 0, (const u8 *)"0", 12);
                break;
            case 1: // 设置1
                OLED_ShowString(96, 0, (const u8 *)"1", 12);
                break;
            default:
                break;
            }
        }
        case 2: // 设置内容界面
        {
            switch (OLEDPAGE3)
            {
            case 0: //设置0
                OLED_ShowString(0, 1, (const u8 *)str_voltage, 12);
                OLED_ShowNum(28, 1, (int)PCT_voltage, 3, 12);
                OLED_ShowChar(52, 1, (u8)'%', 12, 1);

                // OLED_ShowUnFloat(0, 28, testx, 6, 2, 12);
                if (ESPIN_flag == true)
                {
                    OLED_ShowString(0, 40, (const u8 *)"ESPIN_flag", 12);
                }
                // OLED_ShowString(0, 52, (const u8 *)RxBuffer1, 12);
                break;
            default:
                break;
            }
        }
        break;
        default:
            break;
        }
        OLED_Refresh_Gram(); // 刷屏
        delay_ms(1000);      // 延时1s
    }
}

// 按键检测任务函数
void Keyinput_task(void *pvParameters)
{
    uint16_t Lkey = 0, Rkey = 0;
    while (1)
    {
        if (GPIO_Input_Pin_Data_Get(keyinput_PORT, keyinput_PINL) == 0)
        {
            Lkey++;
        }
        else
        {
            if (Lkey < 2) // 2*50=100ms
            {
                Lkey = 0;
            }
            else if (Lkey >= 2 && Lkey <= 10) // 10*50=500ms
            {
                Lkey = 0;
                LButton = true;
            }
            else if (Lkey > 10) // 10*50=500ms
            {
                Lkey = 0;
                LButtonLong = true;
            }
        }
        if (GPIO_Input_Pin_Data_Get(keyinput_PORT, keyinput_PINR) == 0)
        {
            Rkey++;
        }
        else
        {
            if (Rkey < 2) // 2*50=100ms
            {
                Rkey = 0;
            }
            else if (Rkey >= 2 && Rkey <= 10) // 10*50=500ms
            {
                Rkey = 0;
                RButton = true;
            }
            else if (Rkey > 10) // 10*50=500ms
            {
                Rkey = 0;
                RButtonLong = true;
            }
        }
        delay_ms(50);
    }
}

// PMS传感器开关
void PM_SWITCH(void)
{
    if (PMSWITCH_State == true)
    {
        PMSWITCH_ON;
    }
    else
    {
        PMSWITCH_OFF;
    }
}

// 打杂的任务函数
void another_task(void *pvParameters)
{
    while (1)
    {
        PM_SWITCH();         // PMS传感器开关
        BT_ADC_Read();       //更新电池电压
        anotherinput_Read(); // ESP、充电检测输入
        delay_ms(3000);      // 延时3s
    }
}
