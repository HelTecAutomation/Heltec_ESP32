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
 */


#ifndef __BOARD_CONFIG_H__
#define __BOARD_CONFIG_H__

/*!
 * Defines the time required for the TCXO to wakeup [ms].
 */

#define BOARD_TCXO_WAKEUP_TIME                      5

#if defined(WIFI_LORA_32_V3)||defined(WIFI_LORA_32_V4)||defined(WIRELESS_TRACKER)||defined(WIRELESS_STICK_V3)||defined(WIRELESS_STICK_LITE_V3)||defined(WIRELESS_PAPER)||defined(CAPSULE_SENSOR_V3)||defined(WIRELESS_SHELL_V3)||defined(Vision_Master_E_213)||defined(Vision_Master_E290)||defined(Vision_Master_T190)

#define RADIO_DIO_1    14
#define RADIO_NSS      8
#define RADIO_RESET    12
#define RADIO_BUSY     13

#define LORA_CLK       9
#define LORA_MISO      11
#define LORA_MOSI      10
#if defined(WIFI_LORA_32_V4)
#define LORA_PA_POWER  45
#define LORA_PA_TX_EN  46
#endif
#elif defined(WIFI_LORA_32_V2)||defined(WIRELESS_STICK)||defined(WIRELESS_STICK_LITE)||defined(WIRELESS_BRIDGE)
#define RADIO_RESET                                14

#define LORA_MOSI                                  27
#define LORA_MISO                                  19
#define LORA_CLK                                   5
#define RADIO_NSS                                  18

#define RADIO_DIO_0                                26
#define RADIO_DIO_1    35   // GPIO35 -- SX127x's IRQ(Interrupt Request) V2


#elif defined(WIFI_LORA_32_SX1262)
#define RADIO_DIO_1    35
#define RADIO_NSS      14
#define RADIO_RESET    5
#define RADIO_BUSY     34

#define LORA_CLK 12
#define LORA_MISO 15
#define LORA_MOSI 13

#elif defined(WIFI_LORA_32)
#define RADIO_RESET                                 14

#define LORA_MOSI                                   27
#define LORA_MISO                                   19
#define LORA_CLK                                    5
#define RADIO_NSS                                   18

#define RADIO_DIO_0                                 26
#define RADIO_DIO_1    33   // GPIO35 -- SX127x's IRQ(Interrupt Request) V2
#elif defined(WIRELESS_MINI_SHELL)
#define RADIO_DIO_1    3
#define RADIO_NSS      8
#define RADIO_RESET    5
#define RADIO_BUSY     4

#define LORA_CLK 10
#define LORA_MISO 6
#define LORA_MOSI 7
#endif


#endif // __BOARD_CONFIG_H__
