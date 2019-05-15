#include <stdlib.h>
#include "common.h"
#include "debug_hal.h"
#include "task_systemMonitor.h"

// test RTC clock 
#include "rtc-board.h"

#define ENABLE_SYS_REPORT 0

#if (ENABLE_SYS_REPORT == 1)
	extern SystemInformation systemInfo;
	extern ServerConfig serverCfg;
	extern SystemTime timeSystemCr;
	extern Cycle_TimeData timeDataCtl;
	extern RadioActiveData gRadioActiveData;
#endif


static const char *taskStates[] = { "Unknown","Ready","Running","Blocked","Suspended","Deleted" };

static void Get_System_Task_State(void);

void vtaskMonitor(void *params)
{
	vTaskDelay(1000);
	RTC_DateTypeDef sDate; RTC_TimeTypeDef sTime;
	while(1)
	{
		//RTC_UpdateSystemTime();
		RtcGetCalendarValue(&sDate, &sTime);
		#if (ENABLE_SYS_REPORT == 1)
		DEBUG_D("\r\n\n###### ===== System Reporter ===== ######");
		//Sys Time
		DEBUG_D("\r\n###### ===== System Time ===== ######");
		DEBUG_D("\r\n%02d:%02d:%02d %02d/%02d/%04d", sTime.Hours, sTime.Minutes, sTime.Seconds, sDate.Date, sDate.Month, sDate.Year + 2000);
		
		//Sensor Data
		DEBUG_D("\r\n###### ===== Sensor Data ===== ######");
		DEBUG_D("\r\nGammar: %d -- Neutron: %d", gRadioActiveData.gamma, gRadioActiveData.neutron);
		
		//Server Cfg
		DEBUG_D("\r\n###### ===== Server Config ===== ######");
		DEBUG_D("\r\nServer Addr : %d.%d.%d.%d : %d",serverCfg.IP_Server>>24, serverCfg.IP_Server >> 16 & 0xff, serverCfg.IP_Server >>  8 & 0xff, serverCfg.IP_Server  &  0xff, serverCfg.Port);
		DEBUG_D("\r\nClient ID   : %s", serverCfg.ClientID);
		DEBUG_D("\r\nCellularIMEI: %s", systemInfo.CellularIMEI);
		DEBUG_D("\r\nEnable GPRS : %s", serverCfg.Enable_GPRS ? "ENABLED" : "DISABLED");
		
		//Data Control
		DEBUG_D("\r\n###### ===== System Cycle Data ===== ######");
		DEBUG_D("\r\nCycle GPRS  : %10d [s]", timeDataCtl.Cycle_SendGPRS_Data/1000);
		DEBUG_D("\r\nCycle MDM   : %10d [ms]", systemInfo.Cycle_MDM_Led);
		DEBUG_D("\r\nSenTimeout  : %10d [s]", timeDataCtl.Cycle_SensorTimeOut);
		DEBUG_D("\r\nSensorBaud  : %10d [bps]", serverCfg.SensorBaudrate);

		//Sys Info
		DEBUG_D("\r\n###### ===== Sys Infomation ===== ######");
		DEBUG_D("\r\nFirmware Rev: %10d", serverCfg.Revision_Firmware);		
		DEBUG_D("\r\nSignal RSSI : %10d [dBm]", systemInfo.GSM_RSSI);
		DEBUG_D("\r\nUptime      : %10d [s]", systemInfo.SystemUptime);
		DEBUG_D("\r\nLast Sensor : %10d [s]", systemInfo.Delta_SensorLasttime);
		DEBUG_D("\r\nFree heap   : %10d [bytes]", xPortGetFreeHeapSize());
		DEBUG_D("\r\n###### ===== Task Status ===== ######");
		Get_System_Task_State();
		
		DEBUG_D("\r\n###### ===== SYS REPORT END ==== ######\n");
		#endif
		vTaskDelay(10000);
	}
}

static void Get_System_Task_State()
{
	uint32_t uTotalRuntime;
	UBaseType_t uNumTasks;
	TaskStatus_t *pTS;
	int i;
	
	uNumTasks = uxTaskGetNumberOfTasks();
	pTS = (TaskStatus_t *)pvPortMalloc(sizeof(TaskStatus_t) * uNumTasks);
	
	if( pTS == NULL ) {
		DEBUG_D("\r\nOut of memory..");
	}

	uNumTasks = uxTaskGetSystemState((TaskStatus_t *)pTS,uNumTasks,&uTotalRuntime);
	DEBUG_D("\r\nFound %u tasks - Total runtime: %lu",uNumTasks,uTotalRuntime);

	for(i=0;i<uNumTasks;i++) {
		const char *pzState;
		int32_t pct;

		pzState = taskStates[0];
		switch(pTS[i].eCurrentState) {
			case eRunning:   pzState = taskStates[2]; break;
			case eReady:     pzState = taskStates[1]; break;
			case eBlocked:   pzState = taskStates[3]; break;
			case eSuspended: pzState = taskStates[4]; break;
			case eDeleted:   pzState = taskStates[5]; break;
		}

		pct = pTS[i].ulRunTimeCounter;
		pct *= 100;
		pct = pct / uTotalRuntime;

		DEBUG_D("\r\n%16s: %9s %3lu%% SHW:%u",
			pTS[i].pcTaskName,
			pzState,
			pct,
			pTS[i].usStackHighWaterMark);
	}
	vPortFree(pTS);
}
