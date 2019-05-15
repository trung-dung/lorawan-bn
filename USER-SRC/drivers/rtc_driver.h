#ifndef RTC_DRIVER_H
#define RTC_DRIVER_H

#include "common.h"

#ifdef __cplusplus
extern "C" {
#endif
extern SystemTime timeSystemCr;
	
void RTC_UpdateSystemTime(void);
	
uint8_t RTC_SetNewTime(uint8_t HH, uint8_t MIN, uint8_t SEC, uint8_t DD, uint8_t MM, uint16_t YYYY);
	
uint8_t RTC_SetTime(uint8_t hh, uint8_t min, uint8_t sec);	
	
uint8_t RTC_SetDate(uint8_t day, uint8_t month, uint16_t year);	
	
#ifdef __cplusplus
}
#endif

#endif
