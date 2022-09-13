
#ifndef __I2C_DRV_H__
#define __I2C_DRV_H__

#include <stdio.h>
#include "n32g430.h"

typedef enum
{
    FAILED = 0,
    PASSED = !FAILED
} Status;

typedef enum
{
    C_READY = 0,
    C_START_BIT,
    C_STOP_BIT
}CommCtrl_t;

typedef enum
{
    MASTER_OK = 0,
    MASTER_BUSY,
    MASTER_MODE,
    MASTER_TXMODE,
    MASTER_RXMODE,
    MASTER_SENDING,
    MASTER_SENDED,
    MASTER_RECVD,
    MASTER_BYTEF,
    MASTER_BUSERR,
    MASTER_UNKNOW,
    SLAVE_OK = 20,
    SLAVE_BUSY,
    SLAVE_MODE,
    SLAVE_BUSERR,
    SLAVE_UNKNOW,

}ErrCode_t;

int i2c_master_init(void);
int i2c_master_send(uint8_t* data, int len);
int i2c_master_recv(uint8_t* data, int len);

#endif /* __I2C_H__ */
