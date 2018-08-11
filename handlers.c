#include "handlers.h"

#include "stm32f1xx_ll_exti.h"
#include <stm32f1xx_ll_rtc.h>
#include <stm32f1xx_ll_tim.h>

#include "input.h"
#include "rtc.h"
#include "spectrum.h"
#include "timers.h"

#if defined (_KS0108B)
#include "display/ks0108.h"
#elif defined (_ILI9320)
#include "display/ili9320.h"
#elif defined (_ILI9341)
#include "display/ili9341.h"
#else
#error "Unsupported display driver"
#endif

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
        // Clear the RTC Second interrupt
        LL_RTC_ClearFlag_SEC(RTC);

        // Callback
        rtcReadTime();

        // Wait until last write operation on RTC registers has finished
        LL_RTC_WaitForSynchro(RTC);
    }

    // Clear the EXTI's Flag for RTC Alarm
    LL_EXTI_ClearFlag_0_31(LL_EXTI_LINE_17);
}

void TIM2_IRQHandler(void)
{
    if (LL_TIM_IsActiveFlag_UPDATE(TIM2)) {
        // Clear the update interrupt flag
        LL_TIM_ClearFlag_UPDATE(TIM2);

        // Callbacks
#if defined(_KS0108B)
        ks0108IRQ();        // Update screen from framebuffer
#elif defined(_ILI9320)
        ili9320BusIRQ();    // Read bus
#endif
        glcdPWM();
        spConvertADC();
    }
}
