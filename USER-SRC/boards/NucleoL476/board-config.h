/*!
 * \file      board-config.h
 *
 * \brief     Board configuration
 *
 * \copyright Revised BSD License, see section \ref LICENSE.
 *
 * \code
 *                ______                              _
 *               / _____)             _              | |
 *              ( (____  _____ ____ _| |_ _____  ____| |__
 *               \____ \| ___ |    (_   _) ___ |/ ___)  _ \
 *               _____) ) ____| | | || |_| ____( (___| | | |
 *              (______/|_____)_|_|_| \__)_____)\____)_| |_|
 *              (C)2013-2017 Semtech
 *
 *               ___ _____ _   ___ _  _____ ___  ___  ___ ___
 *              / __|_   _/_\ / __| |/ / __/ _ \| _ \/ __| __|
 *              \__ \ | |/ _ \ (__| ' <| _| (_) |   / (__| _|
 *              |___/ |_/_/ \_\___|_|\_\_| \___/|_|_\\___|___|
 *              embedded.connectivity.solutions===============
 *
 * \endcode
 *
 * \author    Miguel Luis ( Semtech )
 *
 * \author    Gregory Cristian ( Semtech )
 *
 * \author    Daniel Jaeckle ( STACKFORCE )
 *
 * \author    Johannes Bruder ( STACKFORCE )
 *
 * \author	  Kien Hoang ( SANSLAB ) 
 */
#ifndef __BOARD_CONFIG_H__
#define __BOARD_CONFIG_H__

#define SX1276MB1MAS 
/*!
 * Defines the time required for the TCXO to wakeup [ms].
 */
#define BOARD_TCXO_WAKEUP_TIME                      0

/*!
 * Board MCU pins definitions
 */
#define RADIO_MOSI                                  PB_15
#define RADIO_MISO                                  PB_14
#define RADIO_SCLK                                  PB_13

#define RADIO_NSS                                   PB_12
#define RADIO_RESET                                 PC_6


#define RADIO_DIO_0                                 PC_9
#define RADIO_DIO_1                                 PA_8
#define RADIO_DIO_2                                 PA_11
#define RADIO_DIO_3                                 PA_12
#define RADIO_DIO_4                                 PB_0
#define RADIO_DIO_5                                 PC_5
#define RADIO_TXEN								    PC_8 
#define RADIO_RXEN								    PC_7

#define RADIO_ANT_SWITCH                            NC

#define LED_1                                       PC_10 //PC_0
#define LED_2                                       PC_1
#define LED_3                                       PC_2  

// Debug pins definition.
#define RADIO_DBG_PIN_TX                            PA_9
#define RADIO_DBG_PIN_RX                            PA_10

#define OSC_LSE_IN                                  PC_14
#define OSC_LSE_OUT                                 PC_15

#define OSC_HSE_IN                                  PH_0
#define OSC_HSE_OUT                                 PH_1

#define SWCLK                                       PA_14
#define SWDAT                                       PA_13

#define UART_TX                                     PA_2
#define UART_RX                                     PA_3

#define GPS_RST_N 									PB_3

#define LCD_RS 										PB_8
#define LCD_EN 										PC_11
#define LCD_D4 										PB_7
#define LCD_D5 										PB_6
#define LCD_D6 										PB_5
#define LCD_D7 										PB_4
	
#define MDM_PWR    									PA_1
#define MDM_RESET  									PA_0
#define MDM_RTS 									PC_13
#define MDM_DTR										PC_13
#define MDM_RI  									PC_13
#define FLASH_CE  									PC_4


#endif //-- END OF FILE // __BOARD_CONFIG_H__

