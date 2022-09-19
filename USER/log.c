/**
 *\*\file log.c
 *\*\author Sakurashiling
 *\*\HardWareSite https://oshwhub.com/lin_xiandi/desk-aqm
 *\*\License Apache 2.0
 *\*\version v1.0
 **/

#include "log.h"
#include "n32g430.h"
#include "n32g430_it.h"

USART_InitType USART_InitStructure;

void log_init(void)
{
    GPIO_InitType GPIO_InitStructure;
    USART_InitType USART_InitStructure;
    NVIC_InitType NVIC_InitStructure;
    /* Enable GPIO clock */
    RCC_AHB_Peripheral_Clock_Enable(LOG_PERIPH_GPIO);
    RCC_APB2_Peripheral_Clock_Enable(RCC_APB2_PERIPH_AFIO | LOG_PERIPH);
    /* Enable the USART Interrupt */
    NVIC_InitStructure.NVIC_IRQChannel = USARTx_IRQn;
    NVIC_InitStructure.NVIC_IRQChannelSubPriority = 6;//0,8/30 6:09
    NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
    NVIC_Initializes(&NVIC_InitStructure);
    /* Configure the GPIO ports */
    GPIO_Structure_Initialize(&GPIO_InitStructure);
    GPIO_InitStructure.Pin = LOG_TX_PIN;
    GPIO_InitStructure.GPIO_Mode = GPIO_MODE_AF_PP;
    GPIO_InitStructure.GPIO_Alternate = GPIO_AF5_USART1;
    GPIO_Peripheral_Initialize(LOG_GPIO, &GPIO_InitStructure);
    GPIO_InitStructure.Pin = LOG_RX_PIN;
    GPIO_InitStructure.GPIO_Alternate = GPIO_AF5_USART1;
    GPIO_Peripheral_Initialize(LOG_GPIO, &GPIO_InitStructure);
    /* Configure the USART1 */
    USART_InitStructure.BaudRate = 9600;
    USART_InitStructure.WordLength = USART_WL_8B;
    USART_InitStructure.StopBits = USART_STPB_1;
    USART_InitStructure.Parity = USART_PE_NO;
    USART_InitStructure.HardwareFlowControl = USART_HFCTRL_NONE;
    USART_InitStructure.Mode = USART_MODE_TX | USART_MODE_RX;

    /* init uart */
    USART_Initializes(LOG_USARTx, &USART_InitStructure);

    /* enable uart */
    USART_Interrput_Enable(LOG_USARTx, USART_INT_RXDNE);
    USART_Enable(LOG_USARTx);
}

static int is_lr_sent = 0;
int fputc(int ch, FILE *f)
{
    if (ch == '\r')
    {
        is_lr_sent = 1;
    }
    else if (ch == '\n')
    {
        if (!is_lr_sent)
        {
            USART_Data_Send(LOG_USARTx, (uint8_t)'\r');
            /* Loop until the end of transmission */
            while (USART_Flag_Status_Get(LOG_USARTx, USART_FLAG_TXC) == RESET)
            {
            }
        }
        is_lr_sent = 0;
    }
    else
    {
        is_lr_sent = 0;
    }
    USART_Data_Send(LOG_USARTx, (uint8_t)ch);
    /* Loop until the end of transmission */
    while (USART_Flag_Status_Get(LOG_USARTx, USART_FLAG_TXDE) == RESET)
    {
    }
    return ch;
}
