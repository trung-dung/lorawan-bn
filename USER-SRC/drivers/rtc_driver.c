// Set new time to RTC
#include "rtc_driver.h"

RTC_HandleTypeDef hrtc;

#define RTC_LEAP_YEAR(year)		((((year) % 4 == 0) && ((year) % 100 != 0)) || ((year) % 400 == 0))
/* Days in a month */
static uint8_t RTC_Months[2][12] = {
	{31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31},	/* Not leap year */
	{31, 29, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31}	/* Leap year */
};


void RTC_UpdateSystemTime(void)
{
	RTC_TimeTypeDef sTime;
	RTC_DateTypeDef sDate;
	/* Get the RTC current Time */
	HAL_RTC_GetTime(&hrtc, &sTime, RTC_FORMAT_BIN);
	/* Get the RTC current Date */
	HAL_RTC_GetDate(&hrtc, &sDate, RTC_FORMAT_BIN);	
	
	timeSystemCr.Day = sDate.Date;
	timeSystemCr.Month = sDate.Month;
	timeSystemCr.Year = sDate.Year + 2000;
	timeSystemCr.Hour = sTime.Hours;
	timeSystemCr.Min = sTime.Minutes;
	timeSystemCr.Sec = sTime.Seconds;
}

uint8_t RTC_SetTime(uint8_t hh, uint8_t min, uint8_t sec)
{
	RTC_TimeTypeDef sTime;	
	if(hh > 23 || min > 59 || sec > 59) return 0;
	sTime.Hours = hh;
	sTime.Minutes = min;
	sTime.Seconds = sec;
	sTime.SubSeconds = 0;
	sTime.DayLightSaving = 0;
	sTime.StoreOperation = 0;
	if(HAL_RTC_SetTime(&hrtc, &sTime, RTC_FORMAT_BIN) == HAL_OK) return 0;	
	else return 1;
}

uint8_t RTC_SetDate(uint8_t day, uint8_t month, uint16_t year)
{
	RTC_DateTypeDef sDate;
	uint8_t yy = year - 2000;
	if(	yy > 99 || 
		month == 0 || 
		month > 12 ||
		day == 0 ||
		day > RTC_Months[RTC_LEAP_YEAR(2000 + yy) ? 1 : 0][month - 1]) return 0;
	sDate.Month = month;
	sDate.Date = day;
	sDate.Year = yy;
	sDate.WeekDay = 0x00;
	if (HAL_RTC_SetDate(&hrtc, &sDate, RTC_FORMAT_BIN) == HAL_OK) return 0;
	else
		return 1;
}

uint8_t RTC_SetNewTime(uint8_t HH, uint8_t MIN, uint8_t SEC, uint8_t DD, uint8_t MM, uint16_t YYYY)
{
	RTC_TimeTypeDef sTime;
	RTC_DateTypeDef sDate;	
	/* Check year and month */
	uint8_t YY = YYYY - 2000;
	if (
		YY > 99 || 
		MM == 0 || 
		MM > 12 ||
		DD == 0 ||
		DD > RTC_Months[RTC_LEAP_YEAR(2000 + YY) ? 1 : 0][MM - 1] ||
		HH > 23 ||
		MIN > 59 ||
		SEC > 59
	) return 0;
	//Config if 
	sTime.Hours = HH;
	sTime.Minutes = MIN;
	sTime.Seconds = SEC;
//	sTime.SubSeconds = 0;
//	sTime.DayLightSaving = 0;
//	sTime.StoreOperation = 0;
	HAL_RTC_SetTime(&hrtc, &sTime, RTC_FORMAT_BIN);

	sDate.Month = MM;
	sDate.Date = DD;
	sDate.Year = YYYY - 2000;
	HAL_RTC_SetDate(&hrtc, &sDate, RTC_FORMAT_BIN);
	return 1;
}
