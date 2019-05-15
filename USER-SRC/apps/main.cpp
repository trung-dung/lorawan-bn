#include "main.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */
#include "common.h"
#include "debug_hal.h"
#include "board.h"
#include "main_loadconfig.h"

// include task
#include "task_watchdog.h"
#include "task_display.h"
#include "task_lora.h"
#include "task_sensorSerial.h"
#include "task_systemMonitor.h"
#include "task_gnss.h"
//#include "task_cellular.h"
/* USER CODE END Includes */

/* Private function prototypes -----------------------------------------------*/
//extern void vtaskCelluar(void *params);
//extern void vtaskCommandSv(void *params);
//extern void vtaskSms(void *params);
extern void vtaskLoRa(void *params);
extern void vtaskIWDG(void *params);
extern void vtaskGnss(void *params);
extern void vtaskMonitor(void *params);
extern void vtaskDislay(void *params);
extern void vtaskDataSerial(void *params);

/**
  * @brief  The application entry point.
  * @retval int
  */
int main(void)
{
	// Init Mcu and Peripherals
	BoardInitMcu();
	BoardInitPeriph();

	Start_Init_ExtFlash();

	//---------------System Infomations------------------
	DEBUG_D("\r\n------------------------------------------\r\n");
	DEBUG_D("BKRAD-BN Project - Radioactive Detection System\r\n");
	DEBUG_D("Version   :   " __VERSION__ "\r\n");
	DEBUG_D("Compiled  :   " __DATE__ " " __TIME__ "\r\n");
	DEBUG_D("Platform  :   STM32L476RGT6\r\n");
	DEBUG_D("CPU Freq  :   %d MHz\r\n", (unsigned int) SystemCoreClock/1000000);
	DEBUG_D("FreeRTOS  :   %s\r\n", tskKERNEL_VERSION_NUMBER);
	DEBUG_D("Developer :   Kien Hoang ( Sanslab )\r\n");
	DEBUG_D("------------------------------------------\n\n");
	
	HAL_Make_First_System_Parameter();
	
	Check_Flash_Config_Saved();
		
 	if(xTaskCreate(vtaskLoRa, "LoRa", 2000, NULL, tskIDLE_PRIORITY + 1, NULL)!= pdPASS)
	{
		DEBUG_D("\r\nERROR: Creat task LoRaWAN false!\r\n");
		fflush(stdout);
	}
	
 	if(xTaskCreate(vtaskGnss, "GNSS", 768, NULL, tskIDLE_PRIORITY + 1, NULL)!= pdPASS)
	{
		DEBUG_D("\r\nERROR: Creat task CELLULAR false!\r\n");
		fflush(stdout);
	}
	
 	if(xTaskCreate(vtaskDataSerial, "DataSerial", 768, NULL, tskIDLE_PRIORITY + 1, NULL)!= pdPASS)
	{
		DEBUG_D("\r\nERROR: Creat task CELLULAR false!\r\n");
		fflush(stdout);
	}	
	
// 	if(xTaskCreate(vtaskMonitor, "Monitor", 768, NULL, tskIDLE_PRIORITY + 1, NULL)!= pdPASS)
//	{
//		DEBUG_D("\r\nERROR: Creat task CELLULAR false!\r\n");
//		fflush(stdout);
//	}

 	if(xTaskCreate(vtaskDislay, "Display", 768, NULL, tskIDLE_PRIORITY + 1, NULL)!= pdPASS)
	{
		DEBUG_D("\r\nERROR: Creat task CELLULAR false!\r\n");
		fflush(stdout);
	}	

 	if(xTaskCreate(vtaskIWDG, "IWDG", 128, NULL, tskIDLE_PRIORITY + 1, NULL)!= pdPASS)
	{
		DEBUG_D("\r\nERROR: Creat task CELLULAR false!\r\n");
		fflush(stdout);
	}
	
	vTaskStartScheduler();
	
  while (1)
  {
    /* USER CODE END WHILE */

    /* USER CODE BEGIN 3 */
  }
  /* USER CODE END 3 */
}


/**
  * @brief  This function is executed in case of error occurrence.
  * @retval None
  */
void Error_Handler(void)
{
  /* USER CODE BEGIN Error_Handler_Debug */
  /* User can add his own implementation to report the HAL error return state */

  /* USER CODE END Error_Handler_Debug */
}

#ifdef  USE_FULL_ASSERT
/**
  * @brief  Reports the name of the source file and the source line number
  *         where the assert_param error has occurred.
  * @param  file: pointer to the source file name
  * @param  line: assert_param error line source number
  * @retval None
  */
void assert_failed(char *file, uint32_t line)
{ 
  /* USER CODE BEGIN 6 */
  /* User can add his own implementation to report the file name and line number,
     tex: printf("Wrong parameters value: file %s on line %d\r\n", file, line) */
  /* USER CODE END 6 */
}
#endif /* USE_FULL_ASSERT */

/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
