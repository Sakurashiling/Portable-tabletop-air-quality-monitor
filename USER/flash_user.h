/**
 *\*\file flash_user.h
 *\*\author Sakurashiling
 *\*\HardWareSite https://oshwhub.com/lin_xiandi/desk-aqm
 *\*\License Apache 2.0
 *\*\version v2.3
 **/
#ifndef __FLASH_USER_H__
#define __FLASH_USER_H__

#ifdef __cplusplus
extern "C" {
#endif

#include "n32g430.h"
#include "n32g430_flash.h"

#define FLASH_ADDR_Pages31  ((uint32_t)0X0800F800)

FLASH_STS FLASH_Program_User(uint32_t addr, // 起始地址
                             uint8_t *src,  // 写入的数据指针
                             uint32_t len);  // 数据字节长度

#ifdef __cplusplus
}
#endif

#endif /* __FLASH_USER_H__ */
