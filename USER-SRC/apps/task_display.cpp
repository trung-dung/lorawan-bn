#include "task_display.h"
#include "lcd.h"
#include "common.h"
#include "rtc-board.h"

// debug
#include "debug_hal.h"

//lcd Object
lcd_t lcd2004;
extern DataMsgParser dataMsgParser;

void vtaskDislay(void *params)
{
	RTC_DateTypeDef sDate; RTC_TimeTypeDef sTime;
	char buffer[20];
	// PIN mapping and init 
	SimpleLCD lcd(&lcd2004);
	// Init LCD
	lcd.lcdInit();
	lcd.lcdClear();
	lcd.lcdGoToXY(0,6);
	lcd.lcdWriteText((char *) "SANSLAB"); 
	lcd.lcdGoToXY(1,3);
	lcd.lcdWriteText((char *) "LSN59 LoRaWAN");	
	while(1)
	{	
		RtcGetCalendarValue(&sDate, &sTime);
		sprintf(buffer, "     TIME: %02d:%02d     ", (sTime.Hours + 7) % 24, sTime.Minutes);
		lcd.lcdGoToXY(2,0);
		lcd.lcdWriteText((char *) buffer);
		sprintf(buffer, "  T:%0.2f H:%0.2f", dataMsgParser.gammar, dataMsgParser.neutron);
		lcd.lcdGoToXY(3,0);
		lcd.lcdWriteText((char *) buffer);			
		vTaskDelay(30000);
	}
}


