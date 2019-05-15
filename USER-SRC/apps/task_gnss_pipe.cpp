/* mbed Microcontroller Library
 * Copyright (c) 2017 u-blox
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 *
 *	modified and remained: Kien Hoang ( Sanslab )
 *	date: 1/05/2019
 *
 *	processing RMC => lat, long, date data
 *	processing GNA & GNS => timestring + altitude
 *
 */
 
#include "task_gnss.h"
#include "debug_hal.h"
#include "rtc_driver.h"
#include "common.h"
#include "stdlib.h"

#define CHECK_TALKER(s) ((buffer[3] == s[0]) && (buffer[4] == s[1]) && (buffer[5] == s[2]))

// private define
GPSSerial gnss;
extern 	GPSData gGpsData;

void vtaskGnss(void const * argument)
{
const TickType_t delay20s = pdMS_TO_TICKS( 20000UL );
const TickType_t delay1s  = pdMS_TO_TICKS( 1000UL );
	TickType_t blockTime = delay1s;
	int gnssReturnCode = 0;
	int length = 0;
	char buffer[256];
    DEBUG_D("[GNSS] Starting up GNSS...\n");
	if(gnss.init())
	{
		DEBUG_D ("[GNSS] Waiting for GNSS to receive something...\n");
	}
	else
	{
		DEBUG_D ("[GNSS] Unable to initialise GNSS.\n");
	}
	while (1) 
	{
		gnssReturnCode = gnss.getMessage(buffer, sizeof(buffer));
		if (gnssReturnCode > 0) {
			length = LENGTH(gnssReturnCode);
			DEBUG_D("NMEA: %.*s\n", length - 2, buffer);
			if ((PROTOCOL(gnssReturnCode) == GPSSerial::NMEA) && (length > 6)) {
				// Talker is $GA=Galileo $GB=Beidou $GL=Glonass $GN=Combined $GP=GNSS
				if ((buffer[0] == '$') || buffer[1] == 'G') {
					if ( CHECK_TALKER("RMC")) // get lat, lon, altitude, date, time
					{
						const char * dateString = NULL;
						const char * timeString = NULL;
						unsigned long date_nmea = 0;
						unsigned long time_nmea = 0;
						double lat = 0, lon = 0;
						char status;
						if(gnss.getNmeaAngle(3, buffer, length, lat) && 
							gnss.getNmeaAngle(5, buffer, length, lon) &&
							gnss.getNmeaItem(2, buffer, length, status) && status == 'A')
						{
							dateString = gnss.findNmeaItemPos(9, buffer, buffer + length );
							timeString = gnss.findNmeaItemPos(1, buffer, buffer + length );
							// time
							if (timeString != NULL) {
								DEBUG_D("\nGNSS: time is %.6s.\n\n", timeString);
								time_nmea = atof(timeString);
								gGpsData.gpsHh = time_nmea/10000;  // GMT
								//gGpsData.gpsHh = (time_nmea/10000 + 7) % 24;  // UTC+7
								time_nmea = time_nmea % 10000;
								gGpsData.gpsMm = time_nmea/100; 	
								gGpsData.gpsSs = time_nmea % 100; 
								//DEBUG_D("\nGNSS: time %d: %d: %d, alt %d\n", gGpsData.gpsHh, gGpsData.gpsMm, gGpsData.gpsSs, gGpsData.gpsAltide);
							}							
							// date
							if(dateString != NULL)
							{
								DEBUG_D("\nGNSS: date is %.6s.\n\n", dateString);
								DEBUG_D("\nGNSS: location is %.5f %.5f.\n\n", lat, lon);
								date_nmea = atof(dateString);
								gGpsData.gpsDay = date_nmea/10000;
								date_nmea = date_nmea % 10000;
								gGpsData.gpsMonth = date_nmea/100; 	
								gGpsData.gpsYear = (date_nmea % 100) + 2000; 
								gGpsData.gpsLatitude = lat * 100000;
								gGpsData.gpsLongtitude = lon * 100000;
								//DEBUG_D("\nGNSS: %d, %d, %d, %d, %d\n", gGpsData.gpsDay, gGpsData.gpsMonth, gGpsData.gpsYear, gGpsData.gpsLatitude, gGpsData.gpsLongtitude);				
							}	
							// update RTC clock 
							RTC_SetNewTime(gGpsData.gpsHh, gGpsData.gpsMm, gGpsData.gpsSs,
								gGpsData.gpsDay, gGpsData.gpsMonth, gGpsData.gpsYear);
							blockTime = delay20s;							
						}
					}
//						else if(CHECK_TALKER("GLL")) {
//							double latitude = 0, longitude = 0;
//							char ch;
//							if (gnss.getNmeaAngle(1, buffer, length, latitude) && 
//									gnss.getNmeaAngle(3, buffer, length, longitude) && 
//									gnss.getNmeaItem(6, buffer, length, ch) && (ch == 'A')){
//									DEBUG_D("\nGNSS: location is %.5f %.5f.\n\n", latitude, longitude); 
//									DEBUG_D("I am here: https://maps.google.com/?q=%.5f,%.5f\n\n",
//												 latitude, longitude); 
//							}					
//						} 
//					else if (CHECK_TALKER("GGA") || CHECK_TALKER("GNS")) {
//							double altitude = 0; 
//							const char *timeString = NULL;
//							unsigned long time_nmea = 0;	
//							// Altitude
//							if (gnss.getNmeaItem(9, buffer, length, altitude)) {
//								DEBUG_D("\nGNSS: altitude is %.1f m.\n", altitude); 
//								gGpsData.gpsAltide = (uint16_t) (altitude * 10);
//							}													
//							// Time
//							timeString = gnss.findNmeaItemPos(1, buffer, buffer + length);
//							if (timeString != NULL) {
//								DEBUG_D("\nGNSS: time is %.6s.\n\n", timeString);
//								time_nmea = atof(timeString);
//								gGpsData.gpsHh = time_nmea/10000;  // GMT
//								//gGpsData.gpsHh = (time_nmea/10000 + 7) % 24;  // UTC+7
//								time_nmea = time_nmea % 10000;
//								gGpsData.gpsMm = time_nmea/100; 	
//								gGpsData.gpsSs = time_nmea % 100; 
//								// 
//								//DEBUG_D("\nGNSS: time %d: %d: %d, alt %d\n", gGpsData.gpsHh, gGpsData.gpsMm, gGpsData.gpsSs, gGpsData.gpsAltide);
//							}
//					}
//						else if(CHECK_TALKER("RMC")){
//								const char * dateString = NULL;
//								unsigned long date_nmea = 0;
//								uint8_t year = 0;
//								uint8_t month = 0, day = 0;
//								dateString = gnss.findNmeaItemPos(9, buffer, buffer + length );
//								// date
//								if(dateString != NULL)
//								{
//									DEBUG_D("\nGNSS: date is %.6s.\n\n", dateString);
//								}
//								date_nmea = atof(dateString);
//								day = date_nmea/10000;
//								date_nmea = date_nmea % 10000;
//								month = date_nmea/100; 	
//								year = date_nmea % 100; 
//								DEBUG_D("\n Date UTC+7: %d - %d - %d\n\n", day, month, year);		
//						} 
//						else if (CHECK_TALKER("VTG")){
//								double speed = 0; 

//								// Speed
//								if (gnss.getNmeaItem(7, buffer, length, speed)) {
//										DEBUG_D("\nGNSS: speed is %.1f km/h.\n\n", speed);
//								}
//						}
//				gGpsDataSet(gGpsData);
				}
			} // end process NMEA
			
			else
			{
				blockTime = delay1s;
			}
		}
		else // not received GNSS message from GPS module
		{
			DEBUG_D("\nGNSS: Not received GNSS message from GPS module\n");
			blockTime = delay1s;
		}
		vTaskDelay(blockTime);
	}
}
 
// End Of File
