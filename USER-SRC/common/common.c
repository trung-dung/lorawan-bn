#include <stdlib.h>
#include "common.h"

// DEFINE
#define PAYLOAD_SIZE 41 // 33 bytes + 8 bytes DEV_EUI 
uint8_t DATA_PACKET[PAYLOAD_SIZE]; // ban tin duoc su dung trong BKRAD-BN project

/* =========DEFINE PARAMETER======================= */

/* =========INITIAL SYSTEM PARAMETER================== */
SystemTime timeSystemCr;
ServerConfig serverCfg;
Cycle_TimeData timeDataCtl;
SystemInformation systemInfo;

// global variables, system variables
GPSData gGpsData;
static uint16_t gMsgNum; // so luong ban tin da gui di tu luc khoi dong 
RadioActiveData gRadioActiveData;
static DeviceInfor gDeviceInfor;
DataMsgParser dataMsgParser;
uint8_t DEVICE_EUI[8]  = { 0x11, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xFF };
/* ================================================ */

void HAL_Make_First_System_Parameter(void)
{
	GPSData gpsDataParam;
	
	gMsgNum = 0;
	gRadioActiveData.gamma = 0;
	gRadioActiveData.neutron = 0;
	gRadioActiveData.dutru = 0;
	
	gDeviceInfor.batteryLevel = 0;
	gDeviceInfor.cpuTemp = 0;
	// GPS Data

	gpsDataParam.gpsAltide = 0;
	gpsDataParam.gpsLatitude = 20280361;
	gpsDataParam.gpsLongtitude = 106164101;
	gpsDataParam.gpsDay = 1;
	gpsDataParam.gpsMonth = 5;
	gpsDataParam.gpsYear = 2019;
	gpsDataParam.gpsHh = 0;
	gpsDataParam.gpsMm = 0;
	gpsDataParam.gpsSs = 0;
	GpsDataSet(gpsDataParam);
	
	dataMsgParser.neutron = 0;
	dataMsgParser.gammar = 0;
		
	//System Time
	timeSystemCr.Day   = 01;
	timeSystemCr.Month = 05;
	timeSystemCr.Year  = 2019;
	timeSystemCr.Hour  = 5;
	timeSystemCr.Min   = 10;
	timeSystemCr.Sec   = 0;
	
	//System Infomations
	systemInfo.SystemUptime       = 0;
	systemInfo.CPU_Temp           = 0;
	systemInfo.VBAT_Value         = 0;
	systemInfo.Flag_Change_Server = 0;
	systemInfo.Flag_Sensor_OverRange = 0;
	systemInfo.Cycle_MDM_Led      = 0;
	systemInfo.Delta_SensorLasttime = 0;
	systemInfo.GSM_RSSI           = 0;
	systemInfo.CellularIMEI[0]  = '0';
	systemInfo.CellularIMEI[1]  = '0';
	systemInfo.CellularIMEI[2]  = '0';
	systemInfo.CellularIMEI[3]  = '0';
	systemInfo.CellularIMEI[4]  = '0';
	systemInfo.CellularIMEI[5]  = '0';
	systemInfo.CellularIMEI[6]  = '0';
	systemInfo.CellularIMEI[7]  = '0';
	systemInfo.CellularIMEI[8]  = '0';
	systemInfo.CellularIMEI[9]  = '0';
	systemInfo.CellularIMEI[10] = '0';
	systemInfo.CellularIMEI[11] = '0';
	systemInfo.CellularIMEI[12] = '0';
	systemInfo.CellularIMEI[13] = '0';
	systemInfo.CellularIMEI[14] = '0';
	systemInfo.CellularIMEI[15] = '\0';
	
	//System Config
	Set_Default_System_Config();
}

void Set_Default_System_Config()
{	
	//Server configuration
	serverCfg.IP_Server = (((uint32_t)(202))<<24) | (((uint32_t)(191))<<16) | (((uint32_t)(56))<< 8) | (((uint32_t)(103))<< 0);
	serverCfg.Port      = 5510;
	serverCfg.Password  = 12345;
	serverCfg.ClientID[0]  = '3';
	serverCfg.ClientID[1]  = '5';
	serverCfg.ClientID[2]  = '4';
	serverCfg.ClientID[3]  = '7';
	serverCfg.ClientID[4]  = '2';
	serverCfg.ClientID[5]  = '5';
	serverCfg.ClientID[6]  = '0';
	serverCfg.ClientID[7]  = '6';
	serverCfg.ClientID[8]  = '5';
	serverCfg.ClientID[9]  = '5';
	serverCfg.ClientID[10] = '0';
	serverCfg.ClientID[11] = '8';
	serverCfg.ClientID[12] = '1';
	serverCfg.ClientID[13] = '3';
	serverCfg.ClientID[14] = '1';
	serverCfg.ClientID[15] = '\0';
	serverCfg.Enable_GPRS  = 1;
	serverCfg.SensorBaudrate = 9600;
	serverCfg.Revision_Firmware  = 0;
	
	//Time process data(ms)
	timeDataCtl.Cycle_SendGPRS_Data = 60000; // ms
	timeDataCtl.Cycle_SensorTimeOut = 60;    // s
}

void HAL_Delay_ms(uint32_t nTime)
{
	HAL_Delay(nTime);
}

void HAL_Core_System_Reset(void)
{
	NVIC_SystemReset();
	while(1);
}


// set gps data
void GpsDataSet(GPSData gpsDataParam)
{
	gGpsData.gpsAltide = gpsDataParam.gpsAltide;
	gGpsData.gpsLatitude = gpsDataParam.gpsLatitude;
	gGpsData.gpsLongtitude = gpsDataParam.gpsLongtitude;
	gGpsData.gpsDay = gpsDataParam.gpsDay;
	gGpsData.gpsMonth = gpsDataParam.gpsMonth;
	gGpsData.gpsYear = gpsDataParam.gpsYear;
	gGpsData.gpsHh = gpsDataParam.gpsHh;
	gGpsData.gpsMm = gpsDataParam.gpsMm;
	gGpsData.gpsSs = gpsDataParam.gpsSs;
}

// get GPS data
GPSData GpsDataGet(void)
{
	GPSData gpsDateValue;
	gpsDateValue.gpsAltide = gGpsData.gpsAltide;
	gpsDateValue.gpsLatitude = gGpsData.gpsLatitude;
	gpsDateValue.gpsLongtitude = gGpsData.gpsLongtitude;
	gpsDateValue.gpsDay = gGpsData.gpsDay;
	gpsDateValue.gpsMonth = gGpsData.gpsMonth;
	gpsDateValue.gpsYear = gGpsData.gpsYear;
	gpsDateValue.gpsHh = gGpsData.gpsHh;
	gpsDateValue.gpsMm = gGpsData.gpsMm;
	gpsDateValue.gpsSs = gGpsData.gpsSs;	
	return gpsDateValue;
}

void CreateDataPackage(uint8_t msgType, uint8_t version, uint8_t proType)
{
	uint8_t i = 0;
	uint8_t j = 0;
	uint16_t msgNum = gMsgNum++; // when call this function gMsgNum increase 1
	memset(DATA_PACKET, 0, SIZE_MAX);
	if(version == 0)
	{   //1. HEADER
		//1.1 Control information
		if(msgType == 0) DATA_PACKET[i] = 0x00; // BN
		else if(msgType == 1) DATA_PACKET[i] = 0x01; // PHU YEN
		else return;
		if(proType == 0) DATA_PACKET[++i] = 0x00; // LORA
		else if(proType == 1) DATA_PACKET[++i] = 0x10; // GPRS
		else return;
		DATA_PACKET[++i] = (msgNum >> 8) & 0xFF;  // number of packet - byte #2
		DATA_PACKET[++i] = msgNum & 0xFF; 
		DATA_PACKET[++i] = 0x00; // checksum
		DATA_PACKET[++i] = 0x00; // checksum
		//1.2 LORAWAN DEV EUI #6 - 13 
		for(j = 0; j < 8; j++)
		{
			DATA_PACKET[++i] = DEVICE_EUI[j];
		}
		//2. DATA PAYLOAD
		//2.1 Radioactive Data 
		DATA_PACKET[++i] = (gRadioActiveData.gamma >> 8) & 0xFF; //#14
		DATA_PACKET[++i] = gRadioActiveData.gamma & 0xFF;
		DATA_PACKET[++i] = (gRadioActiveData.neutron >> 8) & 0xFF; // #16
		DATA_PACKET[++i] = gRadioActiveData.neutron & 0xFF;
		DATA_PACKET[++i] = (gRadioActiveData.dutru >> 8) & 0xFF; // #18
		DATA_PACKET[++i] = gRadioActiveData.dutru & 0xFF;
		//2.2 Time data (GMT from GPS)
		DATA_PACKET[++i] = gGpsData.gpsDay; //  ngay            // # 20
		DATA_PACKET[++i] = gGpsData.gpsMonth; // thang          // #21
		DATA_PACKET[++i] = (gGpsData.gpsYear >> 8) & 0xFF; // nam //#22
		DATA_PACKET[++i] = gGpsData.gpsYear & 0xFF;
		DATA_PACKET[++i] = gGpsData.gpsHh; //#24
		DATA_PACKET[++i] = gGpsData.gpsMm; //#25
		DATA_PACKET[++i] = gGpsData.gpsSs; //#26
		//2.3 Position data (from GPS lat - long - altitude)
		DATA_PACKET[++i] = ( gGpsData.gpsLatitude >> 24 ) & 0xFF; //#27
		DATA_PACKET[++i] = ( gGpsData.gpsLatitude >> 16 ) & 0xFF;
		DATA_PACKET[++i] = ( gGpsData.gpsLatitude >> 8 ) & 0xFF;
		DATA_PACKET[++i] = gGpsData.gpsLatitude & 0xFF ;
		DATA_PACKET[++i] = ( gGpsData.gpsLongtitude >> 24 ) & 0xFF; //#31
		DATA_PACKET[++i] = ( gGpsData.gpsLongtitude >> 16 ) & 0xFF;
		DATA_PACKET[++i] = ( gGpsData.gpsLongtitude >> 8 ) & 0xFF;
		DATA_PACKET[++i] = gGpsData.gpsLongtitude & 0xFF ;
		DATA_PACKET[++i] = ( gGpsData.gpsAltide >> 8 ) & 0xFF; //#35
		DATA_PACKET[++i] = gGpsData.gpsAltide & 0xFF ;
		//2.4 Device infor
		DATA_PACKET[++i] = ( gDeviceInfor.batteryLevel >> 8 ) & 0xFF;
		DATA_PACKET[++i] = gDeviceInfor.batteryLevel & 0xFF ;
		DATA_PACKET[++i] = ( gDeviceInfor.cpuTemp >> 8 ) & 0xFF;
		DATA_PACKET[++i] = gDeviceInfor.cpuTemp & 0xFF ;
		return;
	}
}

