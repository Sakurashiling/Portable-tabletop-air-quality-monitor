/**
 *\*\file flash_user.c
 *\*\author Sakurashiling
 *\*\HardWareSite https://oshwhub.com/lin_xiandi/desk-aqm
 *\*\License Apache 2.0
 *\*\version v2.3
 **/
#include "flash_user.h"
#include <stdio.h>
#include "string.h"

// #define FLASH_WRITE_START_ADDR (FLASH_ADDR_Pages31)
#define FLASH_WRITE_END_ADDR ((uint32_t)0X0800FFFF)
#define FLASH_ERROR_WRP FLASH_ERR_WRP

//=======================================================================
/** 		用户flash写入函数
    说明：	任意起始地址, 没有对齐限制
            任意数据长度, 支持flash跨页
*/

#define FLASH_PAGE_SIZE 2048 // flash一页空间为2048字节
static FLASH_STS FLASH_ProgramPage(uint32_t *SrcAddr, uint32_t DstAddr, uint16_t Len)
{
    uint32_t i;

    for (i = 0; i < Len; i += 4)
    {
        FLASH_Word_Program(DstAddr + i, *(uint32_t *)((uint32_t)SrcAddr + i));
#if (0) // 是否开启擦除校验
        if (*(uint32_t *)(DstAddr + i) != *(uint32_t *)((uint32_t)SrcAddr + i))
        {
            return FLASH_ERROR_PG;
        }
#endif
    }
    return FLASH_EOP;
}

FLASH_STS FLASH_Program_User(uint32_t addr, // 起始地址
                             uint8_t *src,  // 写入的数据指针
                             uint32_t len)  // 数据字节长度
{
    uint8_t FLASH_PAGE_BUF[FLASH_PAGE_SIZE];
    uint32_t i, flag = 0;
    uint32_t pageAddr, pageCount;
    uint32_t tmpAddr, tmpLen;
    uint32_t startAddr, endAddr;

    FLASH_Unlock();
    startAddr = addr % FLASH_PAGE_SIZE;
    endAddr = (addr + len) % FLASH_PAGE_SIZE;

    if (startAddr == 0)
    {
        pageAddr = addr;
        pageCount = len / FLASH_PAGE_SIZE;
        for (i = 0; i < pageCount; i++)
        {
            if (FLASH_EOP != FLASH_One_Page_Erase(pageAddr) != 0)
            {
                FLASH_Lock();
                return FLASH_ERROR_WRP;
            }

            if (FLASH_EOP != FLASH_ProgramPage((uint32_t *)(src + (i * FLASH_PAGE_SIZE)), pageAddr, FLASH_PAGE_SIZE))
            {
                FLASH_Lock();
                return FLASH_ERROR_WRP;
            }

            pageAddr += FLASH_PAGE_SIZE;
        }

        if (endAddr != 0)
        {
            for (i = 0; i < FLASH_PAGE_SIZE; i++)
            {
                FLASH_PAGE_BUF[i] = ((uint8_t *)pageAddr)[i];
            }

            tmpAddr = len % FLASH_PAGE_SIZE;
            for (i = 0; i < tmpAddr; i++)
            {
                FLASH_PAGE_BUF[i] = ((uint8_t *)(src + (pageCount * FLASH_PAGE_SIZE)))[i];
            }

            if (FLASH_EOP != FLASH_One_Page_Erase(pageAddr))
            {
                FLASH_Lock();
                return FLASH_ERROR_WRP;
            }

            if (FLASH_EOP != FLASH_ProgramPage((uint32_t *)FLASH_PAGE_BUF, pageAddr, FLASH_PAGE_SIZE))
            {
                FLASH_Lock();
                return FLASH_ERROR_WRP;
            }
        }
    }
    else
    {
        pageAddr = (addr / FLASH_PAGE_SIZE) * FLASH_PAGE_SIZE;
        tmpAddr = addr % FLASH_PAGE_SIZE;
        tmpLen = FLASH_PAGE_SIZE - tmpAddr;

        if (tmpLen > len)
        {
            tmpLen = len;
            flag = 0;
        }
        else
        {
            flag = 1;
        }

        for (i = 0; i < FLASH_PAGE_SIZE; i++)
        {
            FLASH_PAGE_BUF[i] = ((uint8_t *)pageAddr)[i];
        }

        for (i = 0; i < tmpLen; i++)
        {
            FLASH_PAGE_BUF[tmpAddr + i] = ((uint8_t *)src)[i];
        }

        if (FLASH_EOP != FLASH_One_Page_Erase(pageAddr))
        {
            FLASH_Lock();
            return FLASH_ERROR_WRP;
        }

        if (FLASH_EOP != FLASH_ProgramPage((uint32_t *)FLASH_PAGE_BUF, pageAddr, FLASH_PAGE_SIZE))
        {
            FLASH_Lock();
            return FLASH_ERROR_WRP;
        }

        pageCount = (len - tmpLen) / FLASH_PAGE_SIZE;
        pageAddr += FLASH_PAGE_SIZE;
        for (i = 0; i < pageCount; i++)
        {
            if (FLASH_EOP != FLASH_One_Page_Erase(pageAddr))
            {
                FLASH_Lock();
                return FLASH_ERROR_WRP;
            }

            if (FLASH_EOP != FLASH_ProgramPage((uint32_t *)(src + tmpLen + (i * FLASH_PAGE_SIZE)), pageAddr, FLASH_PAGE_SIZE))
            {
                FLASH_Lock();
                return FLASH_ERROR_WRP;
            }

            pageAddr += FLASH_PAGE_SIZE;
        }

        if ((endAddr != 0) && (flag == 1))
        {
            for (i = 0; i < FLASH_PAGE_SIZE; i++)
            {
                FLASH_PAGE_BUF[i] = ((uint8_t *)pageAddr)[i];
            }

            tmpAddr = (len - tmpLen) % FLASH_PAGE_SIZE;

            for (i = 0; i < tmpAddr; i++)
            {
                FLASH_PAGE_BUF[i] = ((uint8_t *)(src + tmpLen + (pageCount * FLASH_PAGE_SIZE)))[i];
            }

            if (FLASH_EOP != FLASH_One_Page_Erase(pageAddr))
            {
                FLASH_Lock();
                return FLASH_ERROR_WRP;
            }

            if (FLASH_EOP != FLASH_ProgramPage((uint32_t *)FLASH_PAGE_BUF, pageAddr, FLASH_PAGE_SIZE))
            {
                FLASH_Lock();
                return FLASH_ERROR_WRP;
            }
        }
    }

    FLASH_Lock();
    return FLASH_EOP;
}

// 示例程序
// int test(void)
// {
//     uint8_t user_buf[128];
//     uint32_t addr;

/*-------- 字对齐写操作 --------*/
// addr = FLASH_ADDR_Pages31 + 4;

// memset(user_buf, 0xa1, 128);
// user_buf[0] = 0x01;
// FLASH_Program_User(addr, user_buf, 4); // 0x01, 0xa1, 0xa1, 0xa1
// printf("addr = %04x\r\n", addr);
// printf("dat = %04x\r\n", *(uint32_t *)addr);

/*-------- 非字齐写操作 --------*/
// addr += 9;

// memset(user_buf, 0xa2, 128);
// user_buf[0] = 0x02;
// FLASH_Program_User(addr, user_buf, 4); // 0x02, 0xa2, 0xa2, 0xa2
// printf("addr = %04x\r\n", addr);
// printf("dat = %04x\r\n", *(uint32_t *)(addr));

/*-------- 跨页写操作 --------*/
// addr = FLASH_ADDR_Pages31 - 2;

// memset(user_buf, 0xa3, 128);
// user_buf[0] = 0x03;
// FLASH_Program_User(addr, user_buf, 4); // 0x03, 0xa3, 0xa3, 0xa3
// printf("addr = %04x\r\n", addr);
// printf("dat = %04x\r\n", *(uint32_t *)(addr));
// }
