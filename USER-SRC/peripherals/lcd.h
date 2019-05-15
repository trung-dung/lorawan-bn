/*
  SimpleLCD.h - Library for interfacing 20x4 LCD character display.
  Created by Raivis Strogonovs, August 6, 2013.
  Released into the public domain.
*/
#ifndef SimpleLCD_h
#define SimpleLCD_h
 
#include "common.h"
#include "gpio.h"
 
typedef struct{
	Gpio_t rs;
	Gpio_t en;
	Gpio_t d4;
	Gpio_t d5;
	Gpio_t d6;
	Gpio_t d7;
}lcd_t;	

class SimpleLCD
{
  public:
    SimpleLCD(lcd_t *lcdObj);
    void lcdGoToXY(char x, char y);
    void lcdGoToAddr(char addr);
    void lcdInit();
    void lcdClear();
    void lcdWriteText(char *text);
    void sendCommand(char opCode);
    void sendCommand4Bit(char opCode);
 
  private:
    Gpio_t _RS;
    Gpio_t _RW;
    Gpio_t _E;
    Gpio_t _D4;
    Gpio_t _D5;
    Gpio_t _D6;
    Gpio_t _D7;
};
 
#endif
