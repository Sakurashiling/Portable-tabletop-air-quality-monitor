/**
 *\*\file log.h
 *\*\author Sakurashiling
 *\*\HardWareSite https://oshwhub.com/lin_xiandi/desk-aqm
 *\*\License Apache 2.0
 *\*\version v1.0
 **/

#ifndef __LOG_H__
#define __LOG_H__

#include <stdio.h>

#define LOG_USARTx USART1
#define USARTx_IRQn       USART1_IRQn
#define USARTx_IRQHandler USART1_IRQHandler
#define LOG_PERIPH RCC_APB2_PERIPH_USART1
#define LOG_GPIO GPIOA
#define LOG_PERIPH_GPIO RCC_AHB_PERIPH_GPIOA
#define LOG_TX_PIN GPIO_PIN_9
#define LOG_RX_PIN GPIO_PIN_10

#define LOG_NONE    0
#define LOG_ERROR   10
#define LOG_WARNING 20
#define LOG_INFO    30
#define LOG_DEBUG   40

#ifndef LOG_LEVEL
#define LOG_LEVEL LOG_DEBUG
#endif

#if LOG_LEVEL >= LOG_INFO
#define log_info(...) printf(__VA_ARGS__)
#else
#define log_info(...)
#endif

#if LOG_LEVEL >= LOG_ERROR
#define log_error(...) printf(__VA_ARGS__)
#else
#define log_error(...)
#endif

#if LOG_LEVEL >= LOG_WARNING
#define log_warning(...) printf(__VA_ARGS__)
#else
#define log_warning(...)
#endif

#if LOG_LEVEL >= LOG_DEBUG
#define log_debug(...) printf(__VA_ARGS__)
#else
#define log_debug(...)
#endif

void log_init(void);

#define log_func() log_debug("call %s\r\n", __FUNCTION__)

#endif /* __LOG_H__ */
