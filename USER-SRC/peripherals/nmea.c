// use this library for GNSS when use GPRS module
#include <inttypes.h>
#include <ctype.h>
#include "nmea.h"
#include "fifo.h"

// DEBUG
#include "debug_hal.h"
#include "common.h"
#include "rtc-board.h"

extern GPSData gGpsData;
const char toHex[] = { '0','1','2','3','4','5','6','7','8','9','A','B','C','D','E','F' };


void nmea_parse_gpgga(char *nmea, gpgga_t *loc)
{
    char *p = nmea;
	float time;
	unsigned long time_nmea;
	unsigned char hh,mm,ss;
	//printf(nmea);
    p = strchr(p, ',')+1; //skip time 120823.012 120823012
	time = atof(p);
	time_nmea =time;
	hh = time_nmea/10000; 
	time_nmea %= 10000;
	mm = time_nmea/100; 	
	ss = time_nmea%100; 
	
	//m_s = time_nmea;
	DEBUG_D("UTC time(GPGGA): %.03f %02d:%02d:%02d\r\n",time,hh,mm,ss);
    p = strchr(p, ',')+1;
    loc->latitude = atof(p);

    p = strchr(p, ',')+1;
    switch (p[0]) {
        case 'N':
            loc->lat = 'N';
            break;
        case 'S':
            loc->lat = 'S';
            break;
        case ',':
            loc->lat = '\0';
            break;
    }

    p = strchr(p, ',')+1;
    loc->longitude = atof(p);

    p = strchr(p, ',')+1;
    switch (p[0]) {
        case 'W':
            loc->lon = 'W';
            break;
        case 'E':
            loc->lon = 'E';
            break;
        case ',':
            loc->lon = '\0';
            break;
    }

    p = strchr(p, ',')+1;
    loc->quality = (uint8_t)atoi(p);//Fix quality

    p = strchr(p, ',')+1;
    loc->satellites = (uint8_t)atoi(p);

    p = strchr(p, ',')+1;

    p = strchr(p, ',')+1;
    loc->altitude = atof(p);
	DEBUG_D("\nLat: %f, lon: %f\n", loc->latitude, loc->longitude ); 
	
	// update 
	if( hh != 0 || mm != 0 || ss != 0)
	{
		gGpsData.gpsHh = hh;
		gGpsData.gpsMm = mm;
		gGpsData.gpsSs = ss;	
		RtcSetNewTimeValue(hh, mm, ss);
	}
}

void nmea_parse_gprmc(char *nmea, gprmc_t *loc)
{
    char *p = nmea; char status;
	float time;
	unsigned long time_nmea;
	unsigned char hh,mm,ss,day,mon,year;
	//printf(nmea);
    p = strchr(p, ',')+1; //skip time	1

	time = atof(p);
	time_nmea =time;
	hh = time_nmea/10000; 
	time_nmea %= 10000;
	mm = time_nmea/100; 	
	ss = time_nmea%100; 
	
	//m_s = time_nmea;
	DEBUG_D("UTC time(GPRMC): %.03f %02d:%02d:%02d\r\n",time,hh,mm,ss);

    p = strchr(p, ',')+1; //skip status	 2
	status = atof(p);
	//status = (char) p;
	if(status != 'A') return;
	
    p = strchr(p, ',')+1;				  //3
    loc->latitude = atof(p);

    p = strchr(p, ',')+1;					 //4
    switch (p[0]) {
        case 'N':
            loc->lat = 'N';
            break;
        case 'S':
            loc->lat = 'S';
            break;
        case ',':
            loc->lat = '\0';
            break;
    }

    p = strchr(p, ',')+1;						//5
    loc->longitude = atof(p);

    p = strchr(p, ',')+1;						   //6
    switch (p[0]) {
        case 'W':
            loc->lon = 'W';
            break;
        case 'E':
            loc->lon = 'E';
            break;
        case ',':
            loc->lon = '\0';
            break;
    }

    p = strchr(p, ',')+1;							  //7	speed
    loc->speed = atof(p);

    p = strchr(p, ',')+1;								 //8 course
    loc->course = atof(p);

	p = strchr(p, ',')+1;								 //9 date
	time = atof(p);
	time_nmea = time;
	day = time_nmea/10000; 
	time_nmea %= 10000;
	mon = time_nmea/100; 	
	year = time_nmea%100; 
	
	//m_s = time_nmea;
	DEBUG_D("UTC date (GPRMC): %.0f %02d-%02d-%02d\r\n",time,day,mon,year);
    if(year < 90)
        year += 100;
    mon -= 1; 
	DEBUG_D("\nLat: %f, lon: %f\n", loc->latitude, loc->longitude );

	// update time
	if( hh != 0 || mm != 0 || ss != 0 )
	{
		gGpsData.gpsHh = hh;
		gGpsData.gpsMm = mm;
		gGpsData.gpsSs = ss;		
	}	
	// update date
	if( day != 0 || mon != 0 || year != 0 )
	{
		gGpsData.gpsDay = day;
		gGpsData.gpsMonth = mon;
		gGpsData.gpsYear = year;
		RtcSetNewCalenarValue(hh, mm, ss, day, mon, year);
	}	
	// update position
	if( loc->latitude != 0 || loc->longitude != 0)
	{
		gGpsData.gpsLatitude = (uint32_t) loc->latitude * 1000;
		gGpsData.gpsLongtitude = (uint32_t) loc->longitude * 1000;
	}	
}

/**
 * Get the message type (GPGGA, GPRMC, etc..)
 *
 * This function filters out also wrong packages (invalid checksum)
 *
 * @param message The NMEA message
 * @return The type of message if it is valid
 */
uint8_t nmea_get_message_type(unsigned char *message)
{
    uint8_t checksum = 0;
    if ((checksum = nmea_valid_checksum(message)) != _EMPTY) {
        return checksum;
    }

    if (strstr((const char *) message, NMEA_GPGGA_STR) != NULL) {
        return NMEA_GPGGA;
    }

    if (strstr((const char *) message, NMEA_GPRMC_STR) != NULL) {
        return NMEA_GPRMC;
    }
    return NMEA_UNKNOWN;
}

uint8_t nmea_valid_checksum(unsigned char * message) {
    uint8_t checksum= (uint8_t)strtol(strchr((const char*) message, '*')+1, NULL, 16);

    char p;
    uint8_t sum = 0;
    ++message;
    while ((p = *message++) != '*') {
        sum ^= p;
    }

    if (sum != checksum) {
        return NMEA_CHECKSUM_ERR;
    }

    return _EMPTY;
}


int parseGPSStr(unsigned char *str1)
{
	unsigned char status = 0;
	gpgga_t gpgga;
    gprmc_t gprmc;
    switch (nmea_get_message_type(str1)) 
	{
        case NMEA_GPGGA:
            nmea_parse_gpgga((char *) str1, &gpgga);        
            status |= NMEA_GPGGA;
        break;
        case NMEA_GPRMC:
            nmea_parse_gprmc((char *) str1, &gprmc);
			status |= NMEA_GPRMC;
            break;
    }					 
    return status; // = 0 (not parse); = 1 RMC; = 2 GGA;
}
