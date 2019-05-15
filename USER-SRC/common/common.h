#ifndef COMMON_LIB_H
#define COMMON_LIB_H

#if   defined ( __CC_ARM )
  #define __ASM            __asm                                      /*!< asm keyword for ARM Compiler          */
  #define __INLINE         __inline                                   /*!< inline keyword for ARM Compiler       */
  #define __STATIC_INLINE  static __inline
#elif defined ( __ICCARM__ )
  #define __ASM            __asm                                      /*!< asm keyword for IAR Compiler          */
  #define __INLINE         inline                                     /*!< inline keyword for IAR Compiler. Only available in High optimization mode! */
  #define __STATIC_INLINE  static inline
#elif defined ( __GNUC__ )
  #define __ASM            __asm                                      /*!< asm keyword for GNU Compiler          */
  #define __INLINE         inline                                     /*!< inline keyword for GNU Compiler       */
  #define __STATIC_INLINE  static inline
#endif

#include "version.h"

#include <stdint.h>
#include <stddef.h>
#include <stdlib.h>
#include <stdio.h>	
#include <string.h>  
  
#include "stm32l4xx_hal.h"
#include "stm32l4xx.h"  
#include "core_cmFunc.h"
	
#include "main.h"
#include "cmsis_os.h"

#include "FreeRTOS.h"
#include "task.h"
#include "timers.h"
#include "queue.h"
#include "semphr.h"
#include "event_groups.h"

#ifdef __cplusplus
extern "C" {
#endif
	
#pragma pack(push,1)
//====== GPS DATA ==============================
typedef struct
{
	uint32_t gpsLatitude;
	uint32_t gpsLongtitude;
	uint16_t gpsAltide;
	uint8_t gpsHh;
	uint8_t gpsMm;
	uint8_t gpsSs;
	uint8_t gpsDay;
	uint8_t gpsMonth;
	uint16_t gpsYear;
}GPSData;
	
typedef struct
{
	uint16_t gamma;
	uint16_t neutron;
	uint16_t dutru;
}RadioActiveData;

typedef struct
{
	uint16_t batteryLevel;
	uint16_t cpuTemp;
}DeviceInfor;

typedef struct
{
	float gammar;
	float neutron;
	uint8_t hour;
	uint8_t min;
	uint8_t seconds;
	uint8_t date;
	uint8_t month;
	uint16_t year;
	uint32_t lat;
	uint32_t lon;
}DataMsgParser;

//======SYSTEM TIME=============================
typedef struct
{
  uint8_t Hour;
  uint8_t Min;
  uint8_t Sec;
  uint8_t Day;
  uint8_t Month;
  uint16_t Year;
}SystemTime;

//======SERVER PARAMS CONFIG====================
typedef struct
{
	uint32_t IP_Server;
	uint16_t Port;
	uint32_t Password;
	unsigned char ClientID[16]; //Can be IMEI...
	uint8_t Enable_GPRS;
	uint32_t Revision_Firmware;
	uint32_t SensorBaudrate;
}ServerConfig;

//======SETUP TIME PROCESSING===================
typedef struct
{
	uint32_t Cycle_SendGPRS_Data;
	uint32_t Cycle_SensorTimeOut;
}Cycle_TimeData;



//======SYSTEM INFORMATION======================
typedef struct
{
	uint16_t CPU_Temp;     // oC
	uint16_t VBAT_Value;   // mV
	uint32_t SystemUptime; // Second
	uint8_t Flag_Change_Server;
	uint8_t Flag_Sensor_OverRange;
	uint32_t Cycle_MDM_Led;
	uint32_t Delta_SensorLasttime;
	unsigned char CellularIMEI[16];
	int GSM_RSSI;
}SystemInformation;

//==============================================
typedef struct
{
	ServerConfig Flash_ServerConfig;
	Cycle_TimeData Flash_CycleTimeData;
}Flash_Config_Store;

typedef struct
{
	char IMEI[15];
	char Command;
	SystemTime Time;
	char Data[1];
}DataPackage;

#pragma pack(pop)

//======COMMON FUNCTION=========================

void HAL_Make_First_System_Parameter(void);

void Set_Default_System_Config(void);

void HAL_Delay_ms(uint32_t);

void HAL_Core_System_Reset(void);

// get-set function
// set gps data
void GpsDataSet(GPSData gpsDataParam);

// get GPS data
GPSData GpsDataGet(void);

// pack data BKRAD-BN
//msgType: Kieu ban tin (0: Bac Ninh, 1 Phu Yen);
//proType: Kieu truyen thong (0: LoRa, 1: GPRS-SOCKET, 2: WiSUN);
//version: Phien ban, hien tai mac dinh bang khong

void CreateDataPackage(uint8_t msgType, uint8_t version, uint8_t proType);



#ifdef __cplusplus
}
#endif

#endif
