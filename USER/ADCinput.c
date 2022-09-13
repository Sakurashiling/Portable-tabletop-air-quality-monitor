/**
 *\*\file ADCinput.c
 *\*\author Sakurashiling
 *\*\HardWareSite https://oshwhub.com/lin_xiandi/desk-aqm
 *\*\License Apache 2.0
 *\*\version v1.0
 **/
#include "ADCinput.h"
#include "bsp_delay.h"
#include "FreeRTOS.h"
#include "string.h"

char str_voltage[8];
uint16_t PCT_voltage = 0;
const float volFactor = 8.471446586200685F * VOLTAGE_FACTOR;

#define ARRAY_DIM(a) (sizeof(a) / sizeof((a)[0]))
const static int Battery_Level_Percent_Table[11] = {3000, 3650, 3700, 3740, 3760, 3795, 3840, 3910, 3980, 4070, 4150};
int toPercentage(int voltage)
{
    int i;
    if (voltage < Battery_Level_Percent_Table[0])
        return 0;
    for (i = 0; i < ARRAY_DIM(Battery_Level_Percent_Table); i++)
    {
        if (voltage < Battery_Level_Percent_Table[i])
            return i * 10 - (10UL * (int)(Battery_Level_Percent_Table[i] - voltage)) /
                                (int)(Battery_Level_Percent_Table[i] - Battery_Level_Percent_Table[i - 1]);
        ;
    }
    return 100;
}

void BT_ADC_Init(void)
{
    GPIO_InitType GPIO_InitStructure;
    ADC_InitType ADC_InitStructure;
    // 启用外设时钟
    RCC_AHB_Peripheral_Clock_Enable(RCC_AHB_PERIPH_GPIOA | RCC_AHB_PERIPH_ADC);
    ADC_Clock_Mode_Config(ADC_CKMOD_AHB, RCC_ADCHCLK_DIV16);
    RCC_ADC_1M_Clock_Config(RCC_ADC1MCLK_SRC_HSI, RCC_ADC1MCLK_DIV8); // selsect HSE as RCC ADC1M CLK Source
    // 将ADC相关引脚设置工作模式设置为模拟模式
    GPIO_Structure_Initialize(&GPIO_InitStructure);
    GPIO_InitStructure.Pin = GPIO_PIN_1;
    GPIO_InitStructure.GPIO_Mode = GPIO_MODE_ANALOG;
    GPIO_Peripheral_Initialize(GPIOA, &GPIO_InitStructure);
    // 配置ADC参数
    ADC_InitStructure.MultiChEn = DISABLE;
    ADC_InitStructure.ContinueConvEn = DISABLE;
    ADC_InitStructure.ExtTrigSelect = ADC_EXT_TRIGCONV_REGULAR_SWSTRRCH;
    ADC_InitStructure.DatAlign = ADC_DAT_ALIGN_R;
    ADC_InitStructure.ChsNumber = ADC_REGULAR_LEN_1;
    ADC_Initializes(&ADC_InitStructure);
    // 启动ADC
    ADC_ON();
    // 等待ADC就绪
    while (ADC_Flag_Status_Get(ADC_RD_FLAG, ADC_FLAG_AWDG, ADC_FLAG_RDY) == RESET)
    {
        __NOP();
    }
    // 启动ADC自校准
    ADC_Calibration_Operation(ADC_CALIBRATION_ENABLE);
    // 等待ADC自校准完成
    while (ADC_Calibration_Operation(ADC_CALIBRATION_STS) == SET)
    {
        __NOP();
    }
}

uint16_t BSP_ADC_GetData(uint8_t ADC_Channel)
{
    // 配置采样参数
    ADC_Channel_Sample_Time_Config(ADC_Channel, ADC_SAMP_TIME_55CYCLES5);
    ADC_Regular_Sequence_Conversion_Number_Config(ADC_Channel, ADC_REGULAR_NUMBER_1);

    // 启动ADC转换
    ADC_Regular_Channels_Software_Conversion_Operation(ADC_EXTRTRIG_SWSTRRCH_ENABLE);
    // 等待ADC转换完成
    while (ADC_Flag_Status_Get(ADC_RUN_FLAG, ADC_FLAG_ENDC, ADC_FLAG_RDY) == 0)
    {
        __NOP();
    }
    ADC_Flag_Status_Clear(ADC_FLAG_ENDC);
    ADC_Flag_Status_Clear(ADC_FLAG_STR);

    // 获取ADC采样值
    return ADC_Regular_Group_Conversion_Data_Get();
}

void BT_ADC_Read(void)
{
    // 定义电压采样值
    uint32_t volRaw = 0;
    // 定义电压值，单位mV
    uint16_t voltage = 0;
    // 采样电压和电流的ADC值，16倍过采样
    int i;
    for (i = 0; i < 16; i++)
    {
        volRaw += BSP_ADC_GetData(VOLTAGE_ADC_CHANNEL);
        delay_ms(20);
    }
    volRaw >>= 4;
    voltage = volRaw * volFactor;
    sprintf(str_voltage, "%u.%02uV", voltage / 1000, (voltage % 1000) / 10);
    PCT_voltage = toPercentage(voltage);
    // PCT_voltage = voltage;
}
