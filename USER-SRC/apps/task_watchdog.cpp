#include "common.h"
#include "iwdg_driver.h"
#include "task_watchdog.h"

void vtaskIWDG(void *params)
{
	HAL_IWDG_Init(&hiwdg);
	while(1)
	{
		vTaskDelay(10000);	
		HAL_IWDG_Refresh(&hiwdg);
	}
}
