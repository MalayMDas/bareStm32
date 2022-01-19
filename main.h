#ifndef _MAIN_H_
#define _MAIN_H_

#define RCC_AHB1ENR         ((uint32_t*)0x40023830U)
#define RCC_AHB1RSTR        ((uint32_t*)0x40023810U)
#define RCC_APB1ENR         ((uint32_t*)0x40023840U)
#define RCC_APB1RSTR        ((uint32_t*)0x40023820U)
#define GPIOA_BASE          ((uint32_t*)0x40020000U)
#define GPIOA_ODR           ((uint32_t*)0x40020014U)
#define GPIOA_AFRL          ((uint32_t*)0x40020020U)
#define SCB_SHCSR           ((uint32_t*)0xE000ED24U)
#define SCB_SHPR3           ((uint32_t*)0xE000ED20U)
#define SCB_ICSR            ((uint32_t*)0xE000ED04U)
#define STK_SRVR            ((uint32_t*)0xE000E014U)
#define STK_SCSR            ((uint32_t*)0xE000E010U) 

#define USART6              ((uint32_t*)0x40011400U) 
#define USART1              ((uint32_t*)0x40011000U) 
#define UART5               ((uint32_t*)0x40005000U) 
#define UART4               ((uint32_t*)0x40004C00U) 
#define USART3              ((uint32_t*)0x40004800U) 

#define USART2              ((uint32_t*)0x40004400U) 
#define USART2_DR           ((uint32_t*)0x40004404U) 
#define USART2_BRR          ((uint32_t*)0x40004408U) 
#define USART2_CR1          ((uint32_t*)0x4000440CU) 
#define USART2_CR2          ((uint32_t*)0x40004410U) 
#define USART2_CR3          ((uint32_t*)0x40004414U) 
#define USART2_GTPR         ((uint32_t*)0x40004418U) 


/* stack details */

#define TASK_STACK_SIZE     4096U
#define SCHED_STACK_SIZE    1024U

#define SRAM_START          0x20000000U
#define SRAM_SIZE           112*1024
#define SRAM_END            (SRAM_START+SRAM_SIZE)


#define MAX_TASKS           10

#define TICK_HZ             1000U
#define HSI_CLOCK           16000000U
#define SYSTICK_TIM_CLK     HSI_CLOCK

#define TASK_INVALID_STATE      0
#define TASK_READY_STATE        1
#define TASK_BLOCKED_STATE      2

typedef struct{
    uint32_t osTCB_PSPValue;
    uint32_t osTCB_BlockCount;
    uint32_t osTCB_CurrentState;
    void (*osTCB_TaskHandler)(void);
}osTaskControlBlock_t;

void osDelay(uint32_t tick_count);
uint32_t osCreateThread(void (*osTCB_TaskHandler)(void));
void osDeleteThread();

#endif