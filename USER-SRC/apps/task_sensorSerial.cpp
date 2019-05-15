#include "common.h"
#include "task_sensorSerial.h"
#include "uart_driver.h"
#include "board.h"
#include "fifo.h"

//debug
#include "debug_hal.h"
#define MAX_SIZE 										256
const char toHex[] = { '0','1','2','3','4','5','6','7','8','9','A','B','C','D','E','F' };
extern SystemInformation systemInfo;
extern Cycle_TimeData timeDataCtl;
extern Fifo_t fifoDataRx;

extern DataMsgParser dataMsgParser;
extern RadioActiveData gRadioActiveData;

// parser message received 
static void parserDataMessage( char * dataMsg );
/* Task Create --------------------------------------------------------------*/

void vtaskDataSerial(void *params)
{
	DEBUG_D("\nTask data serial\r\n");
	const TickType_t delay30s = pdMS_TO_TICKS( 30000UL );
	const TickType_t delay1s = pdMS_TO_TICKS( 1000UL );
	TickType_t blockTime = delay1s;
	uint32_t _lastTimeSensorResponse = 0;
	uint32_t _deltaTime = 0;
	
	char c; uint8_t cnt = 0, index = 0;
	char checksum;
	int crc = 0;
	char dataPacket[MAX_SIZE];
	memset(dataPacket, 0, MAX_SIZE);
	
	// Init and Start digital channel
	HW_UART3_Init();
	_lastTimeSensorResponse = systemInfo.SystemUptime;
	
	// read data from buffer fifo and print content to debug console
	while(1)
	{
		checksum = 0, crc = 0;
		index = 0; cnt = 0;
		// pop char until '$'
		do
		{
			c = FifoPop(&fifoDataRx);
		}while(c != '$');
		
		dataPacket[index] = '$';
		do
		{
			c = FifoPop(&fifoDataRx);
			dataPacket[++index] = c;				 // '\r'
			if(c == '*') break;
			checksum ^= c;
		}while(c != '\r' && cnt++ < (MAX_SIZE - 1));
		// check crc
//		c = FifoPop(&fifoDataRx);
//		if( c != toHex[ (checksum >> 4) & 0xF] ) crc = 1;  else dataPacket[++index] = c;
//		c = FifoPop(&fifoDataRx);
//		if( (c != toHex[ (checksum >> 0) & 0xF])) crc = 1;  else dataPacket[++index] = c;
		if( crc == 0) // valid crc -> parser msg
		{
			dataPacket[++index] = '\r';
			dataPacket[++index] = '\n';
			dataPacket[++index] = '\0';
			c = '\0';
			  
			  // debug
			DEBUG_D("\nData: %s\r\n", dataPacket);
			parserDataMessage((char *) dataPacket); // parser msg 
			memset(dataPacket, 0, MAX_SIZE); // clear buffer
//			_lastTimeSensorResponse = systemInfo.SystemUptime;
			blockTime = delay30s;
		}	
		else 
		{
			memset(dataPacket, 0, MAX_SIZE);			
			blockTime = delay1s;
		}
//		_deltaTime = systemInfo.SystemUptime - _lastTimeSensorResponse;
//		systemInfo.Delta_SensorLasttime = _deltaTime;
//		
//		// (-_-) One day... No response from sensor
//		// If expired time -> Set sensor data to zero
//		if(_deltaTime > timeDataCtl.Cycle_SensorTimeOut)
//		{
//			sensorData.Salt = 0;
//			sensorData.Oxy  = 0;
//			sensorData.PH   = 0;
//			sensorData.Temp = 0;
//			sensorData.NH3  = 0;
//			sensorData.H2S  = 0;
//		}				
		vTaskDelay(blockTime);
	}
}

static void parserDataMessage( char * dataMsg )
{
    char *p = dataMsg;
	float time;
	unsigned long time_nmea;
	unsigned char hh, mm, ss, day, mon, year;
	double lat, lon;
	float gammar, neutron;
	//printf(nmea);
	
    p = strchr(p, ',')+1; //skip time	1 (BKRADBN)
	gammar = atof(p);
	p = strchr(p, ',')+1; // neutron    2
	neutron = atof(p); 
	DEBUG_D("Gammar: %0.2f - Neutron: %0.2f\r\n", gammar, neutron);
	p = strchr(p, ',')+ 1; // time      3
	time = atof(p);
	time_nmea =time;
	hh = time_nmea/10000; 
	time_nmea %= 10000;
	mm = time_nmea/100; 	
	ss = time_nmea%100; 
	DEBUG_D("UTC Data time: %.03f %02d:%02d:%02d\r\n",time,hh,mm,ss);

    p = strchr(p, ',')+1; //	 4  date
 	time = atof(p);
	time_nmea = time;
	day = time_nmea/10000; 
	time_nmea %= 10000;
	mon = time_nmea/100; 	
	year = time_nmea%100;	
	DEBUG_D("UTC Data date: %.0f %02d-%02d-%02d\r\n",time,day,mon,year);
    if(year < 90)
        year += 100;
    mon -= 1;
	
    p = strchr(p, ',')+1;  // 5				  
    lat = atof(p);
    p = strchr(p, ',')+1;  // 6					
	lon = atof(p);
	DEBUG_D("Lat: %0.5f - long: %0.5f\r\n", lat, lon);
	dataMsgParser.gammar = gammar;
	dataMsgParser.neutron = neutron;
	gRadioActiveData.gamma = (uint16_t) (dataMsgParser.gammar * 100);
	gRadioActiveData.neutron = (uint16_t) (dataMsgParser.neutron * 100);
	DEBUG_D("Gammar: %d, Neutron: %d\r\n", gRadioActiveData.gamma, gRadioActiveData.neutron);
	
}

