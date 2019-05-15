/*
  SimpleLCD.h - Library for interfacing 20x4 LCD character display.
  Created by Raivis Strogonovs, August 6, 2013.
  Released into the public domain.
	
	modified and maintained by Kien Hoang - SANSLAB - April 24, 2019 for STM32L476RGT6
*/

#include "lcd.h"
#include "board-config.h"
 
SimpleLCD::SimpleLCD(lcd_t *lcdObj)
{

		GpioInit( &lcdObj->rs, LCD_RS, PIN_OUTPUT, PIN_PUSH_PULL, PIN_NO_PULL, 0 );
		GpioInit( &lcdObj->en, LCD_EN, PIN_OUTPUT, PIN_PUSH_PULL, PIN_NO_PULL, 0 );
		GpioInit( &lcdObj->d4, LCD_D4, PIN_OUTPUT, PIN_PUSH_PULL, PIN_NO_PULL, 0 );
		GpioInit( &lcdObj->d5, LCD_D5, PIN_OUTPUT, PIN_PUSH_PULL, PIN_NO_PULL, 0 );
		GpioInit( &lcdObj->d6, LCD_D6, PIN_OUTPUT, PIN_PUSH_PULL, PIN_NO_PULL, 0 );
		GpioInit( &lcdObj->d7, LCD_D7, PIN_OUTPUT, PIN_PUSH_PULL, PIN_NO_PULL, 0 );

		_RS = lcdObj->rs; /*_RW = RW;*/ _E = lcdObj->en;
		_D4 = lcdObj->d4; _D5 = lcdObj->d5; _D6 = lcdObj->d6; _D7 = lcdObj->d7;
	
}
 
void SimpleLCD::lcdGoToXY(char x, char y)
{
  char addr;
  switch(x)
  {
     case 0: addr = 0x00; break; //Starting address of 1st line
     case 1: addr = 0x40; break; //Starting address of 2nd line
     case 2: addr = 0x14; break; //Starting address of 3rd line
     case 3: addr = 0x54; break; //Starting address of 4th line
     default: ; 
  }
   
  addr +=y;
   
  lcdGoToAddr(addr);
}
 
void SimpleLCD::lcdGoToAddr(char addr)
{
    char cmd = 0x80 | addr;
		GpioWrite(&_RS, 0);
    sendCommand4Bit(cmd);
}
 
void SimpleLCD::lcdInit()
{
  //Set all the control pins to logic Zero
  GpioWrite(&_RS, 0);
  GpioWrite(&_E, 0);
  //Do the wake up call
  HAL_Delay(20);
  sendCommand(0x30);
  HAL_Delay(20);
  sendCommand(0x30);
  HAL_Delay(20);
  sendCommand(0x30);  
  HAL_Delay(20);
  sendCommand(0x20);  //Let's make it 4 bit mode
  HAL_Delay(10);
  //That's it LCD is initialized in 4 bit mode.
  sendCommand4Bit(0x28); //N = 1 (2 line display) F = 0 (5x8 characters)
  sendCommand4Bit(0x08); //Display on/off control D=0,C=0, B=0 
  sendCommand4Bit(0x01); //Clear Display
  sendCommand4Bit(0x06); //Entry mode set - I/D = 1 (increment cursor) & S = 0 (no shift) 
  sendCommand4Bit(0x0C); //Display on/off control. D = 1, C and B = 0. (Cursor and blink, last two bits)  
}
 
void SimpleLCD::lcdClear()
{
	GpioWrite(&_RS, 0);
	sendCommand4Bit(0x01);
}
 
void SimpleLCD::lcdWriteText(char *text)
{      
	while( *text)
	{
		GpioWrite(&_RS, 1);
		sendCommand4Bit(*text++);
	}
}
 
void SimpleLCD::sendCommand(char opCode)
{  
	GpioWrite(&_D4, opCode & 0x10);
	GpioWrite(&_D5, opCode & 0x20);
	GpioWrite(&_D6, opCode & 0x40);
	GpioWrite(&_D7, opCode & 0x80);
}
 
void SimpleLCD::sendCommand4Bit(char opCode)
{	
	GpioWrite(&_D4, opCode & 0x10);
	GpioWrite(&_D5, opCode & 0x20);
	GpioWrite(&_D6, opCode & 0x40);
	GpioWrite(&_D7, opCode & 0x80);
	GpioWrite(&_E,1);
	GpioWrite(&_E,0);
	GpioWrite(&_D4, opCode & 0x01);
	GpioWrite(&_D5, opCode & 0x02);
	GpioWrite(&_D6, opCode & 0x04);
	GpioWrite(&_D7, opCode & 0x08);
	GpioWrite(&_E,1);
	GpioWrite(&_E,0);
	HAL_Delay(10); // replace for lcd busy
}
