#include "handlers.h"

#include <stm32f1xx_ll_rtc.h>
#include <stm32f1xx_ll_tim.h>

#include "display/gdfb.h"
#include "input.h"
#include "rtc.h"
#include "swtimers.h"

void _Error_Handler(char *file, int line)
{
    while (1) {
    }
}

void NMI_Handler(void)
{
}

void HardFault_Handler(void)
{
    while (1) {
    }
}

void MemManage_Handler(void)
{
    while (1) {
    }
}

void BusFault_Handler(void)
{
    while (1) {
    }
}

void UsageFault_Handler(void)
{
    while (1) {
    }
}

void SVC_Handler(void)
{
}

void DebugMon_Handler(void)
{
}

void PendSV_Handler(void)
{
}

void SysTick_Handler(void)
{
    inputPoll();
    swTimUpdate();
}

void RTC_IRQHandler(void)
{
    if (LL_RTC_IsEnabledIT_SEC(RTC) != 0) {
        /* Clear the RTC Second interrupt */
        LL_RTC_ClearFlag_SEC(RTC);
        rtcReadTime();

        /* Wait until last write operation on RTC registers has finished */
        LL_RTC_WaitForSynchro(RTC);
    }
    /* Clear the EXTI's Flag for RTC Alarm */
    LL_EXTI_ClearFlag_0_31(LL_EXTI_LINE_17);
}

void TIM2_IRQHandler(void)
{
    if (LL_TIM_IsActiveFlag_UPDATE(TIM2)) {
        /* Clear the update interrupt flag*/
        LL_TIM_ClearFlag_UPDATE(TIM2);
    }
    ks0108IRQ();
}