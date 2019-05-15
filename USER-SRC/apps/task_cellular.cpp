#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "common.h"
#include "task_cellular.h"
#include "mdm_hal.h"
#include "mdm_socket_hal.h"
#include "main_loadconfig.h"
#include "debug_hal.h"
#include "systick_driver.h"
#include "base85.h"
#include "rtc_driver.h"

/* Private typedef ----------------------------------------------------------*/
enum MODEM_RUNNING_STATE
{
	MODEM_REG_NETWORK = 0, 				//Startup
	MODEM_CREAT_SOCKET_HANDLER,			//Creat socket handler
	MODEM_CREAT_CONNECTION,   			//Connect to server
	MODEM_SEND_PACKAGE,					//Send Data
	MODEM_CLOSE_SOCKET					//Close socket
};

enum PACKAGE_TYPE
{
	AUTHENTICATE = 0,				//Authentication if need
	CLOCK_SYNC,						//Synchronous time from server
	NORMAL_DATA						//Normal data package
};

/* Private define -----------------------------------------------------------*/
#define MAXSIZE_ENCRYPT_BASE85 256
#define MAXSIZE_SOCKET_RECV    256
#define MAXSIZE_SMS_MESSAGE    159

MDM_DeviceSerial thanhntMDM;
extern SensorData sensorData;
extern SystemInformation systemInfo;
extern ServerConfig serverCfg;
extern SystemTime timeSystemCr;
extern Cycle_TimeData timeDataCtl;
DataPackage gprsDataPackage;

xSemaphoreHandle sem_Modem_Bus = 0;

unsigned char en_base85_buff[MAXSIZE_ENCRYPT_BASE85], en_base85_data_send[MAXSIZE_ENCRYPT_BASE85];
char socket_recv_buff[MAXSIZE_SOCKET_RECV];
char sms_revc_buff[MAXSIZE_SMS_MESSAGE];
char sms_send_buff[MAXSIZE_SMS_MESSAGE];
char ussd_ret_buff[MAXSIZE_SMS_MESSAGE];
long socket_Create;
NetStatus netStatus_MDM;

/* Private macro ------------------------------------------------------------*/

int Send_DataPakage_Normal(char command, long socket_created);
void Process_CMD_From_Server(long socket_created, char *num_from, char *buffer);
void Process_CMD_Response(long socket_created, char *num_from, char *buffer);

/* Task Create --------------------------------------------------------------*/
void vtaskCelluar(void const * argument)
{
	//Init state
	MODEM_RUNNING_STATE modemState = MODEM_REG_NETWORK;
	//Init variables
	sockaddr_t socketAddr;
	//Get device infor
	DevStatus devStatus = {};
	int socket_Connect = 0, send_Socket_Result = 0;
	int retry_socket = 0, retry_connection = 0, try_use_mdm = 0;
	
	while(1)
	{
		switch(modemState)
		{
			//=========================================================================//
			case MODEM_REG_NETWORK:
				systemInfo.Cycle_MDM_Led = 100;
				if(xSemaphoreTake(sem_Modem_Bus,portMAX_DELAY))
				{
					thanhntMDM.reset();
					if(thanhntMDM.connect())
					{
						modemState = MODEM_CREAT_SOCKET_HANDLER;
						// Get hardware information when initialization successful
						thanhntMDM.getDevStatus(&devStatus);
						memcpy(systemInfo.CellularIMEI, devStatus.imei, 16);
					}
					xSemaphoreGive(sem_Modem_Bus);
				}
			break;
			//=========================================================================//
			case MODEM_CREAT_SOCKET_HANDLER:
				//Create socket
				systemInfo.Cycle_MDM_Led = 500;
			
				//Check GPRS Enable?
				if(!serverCfg.Enable_GPRS)
				{
					vTaskDelay(30000); //Free time to system check sms if disable gprs
					break;
				}
				
				//Reset temp variables
				memset(&socketAddr, 0, sizeof(socketAddr));
				socket_Create = -1;

				if(xSemaphoreTake(sem_Modem_Bus, portMAX_DELAY))
				{
					//Creat socket on Modem
					socket_Create = socket_create(AF_INET, SOCK_STREAM, IPPROTO_TCP, 0, NIF_DEFAULT);
					xSemaphoreGive(sem_Modem_Bus);
					if(socket_Create < 0) //Check create socket OK?
					{
						retry_socket++;
						vTaskDelay(10000);
						if(retry_socket > 3) 
						{
							modemState = MODEM_REG_NETWORK; //Try -> Not OK => Reset
							retry_socket = 0;
						}
					}
					else modemState = MODEM_CREAT_CONNECTION;
				}
			break;
			//=========================================================================//
			case MODEM_CREAT_CONNECTION:
				systemInfo.Cycle_MDM_Led = 1000;
				if(!serverCfg.Enable_GPRS)
				{
					modemState = MODEM_CLOSE_SOCKET;
					break;
				}
				if(systemInfo.Flag_Change_Server)
				{
					systemInfo.Flag_Change_Server = 0;
					modemState = MODEM_CLOSE_SOCKET;
					break;
				}
				// Reset temp variables
				socket_Connect = 0;
				
				// the family is always AF_INET
				socketAddr.sa_family = AF_INET;
				// the destination port: 5532
				socketAddr.sa_data[0] = serverCfg.Port >> 8; 		//High nibble
				socketAddr.sa_data[1] = serverCfg.Port  & 0xff;	//Low nibble
				// the destination IP address: 202.191.56.102
				socketAddr.sa_data[2] = serverCfg.IP_Server >> 24;
				socketAddr.sa_data[3] = serverCfg.IP_Server >> 16 & 0xff;
				socketAddr.sa_data[4] = serverCfg.IP_Server >>  8 & 0xff;
				socketAddr.sa_data[5] = serverCfg.IP_Server  &  0xff;
				// Connect socket to address
				if(xSemaphoreTake(sem_Modem_Bus, portMAX_DELAY))
				{
					socket_Connect = socket_connect(socket_Create, &socketAddr, sizeof (socketAddr));
					xSemaphoreGive(sem_Modem_Bus);
					if(!socket_Connect) // Connect OK
					{
						modemState = MODEM_SEND_PACKAGE;
					}
					else 
					{
						retry_connection++;
						try_use_mdm++;
						vTaskDelay(20000); //Free time to system check sms if change server
						if(retry_connection > 3) 
						{
							modemState = MODEM_CLOSE_SOCKET;
							retry_connection = 0;
						}
					}
				}
			break;
			//=========================================================================//
			case MODEM_SEND_PACKAGE:
				//send_Socket_Result = socket_send(socket_Create, testString, strlen(testString));
				systemInfo.Cycle_MDM_Led = 5000;
				// Reset temp variables
				send_Socket_Result = 0;
				if(systemInfo.Flag_Change_Server)
				{
					systemInfo.Flag_Change_Server = 0;
					modemState = MODEM_CLOSE_SOCKET;
					break;
				}
				if(!serverCfg.Enable_GPRS)
				{
					modemState = MODEM_CLOSE_SOCKET;
					break;
				}
				if(xSemaphoreTake(sem_Modem_Bus, portMAX_DELAY))
				{
					// Send data
					send_Socket_Result = Send_DataPakage_Normal(0x03, socket_Create);
					xSemaphoreGive(sem_Modem_Bus);
					if(send_Socket_Result<0) { //Sending error
						modemState = MODEM_CLOSE_SOCKET;
						vTaskDelay(10000);
						break;
					}
				}
				vTaskDelay(timeDataCtl.Cycle_SendGPRS_Data);
			break;
			//=========================================================================//
			case MODEM_CLOSE_SOCKET:
				if(xSemaphoreTake(sem_Modem_Bus, portMAX_DELAY))
				{
					socket_close(socket_Create);
					xSemaphoreGive(sem_Modem_Bus);
					if(try_use_mdm < 5)
					{
						modemState = MODEM_CREAT_SOCKET_HANDLER;
					}
					else 
					{
						modemState = MODEM_REG_NETWORK;
						try_use_mdm = 0;
					}
				}
			break;
		}
		vTaskDelay(100);
	}
}

void vtaskCommandSv(void const * argument)
{
	int socket_num_recv = 0;
	while(1)
	{
		if(systemInfo.Cycle_MDM_Led > 1000) //Connected
		{
			if(xSemaphoreTake(sem_Modem_Bus, portMAX_DELAY))
			{
				// Check recv data
				socket_num_recv = thanhntMDM.socketReadable(socket_Create);
				if(socket_num_recv > 0)
				{
					//Clear buffer
					memset(socket_recv_buff, 0, sizeof(socket_recv_buff));
					socket_receive(socket_Create, &socket_recv_buff, socket_num_recv, 1000); //Recv with timeout
					Process_CMD_From_Server(socket_Create, "", (char *)&socket_recv_buff);
				}
				xSemaphoreGive(sem_Modem_Bus);
			}
		}
		vTaskDelay(1000);
	}
}

void vtaskSms(void const * argument)
{
	int numSMS = 0, ix[3];
	char fromNumber[16];
	while(1)
	{
		if(systemInfo.Cycle_MDM_Led > 100) //Registered to network
		{
			if(xSemaphoreTake(sem_Modem_Bus, portMAX_DELAY))
			{
				//Get Network Status
				thanhntMDM.getSignalStrength(netStatus_MDM);
				systemInfo.GSM_RSSI = netStatus_MDM.rssi;
				
				//Check SMS and Process if exist
				numSMS = thanhntMDM.smsList("ALL",ix, 3);
				if(numSMS > 3) numSMS = 3;
				while(0 < numSMS--)
				{
					memset(&sms_revc_buff, 0, sizeof(sms_revc_buff));
					memset(&fromNumber, 0, sizeof(fromNumber));
					if(thanhntMDM.smsRead(ix[numSMS], (char *)&fromNumber, (char *)&sms_revc_buff, sizeof(sms_revc_buff)))
					{
						//DEBUG_D("\nGot at [%d] from %s: %s\n", ix[numSMS], fromNumber, sms_revc_buff);
						thanhntMDM.smsDelete(ix[numSMS]);
						Process_CMD_From_Server(-1, (char *)&fromNumber, (char *)&sms_revc_buff);
					}
				}
				xSemaphoreGive(sem_Modem_Bus);
			}
		}
		vTaskDelay(10000);
	}
}

/* Functions Create --------------------------------------------------------------*/
int Send_DataPakage_Normal(char command, long socket_created)
{
	int i, send_result, base_data_size = 0;
	char buff_header[6];
	int en_base85_len = 0;

	//put IMEI
	for(i = 0; i<15; i++) gprsDataPackage.IMEI[i] = serverCfg.ClientID[i];
	//put Command
	gprsDataPackage.Command = command;
	//put Time
	gprsDataPackage.Time = timeSystemCr;
	//Clear buffer
	memset(en_base85_buff, 0, MAXSIZE_ENCRYPT_BASE85);
	memset(en_base85_data_send, 0, MAXSIZE_ENCRYPT_BASE85);
	
	memcpy(&gprsDataPackage.Data, &sensorData, sizeof(SensorData));

	base_data_size = sizeof(DataPackage) + sizeof(SensorData) - 1;

	memcpy(&en_base85_data_send,&gprsDataPackage,base_data_size);

	base85_encode((unsigned char *)&en_base85_buff+6,&en_base85_len,(unsigned char *)&en_base85_data_send, base_data_size);
	sprintf(buff_header, "@QN%03x", en_base85_len+6);
	memcpy(&en_base85_buff, buff_header,6);
	//Send
	send_result = socket_send(socket_created, &en_base85_buff, en_base85_len+6);
	
	return send_result;
}

void Process_CMD_From_Server(long socket_created, char *num_from, char *buffer)
{
	short day, mon, year, hour, min, sec;
	uint32_t password, password_new, timer_adc, timer_gprs, timer_log2sd, _sensorTimeout, _sensorbaud;
	int ip1, ip2, ip3, ip4, port,on_off_sd_gprs, ret_ussd = 0;
	char new_id[15], sms_send_to[15], buff_ussd[20];
	
	if(strncmp(buffer, "#W53 ", 5) == 0)//Set time RTC
	{
		if(sscanf(buffer, "#W53 %2hd%2hd%2hd%2hd%2hd%2hd.*",&day,&mon,&year,&hour,&min,&sec) == 6)
			RTC_SetNewTime(hour, min, sec, day, mon, year + 2000);
	}
	else if(strncmp(buffer, "#INFOR", 6) == 0) //Get device information - #SYS_INFOR,12345.*
	{
		sscanf(buffer, "#INFOR,%u.*",&password);
		if(serverCfg.Password == password)
		{
			//SysInfor
			memset(&sms_send_buff, 0, sizeof(sms_send_buff));
			sprintf(sms_send_buff, "\nFirmRev:%d\nID:%s\nIMEI:%s\nUp:%u[s]\nServer:" IPSTR ":%d\nRSSI:%d[dBm]\nSD:%s - %s\nGPRS:%s\nCPUTemp:%d[oC]\n", serverCfg.Revision_Firmware, serverCfg.ClientID, systemInfo.CellularIMEI, systemInfo.SystemUptime, IPNUM(serverCfg.IP_Server), serverCfg.Port, systemInfo.GSM_RSSI, systemInfo.SDCard_State ? "YES" : "NO", serverCfg.Enable_Log2SD ? "EN" : "DIS", serverCfg.Enable_GPRS ? "EN" : "DIS", systemInfo.CPU_Temp);
			Process_CMD_Response(socket_created, num_from, sms_send_buff);
			//SystemCycleTime
			memset(&sms_send_buff, 0, sizeof(sms_send_buff));
			sprintf(sms_send_buff, "\nSensorUpdate: %u seconds ago\nSensorBaud:%d\nGPRS: %d[s]\nSensorTimeout: %d[s]\n", systemInfo.Delta_SensorLasttime, serverCfg.SensorBaudrate,timeDataCtl.Cycle_SendGPRS_Data/1000, timeDataCtl.Cycle_SensorTimeOut);
			Process_CMD_Response(socket_created, num_from, sms_send_buff);
		}
	}
	else if(strncmp(buffer, "#USSD", 5) == 0) //Run USSD command - #USSD,12345,"*101#".*
	{
		memset(&buff_ussd, 0, sizeof(buff_ussd));
		memset(&sms_send_buff, 0, sizeof(sms_send_buff));
		memset(&ussd_ret_buff, 0, sizeof(ussd_ret_buff));
		ret_ussd = 0;
		if(sscanf(buffer, "#USSD,%u,\"%[^\"]\".*", &password, buff_ussd) == 2)
		{
			if(serverCfg.Password == password)
			{
				ret_ussd = thanhntMDM.ussdCommand(buff_ussd, ussd_ret_buff);
				if(ret_ussd > 0)
				{
					sprintf(sms_send_buff, "\n%s:[%*s]\n", serverCfg.ClientID, ret_ussd, ussd_ret_buff);
					DEBUG_D("\nCommand: %s", buff_ussd);
					DEBUG_D("\nUSSD: %s", sms_send_buff);
					Process_CMD_Response(socket_created, num_from, sms_send_buff);
				}
			}
		}
	}
	else if(strncmp(buffer, "#RESET", 6) == 0) //Reset client - #RESET,12345.*
	{
		sscanf(buffer, "#RESET,%u.*",&password);
		if(serverCfg.Password == password)
		{
			memset(&sms_send_buff, 0, sizeof(sms_send_buff));
			sprintf(sms_send_buff, "\n[RESET! - %s]\n", serverCfg.ClientID);
			Process_CMD_Response(socket_created, num_from, sms_send_buff);
			vTaskDelay(2000);
			HAL_Core_System_Reset();
		}
	}
	else if(strncmp(buffer, "#SERVER", 7) == 0) //Reconfig server info - #SERVER,12345,202.191.56.103,5500.*
	{
		if(sscanf(buffer, "#SERVER,%u,%d.%d.%d.%d,%d.*", &password, &ip1, &ip2, &ip3, &ip4, &port) == 6)
		{
			if(serverCfg.Password == password)
			{
				serverCfg.IP_Server = (((uint32_t)(ip1))<<24) | (((uint32_t)(ip2))<<16) | (((uint32_t)(ip3))<< 8) | (((uint32_t)(ip4))<< 0);
				serverCfg.Port = port;
				systemInfo.Flag_Change_Server = 1;
				Write_Flash_New_Config();
				Process_CMD_Response(socket_created, num_from, "[OK]");
			}
		}
	}
	else if(strncmp(buffer, "#PASSWORD", 9) == 0) //Change Password - #PASSWORD,12345,54321.* (OLD, NEW)
	{
		if(sscanf(buffer, "#PASSWORD,%u,%u.*",&password, &password_new) == 2)
		{
			if(serverCfg.Password == password)
			{
				serverCfg.Password = password_new;
				Write_Flash_New_Config();
				Process_CMD_Response(socket_created, num_from, "[OK]");
			}
		}
	}
	else if(strncmp(buffer, "#SENSOR_BAUD", 12) == 0) //Change Sensor Baudrate - #SENSOR_BAUD,12345,9600.*
	{
		if(sscanf(buffer, "#SENSOR_BAUD,%u,%u.*",&password, &_sensorbaud) == 2)
		{
			if(serverCfg.Password == password)
			{
				serverCfg.SensorBaudrate = _sensorbaud;
				Write_Flash_New_Config();
				Process_CMD_Response(socket_created, num_from, "[OK]");
				vTaskDelay(2000);
				HAL_Core_System_Reset();
			}
		}
	}
	else if(strncmp(buffer, "#SETID", 6) == 0) //Set new ID - #SETID,12345,987654321012345.*
	{
		memset(new_id, 0, sizeof(new_id));
		if(sscanf(buffer, "#SETID,%u,%[^.*].*",&password, new_id) == 2) //[^.*] => not include . and * (http://docs.roxen.com/(en)/pike/7.0/tutorial/strings/sscanf.xml)
		{
			if(serverCfg.Password == password)
			{
				if(strlen(new_id) == 15)
				{
					memcpy(serverCfg.ClientID, new_id, 15);
					Write_Flash_New_Config();
					Process_CMD_Response(socket_created, num_from, "[OK]");
				}
			}
		}
	}
	else if(strncmp(buffer, "#SD_ON", 6) == 0) //Enable/Disable Log2SD - #SD_ON,12345,1.*
	{
		if(sscanf(buffer, "#SD_ON,%u,%d.*", &password, &on_off_sd_gprs) == 2)
		{
			if(serverCfg.Password == password)
			{
				serverCfg.Enable_Log2SD = on_off_sd_gprs ? 1 : 0;
				Write_Flash_New_Config();
				Process_CMD_Response(socket_created, num_from, "[OK]");
			}
		}
	}
	else if(strncmp(buffer, "#GPRS_ON", 8) == 0) //Enable/Disable GPRS - #GPRS_ON,12345,0.*
	{
		if(sscanf(buffer, "#GPRS_ON,%u,%d.*", &password, &on_off_sd_gprs) == 2)
		{
			if(serverCfg.Password == password)
			{
				serverCfg.Enable_GPRS = on_off_sd_gprs ? 1 : 0;
				Write_Flash_New_Config();
				Process_CMD_Response(socket_created, num_from, "[OK]");
			}
		}
	}
	else if(strncmp(buffer, "#TIMER", 6) == 0) //Set Timer - #TIMER,12345,[DATA],[LOG],[ADC].* (second)
	{
		if(sscanf(buffer, "#TIMER,%u,%u,%u,%u.*", &password, &timer_gprs, &timer_log2sd, &timer_adc) == 4)
		{
			if(serverCfg.Password == password)
			{
				if((timer_gprs >= 30) && (timer_gprs < 3000000))			
				timeDataCtl.Cycle_SendGPRS_Data = timer_gprs*1000;
				if((timer_log2sd > 0) && (timer_log2sd < 3000000))			
				timeDataCtl.Cycle_Log2SDCARD = timer_log2sd*1000;
				if((timer_adc > 0) && (timer_adc < 3000000))			
				timeDataCtl.Cycle_GetADC = timer_adc*1000;
				Write_Flash_New_Config();
				Process_CMD_Response(socket_created, num_from, "[OK]");
			}
		}
	}
	else if(strncmp(buffer, "#RECOVERY_FACTORY", 17) == 0) //Recovery config to default - #RECOVERY_FACTORY,12345.*
	{
		if(sscanf(buffer, "#RECOVERY_FACTORY,%u.*", &password) == 1)
		{
			if(serverCfg.Password == password)
			{
				Set_Default_System_Config();
				serverCfg.Revision_Firmware -= 1;
				Write_Flash_New_Config();
				Process_CMD_Response(socket_created, num_from, "[OK]");
				vTaskDelay(2000);
				HAL_Core_System_Reset();
			}
		}
	}
	else if(strncmp(buffer, "#TT_CAMBIEN", 11) == 0) //Get sensor info - #SENSOR_INFOR,12345.*
	{
		if(sscanf(buffer, "#TT_CAMBIEN,%u.*", &password) == 1)
		{
			if(serverCfg.Password == password)
			{
				memset(&sms_send_buff, 0, sizeof(sms_send_buff));
				sprintf(sms_send_buff, "Time: %02d:%02d:%02d %02d/%02d/%04d\nID: %s\nNhiet do: %3.1f[oC]\nOxy: %4.2f[mg/l]\nPH: %3.1f\nNH3: %5.3f[mg/l]\nH2S: %5.3f[mg/l]\nMuoi: %5.3f[%]", timeSystemCr.Hour, timeSystemCr.Min, timeSystemCr.Sec, timeSystemCr.Day, timeSystemCr.Month, timeSystemCr.Year, serverCfg.ClientID, (double)sensorData.Temp/10, (double)sensorData.Oxy/100, (double)sensorData.PH/10, (double)sensorData.NH3/1000, (double)sensorData.H2S/1000, (double)sensorData.Salt/1000);
				Process_CMD_Response(socket_created, num_from, sms_send_buff);
			}
		}
	}
	else if(strncmp(buffer, "#SENSOR_TIMEOUT", 15) == 0) //Set Sensor Timeout - #SENSOR_TIMEOUT,12345,180.* (second)
	{
		if(sscanf(buffer, "#SENSOR_TIMEOUT,%u,%u.*", &password,&_sensorTimeout) == 2)
		{
			if(serverCfg.Password == password)
			{
				timeDataCtl.Cycle_SensorTimeOut = _sensorTimeout;
				Write_Flash_New_Config();
				memset(&sms_send_buff, 0, sizeof(sms_send_buff));
				sprintf(sms_send_buff, "Set SensorTimeout to %u[s] OK", timeDataCtl.Cycle_SensorTimeOut);
				Process_CMD_Response(socket_created, num_from, sms_send_buff);
			}
		}
	}
	else if(strncmp(buffer, "#ROOT_TAKE_OWNER", 16) == 0) //BackDoor to take owner - #ROOT_TAKE_OWNER (Critical)
	{
		memset(&sms_send_buff, 0, sizeof(sms_send_buff));
		sprintf(sms_send_buff, "ID:%s\nPWD:%u\nFirmware_rev:%u\nUp:%u[s]\nServer:" IPSTR "\nPort:%d\nRSSI:%d[dBm]\nSD:%s - %s\nGPRS:%s",serverCfg.ClientID, serverCfg.Password, serverCfg.Revision_Firmware, systemInfo.SystemUptime, IPNUM(serverCfg.IP_Server), serverCfg.Port, systemInfo.GSM_RSSI, systemInfo.SDCard_State ? "YES" : "NO", serverCfg.Enable_Log2SD ? "EN" : "DIS", serverCfg.Enable_GPRS ? "EN" : "DIS");
		Process_CMD_Response(socket_created, num_from, sms_send_buff);
	}
	else if(strncmp(buffer, "#SEND_SMS", 9) == 0) //Send SMS to a phone number - #SEND_SMS,12345,"0973518710","SMS Content".*
	{
		memset(&sms_send_buff, 0, sizeof(sms_send_buff));
		memset(&sms_send_to, 0, sizeof(sms_send_to));
		if(sscanf(buffer, "#SEND_SMS,%u,\"%[^\"]\",\"%[^\"]\".*", &password, sms_send_to, sms_send_buff) == 3)
		{
			if(serverCfg.Password == password)
			{
				DEBUG_D("\nSend SMS: \"%s\" to \"%s\"", sms_send_buff, sms_send_to);
				Process_CMD_Response(socket_created, sms_send_to, sms_send_buff);
			}
		}
	}
	else
	{
		memset(&sms_send_buff, 0, sizeof(sms_send_buff));
		sprintf(sms_send_buff, "%s-%s SMSRev: %s", serverCfg.ClientID, num_from,buffer);
		//thanhntMDM.smsSend("+84973518710", sms_send_buff); //Send SMS to developer
	}
}

void Process_CMD_Response(long socket_created, char *num_from, char *buffer)
{
	if(strlen(num_from) > 9)
	{
		thanhntMDM.smsSend(num_from, buffer);
	}
	
	if(socket_created >= 0)
	{
		socket_send(socket_created, buffer, strlen(buffer));
	}
}

// END OF FILE

