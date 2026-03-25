#include "task.h"
#include "sam.h"

static volatile uint32_t task_ms = 0;
static volatile bool systick_flag = false;

void task_init(void)
{
    // Configure SysTick for 1ms interrupts
    SysTick_Config(SystemCoreClock / 1000);
}

uint32_t task_get_ms(void)
{
    return task_ms;
}

bool task_systick_happened(void)
{
    if (systick_flag)
    {
        systick_flag = false;
        return true;
    }
    return false;
}

// Exception handler
void SysTick_Handler(void)
{
    task_ms++;
    systick_flag = true;
}
