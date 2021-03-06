#ifndef STM32F1XX_IT_H
#define STM32F1XX_IT_H

#include "stm32f1xx.h"

void NMI_Handler(void);
void HardFault_Handler(void);
void MemManage_Handler(void);
void BusFault_Handler(void);
void UsageFault_Handler(void);
void SVC_Handler(void);
void DebugMon_Handler(void);
void PendSV_Handler(void);
void SysTick_Handler(void);
void RTC_IRQHandler(void);
void TIM2_IRQHandler(void);

#endif // STM32F1XX_IT_H
