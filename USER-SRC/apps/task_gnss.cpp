#include "task_gnss.h"
#include "debug_hal.h"
#include "stdlib.h"
#include "fifo.h"
#include "board.h"
#include "board-config.h"
#include "uart_driver.h"
#include "gpio.h"

#define MAX_SIZE 256
extern Fifo_t fifoGpsRx;
 
static uint8_t initGnss(void)
{	
	Gpio_t gpsRst;
	GpioInit( &gpsRst, GPS_RST_N, PIN_OUTPUT, PIN_PUSH_PULL, PIN_NO_PULL, 0 );
	HW_UART5_Init();
	GpioWrite( &gpsRst, 0);
	HAL_Delay(100);
	GpioWrite( &gpsRst, 1 );
	return 0;
}	

void vtaskGnss(void *params) 
{
const TickType_t delay10s = pdMS_TO_TICKS( 10000UL );
const TickType_t delay1s  = pdMS_TO_TICKS( 1000UL );
	TickType_t blockTime = delay1s;
	char c; char checksum;
	int crc = 0;
	int cnt = 0, index = 0, res = 0;
	char NMEAString[256];
	memset(NMEAString, 0, 256);
	// init uart5 - gps
	initGnss();
	while(1)
	{
		checksum = 0, crc = 0;
		index = 0; cnt = 0;
		// pop char until '$'
		do
		{
			c = FifoPop(&fifoGpsRx);
		}while(c != '$');
		
		NMEAString[index] = '$';
		do
		{
			c = FifoPop(&fifoGpsRx);
			NMEAString[++index] = c;				 // '\r'
			if(c == '*') break;
			checksum ^= c;
		}while(c != '\r' && cnt++ < (MAX_SIZE - 1));
		// check crc
		c = FifoPop(&fifoGpsRx);
		if( c != toHex[ (checksum >> 4) & 0xF] ) crc = 1;  else NMEAString[++index] = c;
		c = FifoPop(&fifoGpsRx);
		if( (c != toHex[ (checksum >> 0) & 0xF])) crc = 1;  else NMEAString[++index] = c;
		if( crc == 0) // valid crc -> parser msg
		{
			NMEAString[++index] = '\r';
			NMEAString[++index] = '\n';
			NMEAString[++index] = '\0';
			c = '\0';
			  
			  // debug
			DEBUG_D("\nGNSS: %s\r\n", NMEAString);
			res = parseGPSStr((unsigned char *) NMEAString); // parser msg
//			DEBUG_D("res: %d\r\n", res);
			memset(NMEAString, 0, MAX_SIZE); // clear buffer
			if(res == 0) blockTime = delay10s;
			else blockTime = delay1s;
		}	
		else 
		{
			memset(NMEAString, 0, MAX_SIZE);			
			blockTime = delay1s;
		}
		vTaskDelay(blockTime);
	}
}
 
// End Of File
