#include "n32g430.h"
#include "n32g430_i2c.h"
#include "main.h"
#include "i2c_drv.h"
#include "log.h"

/** I2C_Master **/

#define I2C_MASTER_LOW_LEVEL
#define TEST_BUFFER_SIZE  100
#define I2CT_FLAG_TIMEOUT ((uint32_t)0x1000)
#define I2CT_LONG_TIMEOUT ((uint32_t)(10 * I2CT_FLAG_TIMEOUT))
#define I2C_MASTER_ADDR   0x30
#define I2C_SLAVE_ADDR    0x78

#ifdef NON_REENTRANT /* Avoid function reentrant */
static uint32_t Mutex_Flag = 0;
#endif

#define I2C1_TEST
#define I2C1_REMAP
#define I2Cx I2C1
#define I2Cx_SCL_PIN GPIO_PIN_4
#define I2Cx_SDA_PIN GPIO_PIN_5
#define GPIOx        GPIOA
#define GPIO_AF_I2C  GPIO_AF8_I2C1


volatile Status test_status      = FAILED;
static __IO uint32_t I2CTimeout;
static CommCtrl_t Comm_Flag = C_READY;  /* Avoid star flag and stop flag set 1 at the same time */
static uint8_t RCC_RESET_Flag = 0;
void CommTimeOut_CallBack(ErrCode_t errcode);

/**
*\*\name    Delay.
*\*\fun     system ms delay function.
*\*\param   nCount
*\*\return  none 
**/
void Delay(uint32_t nCount)
{
    uint32_t tcnt;
    while (nCount--)
    {
        tcnt = 48000 / 5;
        while (tcnt--){;}
    }
}

/**
*\*\name    Delay_us.
*\*\fun     system us delay function.
*\*\param   nCount
*\*\return  none 
**/
void Delay_us(uint32_t nCount)
{
    uint32_t tcnt;
    while (nCount--)
    {
        tcnt = 48 / 5;
        while (tcnt--){;}
    }
}

/**
*\*\name    i2c_master_init.
*\*\fun     master gpio/rcc/i2c initializes.
*\*\param   none
*\*\return  result 
**/
int i2c_master_init(void)
{
    I2C_InitType i2c1_master;
    GPIO_InitType i2c1_gpio;
    RCC_APB1_Peripheral_Clock_Enable(RCC_APB1_PERIPH_I2C1);
    RCC_AHB_Peripheral_Clock_Enable(RCC_AHB_PERIPH_GPIOA);

    GPIO_Structure_Initialize(&i2c1_gpio);
    /*PB6 -- SCL; PB7 -- SDA*/
    i2c1_gpio.Pin            = I2Cx_SCL_PIN | I2Cx_SDA_PIN;
    i2c1_gpio.GPIO_Slew_Rate = GPIO_SLEW_RATE_FAST; 
    i2c1_gpio.GPIO_Mode      = GPIO_MODE_AF_OD;
    i2c1_gpio.GPIO_Alternate = GPIO_AF_I2C;
    i2c1_gpio.GPIO_Pull      = GPIO_PULL_UP; 
    GPIO_Peripheral_Initialize(GPIOx, &i2c1_gpio);

    I2C_Reset(I2Cx);
    I2C_Initializes_Structure(&i2c1_master);
    i2c1_master.BusMode     = I2C_BUSMODE_I2C;
    i2c1_master.DutyCycle   = I2C_SMDUTYCYCLE_1; 
    i2c1_master.OwnAddr1    = I2C_MASTER_ADDR;
    i2c1_master.AckEnable   = I2C_ACKDIS;			//为加快速度，将ack去掉
    i2c1_master.AddrMode    = I2C_ADDR_MODE_7BIT;
    i2c1_master.ClkSpeed    = 400000; /* 400k */

    I2C_Initializes(I2Cx, &i2c1_master);

    I2C_ON(I2Cx);
    return 0;
}

/**
*\*\name    i2c_master_send.
*\*\fun     master send data.
*\*\param   data-data to send
*\*\param   len-length of data to send
*\*\return  send result 
**/
int i2c_master_send(uint8_t* data, int len)
{
    uint8_t* sendBufferPtr = data;
    
#ifdef NON_REENTRANT
    if (Mutex_Flag)
        return -1;
    else
        Mutex_Flag = 1; /* Enter function,Mutex_Flag = 1 */
#endif
    
    I2CTimeout             = I2CT_LONG_TIMEOUT;
    while (I2C_Flag_Status_Get(I2C1, I2C_FLAG_BUSY))
    {
        if ((I2CTimeout--) == 0)
        {
            CommTimeOut_CallBack(MASTER_BUSY);
        }
    }
    
    if (Comm_Flag == C_READY)
    {
        Comm_Flag = C_START_BIT;
        I2C_Generate_Start_Enable(I2C1);
    }
    
    I2CTimeout = I2CT_LONG_TIMEOUT;
    while (!I2C_Event_Check(I2C1, I2C_EVT_MASTER_MODE_FLAG)) /* EV5 */
    {
        if ((I2CTimeout--) == 0)
        {
            CommTimeOut_CallBack(MASTER_MODE);
        }
    }
    
    I2C_7bit_Addr_Send(I2C1, I2C_SLAVE_ADDR, I2C_DIRECTION_SEND);   
    I2CTimeout = I2CT_LONG_TIMEOUT;
    while (!I2C_Event_Check(I2C1, I2C_EVT_MASTER_TXMODE_FLAG)) /* EV6 */
    {
        if ((I2CTimeout--) == 0)
        {
            CommTimeOut_CallBack(MASTER_TXMODE);
        }
    }
    Comm_Flag = C_READY;
    
    /* send data */
    while (len-- > 0)
    {
        I2C_Data_Send(I2C1, *sendBufferPtr++);
        I2CTimeout = I2CT_LONG_TIMEOUT;
        while (!I2C_Event_Check(I2C1, I2C_EVT_MASTER_DATA_SENDING)) /* EV8 */
        {
            if ((I2CTimeout--) == 0)
            {
                CommTimeOut_CallBack(MASTER_SENDING);
            }
        }
    }

    I2CTimeout = I2CT_LONG_TIMEOUT;
    while (!I2C_Event_Check(I2C1, I2C_EVT_MASTER_DATA_SENDED)) /* EV8-2 */
    {
        if ((I2CTimeout--) == 0)
        {
            CommTimeOut_CallBack(MASTER_SENDED);
        }
    }
    
    if (Comm_Flag == C_READY)
    {
        Comm_Flag = C_STOP_BIT;
        I2C_Generate_Stop_Enable(I2C1);
    }
    
    while (I2C_Flag_Status_Get(I2C1, I2C_FLAG_BUSY))
    {
        if ((I2CTimeout--) == 0)
        {
            CommTimeOut_CallBack(MASTER_BUSY);
        }
    }
    Comm_Flag = C_READY;

#ifdef NON_REENTRANT
    if (Mutex_Flag)
        Mutex_Flag = 0; /* Exit function,Mutex_Flag = 0 */
    else
        return -2;
#endif
    
    return 0;
}

/**
*\*\name    i2c_master_recv.
*\*\fun     master receive data.
*\*\param   data-data to receive
*\*\param   len-length of data to receive
*\*\return  receive result 
**/
int i2c_master_recv(uint8_t* data, int len)
{
    uint8_t* recvBufferPtr = data;

#ifdef NON_REENTRANT
    if (Mutex_Flag)
        return -1;
    else
        Mutex_Flag = 1; /* Enter function,Mutex_Flag = 1 */
#endif
    
    I2CTimeout             = I2CT_LONG_TIMEOUT;
    while (I2C_Flag_Status_Get(I2C1, I2C_FLAG_BUSY))
    {
        if ((I2CTimeout--) == 0)
        {
            CommTimeOut_CallBack(MASTER_BUSY);
        }
    }
    I2C_Acknowledg_Enable(I2C1);

    /* send start */
    if (Comm_Flag == C_READY)
    {
        Comm_Flag = C_START_BIT;
        I2C_Generate_Start_Enable(I2C1);
    }
    
    I2CTimeout = I2CT_LONG_TIMEOUT;
    while (!I2C_Event_Check(I2C1, I2C_EVT_MASTER_MODE_FLAG)) /* EV5 */
    {
        if ((I2CTimeout--) == 0)
        {
            CommTimeOut_CallBack(MASTER_MODE);
        }
    }
    /* send addr */
    I2C_7bit_Addr_Send(I2C1, I2C_SLAVE_ADDR, I2C_DIRECTION_RECV);
    I2CTimeout = I2CT_LONG_TIMEOUT;
    while (!I2C_Event_Check(I2C1, I2C_EVT_MASTER_RXMODE_FLAG)) /* EV6 */
    {
        if ((I2CTimeout--) == 0)
        {
            CommTimeOut_CallBack(MASTER_RXMODE);
        }
    }
    Comm_Flag = C_READY;
    
    if (len == 1)
    {
        I2C_Acknowledg_Disable(I2C1);
        (void)(I2C1->STS1); /*/ clear ADDR */
        (void)(I2C1->STS2);
        if (Comm_Flag == C_READY)
        {
            Comm_Flag = C_STOP_BIT;
            I2C_Generate_Stop_Enable(I2C1);
        }
        
        I2CTimeout = I2CT_LONG_TIMEOUT;
        while (!I2C_Flag_Status_Get(I2C1, I2C_FLAG_RXDATNE))
        {
            if ((I2CTimeout--) == 0)
            {
                CommTimeOut_CallBack(MASTER_RECVD);
            }
        }
        *recvBufferPtr++ = I2C_Data_Recv(I2C1);
        len--;
    }
    else if (len == 2)
    {
        I2C1->CTRL1 |= 0x0800; /*/ set ACKPOS */
        (void)(I2C1->STS1);
        (void)(I2C1->STS2);
        I2C_Acknowledg_Disable(I2C1);
        
        I2CTimeout = I2CT_LONG_TIMEOUT;
        while (!I2C_Flag_Status_Get(I2C1, I2C_FLAG_BYTEF))
        {
            if ((I2CTimeout--) == 0)
            {
                CommTimeOut_CallBack(MASTER_BYTEF);
            }
        }
        
        if (Comm_Flag == C_READY)
        {
            Comm_Flag = C_STOP_BIT;
            I2C_Generate_Stop_Enable(I2C1);
        }
        
        *recvBufferPtr++ = I2C_Data_Recv(I2C1);
        len--;
        *recvBufferPtr++ = I2C_Data_Recv(I2C1);
        len--;
    }
    else
    {
        I2C_Acknowledg_Enable(I2C1);
        (void)(I2C1->STS1);
        (void)(I2C1->STS2);
        
        while (len)
        {
            if (len == 3)
            {
                I2CTimeout = I2CT_LONG_TIMEOUT;
                while (!I2C_Flag_Status_Get(I2C1, I2C_FLAG_BYTEF))
                {
                    if ((I2CTimeout--) == 0)
                    {
                        CommTimeOut_CallBack(MASTER_BYTEF);
                    }
                }
                I2C_Acknowledg_Disable(I2C1);
                *recvBufferPtr++ = I2C_Data_Recv(I2C1);
                len--;
                
                I2CTimeout = I2CT_LONG_TIMEOUT;
                while (!I2C_Flag_Status_Get(I2C1, I2C_FLAG_BYTEF))
                {
                    if ((I2CTimeout--) == 0)
                    {
                        CommTimeOut_CallBack(MASTER_BYTEF);
                    }
                }
                
                if (Comm_Flag == C_READY)
                {
                    Comm_Flag = C_STOP_BIT;
                    I2C_Generate_Stop_Enable(I2C1);
                }
        
                *recvBufferPtr++ = I2C_Data_Recv(I2C1);
                len--;
                *recvBufferPtr++ = I2C_Data_Recv(I2C1);
                len--;
                
                break;
            }
            
            I2CTimeout = I2CT_LONG_TIMEOUT;
            while (!I2C_Event_Check(I2C1, I2C_EVT_MASTER_DATA_RECVD_FLAG)) /* EV7 */
            {
                if ((I2CTimeout--) == 0)
                {
                    CommTimeOut_CallBack(MASTER_RECVD);
                }
            }
            *recvBufferPtr++ = I2C_Data_Recv(I2C1);
            len--;
        }
    }
    
    I2CTimeout = I2CT_LONG_TIMEOUT;
    while (I2C_Flag_Status_Get(I2C1, I2C_FLAG_BUSY))
    {
        if ((I2CTimeout--) == 0)
        {
            CommTimeOut_CallBack(MASTER_BUSY);
        }
    }
    Comm_Flag = C_READY;
    
#ifdef NON_REENTRANT
    if (Mutex_Flag)
        Mutex_Flag = 0; /* Exit function,Mutex_Flag = 0 */
    else
        return -2;
#endif
    
    return 0;
}


/**
*\*\name    IIC_RestoreSlaveByClock.
*\*\fun     Emulate 9 clock recovery slave by GPIO.
*\*\param   none
*\*\return  none 
**/
void IIC_RestoreSlaveByClock(void)
{
    uint8_t i;
    GPIO_InitType i2cx_gpio;
    
    RCC_AHB_Peripheral_Clock_Enable(RCC_AHB_PERIPH_GPIOB);
    RCC_APB2_Peripheral_Reset(RCC_APB2_PERIPH_AFIO);
    GPIO_Reset(GPIOx);
    
    GPIO_Structure_Initialize(&i2cx_gpio);
    i2cx_gpio.Pin            = I2Cx_SCL_PIN;
    i2cx_gpio.GPIO_Slew_Rate = GPIO_SLEW_RATE_FAST;
    i2cx_gpio.GPIO_Mode      = GPIO_MODE_OUT_PP;
    GPIO_Peripheral_Initialize(GPIOx, &i2cx_gpio);
    
    for (i = 0; i < 9; i++)
    {
        GPIO_Pins_Set(GPIOx, I2Cx_SCL_PIN);
        Delay_us(5);
        GPIO_PBC_Pins_Reset(GPIOx, I2Cx_SCL_PIN);
        Delay_us(5);
    }   
}
 
/**
*\*\name    SystemNVICReset.
*\*\fun     System software reset.
*\*\param   none
*\*\return  none 
**/
void SystemNVICReset(void)
{
    
    __disable_irq();
    log_info("***** NVIC system reset! *****\r\n");
    NVIC_SystemReset();
}

/**
*\*\name    IIC_RCCReset.
*\*\fun     RCC clock reset.
*\*\param   none
*\*\return  none 
**/
void IIC_RCCReset(void)
{
    if (RCC_RESET_Flag >= 3)
    {
        SystemNVICReset();
    }
    else
    {
        RCC_RESET_Flag++;
        
        RCC_APB1_Peripheral_Reset(RCC_APB1_PERIPH_I2C1);
        
        RCC_APB1_Peripheral_Clock_Disable(RCC_APB1_PERIPH_I2C1);
        GPIOB->PMODE &= 0xFFFF0FFF; /*input */
        RCC_APB2_Peripheral_Clock_Disable(RCC_APB2_PERIPH_AFIO);
        RCC_AHB_Peripheral_Clock_Disable(RCC_AHB_PERIPH_GPIOB);
        
        RCC_APB1_Peripheral_Reset(RCC_APB1_PERIPH_I2C1);
        
        IIC_RestoreSlaveByClock();
        
        log_info("***** IIC module by RCC reset! *****\r\n");
        i2c_master_init();
    }
}

/**
*\*\name    IIC_SWReset.
*\*\fun     I2c software reset.
*\*\param   none
*\*\return  none 
**/
void IIC_SWReset(void)
{
    GPIO_InitType i2cx_gpio;
    
    GPIO_Structure_Initialize(&i2cx_gpio);
    i2cx_gpio.Pin            = I2Cx_SCL_PIN | I2Cx_SDA_PIN;
    i2cx_gpio.GPIO_Slew_Rate = GPIO_SLEW_RATE_FAST;
    i2cx_gpio.GPIO_Mode      = GPIO_MODE_INPUT;
    GPIO_Peripheral_Initialize(GPIOx, &i2cx_gpio);
    
    I2CTimeout = I2CT_LONG_TIMEOUT;
    for (;;)
    {
        if ((I2Cx_SCL_PIN | I2Cx_SDA_PIN) == (GPIOx->PID & (I2Cx_SCL_PIN | I2Cx_SDA_PIN)))
        {
            I2Cx->CTRL1 |= 0x8000;
            __NOP();
            __NOP();
            __NOP();
            __NOP();
            __NOP();
            I2Cx->CTRL1 &= ~0x8000;
            
            log_info("***** IIC module self reset! *****\r\n");
            break;
        }
        else
        {
            if ((I2CTimeout--) == 0)
            {
                IIC_RCCReset();
            }
        }
    }
}

/**
*\*\name    CommTimeOut_CallBack.
*\*\fun     Callback function.
*\*\param   none
*\*\return  none 
**/
void CommTimeOut_CallBack(ErrCode_t errcode)
{
    log_info("...ErrCode:%d\r\n", errcode);
    
#if (COMM_RECOVER_MODE == MODULE_SELF_RESET)
    IIC_SWReset();
#elif (COMM_RECOVER_MODE == MODULE_RCC_RESET)
    IIC_RCCReset();
#elif (COMM_RECOVER_MODE == SYSTEM_NVIC_RESET)
    SystemNVICReset();
#endif
}

