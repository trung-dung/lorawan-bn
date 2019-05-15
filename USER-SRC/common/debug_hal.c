#include <stdarg.h>
#include "uart_driver.h"
#include "debug_hal.h"

#define ENABLED_DEBUG
#define DEBUG_MAX_CHAR_PRINT	256

void DEBUG_D(const char* format, ...)
{
	#ifdef ENABLED_DEBUG
	char buffer[DEBUG_MAX_CHAR_PRINT];
	uint32_t len;
	va_list vArgs;		    
	va_start(vArgs, format);
	len = vsprintf((char *)&buffer[0], (char const *)format, vArgs);
	va_end(vArgs);
	if(len >= DEBUG_MAX_CHAR_PRINT) len = DEBUG_MAX_CHAR_PRINT;
	HW_UART1_Send(buffer, len);
	#endif
}
