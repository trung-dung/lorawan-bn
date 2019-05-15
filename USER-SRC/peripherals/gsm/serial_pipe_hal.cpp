#include "uart_driver.h"
#include "serial_pipe_hal.h"
#include "board.h"
#include "fifo.h"
// debug
#include "debug_hal.h"

// define = 0 when dont use module GPRS and = 1 when use 
#define USE_MODULE_GPRS  

#ifdef USE_MODULE_GPRS
	#include "nmea.h"
	#include "mdm_hal.h"	
	extern Fifo_t fifoGpsRx;
#else 
	#include "max7q.h"
#endif

extern Fifo_t fifoDataRx;

SerialPipe::SerialPipe(int rxSize): _pipeRx(rxSize)
{
	
}

SerialPipe::~SerialPipe(void)
{
	
}

// rx channel
int SerialPipe::readable(void)
{
    return _pipeRx.readable();
}

int SerialPipe::getc(void)
{
    if (!_pipeRx.readable())
        return EOF;
    return _pipeRx.getc();
}

int SerialPipe::get(void* buffer, int length, bool blocking)
{
    return _pipeRx.get((char*)buffer, length, blocking);
}

void SerialPipe::rxIRQBuf(char c)
{
	if (_pipeRx.writeable())
        _pipeRx.putc(c);
    else
        /* overflow */;
}

/*******************************************************************************
 * Function Name  : HAL_USART_Handler
 * Description    : This function handles USART global interrupt request.
 * Input          : None.
 * Output         : None.
 * Return         : None.
 *******************************************************************************/

extern "C" void HAL_UART_RxCpltCallback(UART_HandleTypeDef *UartHandle)
{
	__HAL_UART_FLUSH_DRREGISTER(UartHandle); // Clear the buffer to prevent overrun
	if (UartHandle->Instance == UART5)
	{
		#ifdef USE_MODULE_GPRS 
			if(!IsFifoFull(&fifoGpsRx))
			{
				FifoPush(&fifoGpsRx, UART5_RX_DATA);
			}
			HAL_UART_Receive_IT(UartHandle, (uint8_t *)&UART5_RX_DATA, 1);		
		#else
			gnss.rxIRQBuf(UART5_RX_DATA);
			HAL_UART_Receive_IT(UartHandle, (uint8_t *)&UART5_RX_DATA, 1);		
		#endif
	}
	else if(UartHandle->Instance == USART2)
	{
//		#ifdef USE_MODULE_GPRS
//			thanhntMDM.rxIRQBuf(UART2_RX_DATA);
//			HAL_UART_Receive_IT(UartHandle, (uint8_t *)&UART2_RX_DATA, 1);
//		#endif
	}
	else if(UartHandle->Instance == USART3)
	{
		DEBUG_D("%c", UART3_RX_DATA);
		if(!IsFifoFull(&fifoDataRx))
		{
			FifoPush(&fifoDataRx, UART3_RX_DATA);
		}
		HAL_UART_Receive_IT(UartHandle, (uint8_t *)&UART3_RX_DATA, 1);
	}
	else if(UartHandle->Instance == USART1)
	{
		
	}
}
