/*
 * Copyright (C) 2015-2017 Alibaba Group Holding Limited
 */

/*!
 * \file      LoRaMac.h
 *
 * \brief     LoRa MAC layer implementation
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
 *              (C)2013 Semtech
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
 * \defgroup  LORAMAC LoRa MAC layer implementation
 *            This module specifies the API implementation of the LoRaMAC layer.
 *            This is a placeholder for a detailed description of the LoRaMac
 *            layer and the supported features.
 * \{
 *
 * \example   classA/LoRaMote/main.c
 *            LoRaWAN class A application example for the LoRaMote.
 *
 * \example   classB/LoRaMote/main.c
 *            LoRaWAN class B application example for the LoRaMote.
 *
 * \example   classC/LoRaMote/main.c
 *            LoRaWAN class C application example for the LoRaMote.
 */

#ifndef __LORAMAC_H__
#define __LORAMAC_H__

#include <stdint.h>
#include <stdbool.h>
#include "../driver/timer.h"
#include "../loramac/LoRaMacCrypto.h"
#if defined(RADIO_CHIP_SX127X)
#include "../radio/radio_sx127x.h"
#else
#include "../radio/radio.h"
#endif

#ifdef __cplusplus
extern "C" {
#endif

#ifndef LORAWAN_PREAMBLE_LENGTH
#define LORAWAN_PREAMBLE_LENGTH 8
#endif


//#define CLASS_A_WOTA

#ifdef CLASS_A_WOTA
#define CLASS_A_WOTA_CAD_SYMBOLS 2   //CAD symbols 
extern TimerEvent_t wota_CadTimer;
extern bool wota_CadStarted;
extern bool wota_CadTimerStarted;
extern bool wota_CadEnable;
extern uint32_t wota_cycle_time;
extern uint32_t wota_max_rxtime;
extern int8_t wota_dr;
extern uint32_t wota_freq;
void wotaCadProcess();
void stopWotaCad();
#endif

extern bool IsLoRaMacNetworkJoined;
/*!
 * LoRaMac internal state
 */
extern uint32_t LoRaMacState;

/*!
 * Check the Mac layer state every MAC_STATE_CHECK_TIMEOUT in ms
 */
#define MAC_STATE_CHECK_TIMEOUT                     1000

/*!
 * Maximum number of times the MAC layer tries to get an acknowledge.
 */
#define MAX_ACK_RETRIES                             8

/*!
 * RSSI free threshold [dBm]
 */
#define RSSI_FREE_TH                                ( int8_t )( -90 )

/*!
 * Frame direction definition for up-link communications
 */
#define UP_LINK                                     0

/*!
 * Frame direction definition for down-link communications
 */
#define DOWN_LINK                                   1

/*!
 * Sets the length of the LoRaMAC footer field.
 * Mainly indicates the MIC field length
 */
#define LORAMAC_MFR_LEN                             4

/*!
 * LoRaMac MLME-Confirm queue length
 */
#define LORA_MAC_MLME_CONFIRM_QUEUE_LEN             5

/*!
 * FRMPayload overhead to be used when setting the Radio.SetMaxPayloadLength
 * in RxWindowSetup function.
 * Maximum PHYPayload = MaxPayloadOfDatarate/MaxPayloadOfDatarateRepeater + LORA_MAC_FRMPAYLOAD_OVERHEAD
 */
#define LORA_MAC_FRMPAYLOAD_OVERHEAD                13 // MHDR(1) + FHDR(7) + Port(1) + MIC(4)

/*!
 * Syncword for Private LoRa networks
 */
#define LORA_MAC_PRIVATE_SYNCWORD                   0x12

/*!
 * Syncword for Public LoRa networks
 */
#define LORA_MAC_PUBLIC_SYNCWORD                    0x34

/*!
 * LoRaWAN devices classes definition
 *
 * LoRaWAN Specification V1.0.2, chapter 2.1
 */
typedef enum eDeviceClass {
    /*!
     * LoRaWAN device class A
     *
     * LoRaWAN Specification V1.0.2, chapter 3
     */
    CLASS_A,
    /*!
     * LoRaWAN device class B
     *
     * LoRaWAN Specification V1.0.2, chapter 8
     */
    CLASS_B,
    /*!
     * LoRaWAN device class C
     *
     * LoRaWAN Specification V1.0.2, chapter 17
     */
    CLASS_C,
} DeviceClass_t;

/*!
 * LoRaMAC channels parameters definition
 */
typedef union uDrRange {
    /*!
     * Byte-access to the bits
     */
    int8_t Value;
    /*!
     * Structure to store the minimum and the maximum datarate
     */
    struct sFields {
        /*!
        * Minimum data rate
        *
        * LoRaWAN Regional Parameters V1.0.2rB
        *
        * The allowed ranges are region specific. Please refer to \ref DR_0 to \ref DR_15 for details.
        */
        int8_t Min : 4;
        /*!
         * Maximum data rate
         *
         * LoRaWAN Regional Parameters V1.0.2rB
         *
         * The allowed ranges are region specific. Please refer to \ref DR_0 to \ref DR_15 for details.
         */
        int8_t Max : 4;
    } Fields;
} DrRange_t;

/*!
 * LoRaMAC band parameters definition
 */
typedef struct sBand {
    /*!
     * Duty cycle
     */
    uint16_t DCycle;
    /*!
     * Maximum Tx power
     */
    int8_t TxMaxPower;
    /*!
     * Time stamp of the last JoinReq Tx frame.
     */
    TimerTime_t LastJoinTxDoneTime;
    /*!
     * Time stamp of the last Tx frame
     */
    TimerTime_t LastTxDoneTime;
    /*!
     * Holds the time where the device is off
     */
    TimerTime_t TimeOff;
} Band_t;

/*!
 * LoRaMAC channel definition
 */
typedef struct sChannelParams {
    /*!
     * Frequency in Hz
     */
    uint32_t Frequency;
    /*!
     * Alternative frequency for RX window 1
     */
    uint32_t Rx1Frequency;
    /*!
     * Data rate definition
     */
    DrRange_t DrRange;
    /*!
     * Band index
     */
    uint8_t Band;
} ChannelParams_t;

/*!
 * LoRaMAC receive window 2 channel parameters
 */
typedef struct sRx2ChannelParams {
    /*!
     * Frequency in Hz
     */
    uint32_t Frequency;
    /*!
     * Data rate
     *
     * LoRaWAN Regional Parameters V1.0.2rB
     *
     * The allowed ranges are region specific. Please refer to \ref DR_0 to \ref DR_15 for details.
     */
    uint8_t  Datarate;
} Rx2ChannelParams_t;

/*!
 * LoRaMAC receive window enumeration
 */
typedef enum eLoRaMacRxSlot
{
    /*!
     * LoRaMAC receive window 1
     */
    RX_SLOT_WIN_1,
    /*!
     * LoRaMAC receive window 2
     */
    RX_SLOT_WIN_2,
    /*!
     * LoRaMAC receive window 2 for class c - continuous listening
     */
    RX_SLOT_WIN_CLASS_C,
    /*!
     * LoRaMAC class b ping slot window
     */
    RX_SLOT_WIN_PING_SLOT,
    /*!
     * LoRaMAC class b multicast slot window
     */
    RX_SLOT_WIN_MULTICAST_SLOT,
    
    RX_SLOT_WOTA,
}LoRaMacRxSlot_t;

/*!
 * Global MAC layer parameters
 */
typedef struct sLoRaMacParams {
    /*!
     * Channels TX power
     */
    int8_t ChannelsTxPower;
    /*!
     * Channels data rate
     */
    int8_t ChannelsDatarate;
    /*!
     * System overall timing error in milliseconds.
     * [-SystemMaxRxError : +SystemMaxRxError]
     * Default: +/-10 ms
     */
    uint32_t SystemMaxRxError;
    /*!
     * Minimum required number of symbols to detect an Rx frame
     * Default: 6 symbols
     */
    uint8_t MinRxSymbols;
    /*!
     * LoRaMac maximum time a reception window stays open
     */
    uint32_t MaxRxWindow;
    /*!
     * Receive delay 1
     */
    uint32_t ReceiveDelay1;
    /*!
     * Receive delay 2
     */
    uint32_t ReceiveDelay2;
    /*!
     * Join accept delay 1
     */
    uint32_t JoinAcceptDelay1;
    /*!
     * Join accept delay 1
     */
    uint32_t JoinAcceptDelay2;
    /*!
     * Number of uplink messages repetitions [1:15] (unconfirmed messages only)
     */
    uint8_t ChannelsNbRep;
    /*!
     * Datarate offset between uplink and downlink on first window
     */
    uint8_t Rx1DrOffset;
    /*!
     * LoRaMAC 2nd reception window settings
     */
    Rx2ChannelParams_t Rx2Channel;
    /*!
     * Uplink dwell time configuration. 0: No limit, 1: 400ms
     */
    uint8_t UplinkDwellTime;
    /*!
     * Downlink dwell time configuration. 0: No limit, 1: 400ms
     */
    uint8_t DownlinkDwellTime;
    /*!
     * Maximum possible EIRP
     */
    float MaxEirp;
    /*!
     * Antenna gain of the node
     */
    float AntennaGain;
    /*!
     * Indicates if the node supports repeaters
     */
    bool RepeaterSupport;

#ifdef CONFIG_LINKWAN
    uint8_t method;
    uint8_t freqband;
    bool update_freqband;
#endif
} LoRaMacParams_t;

/*!
 * LoRaMAC multicast channel parameter
 */
typedef struct sMulticastParams {
    /*!
     * Address
     */
    uint32_t Address;
    /*!
     * Network session key
     */
    uint8_t NwkSKey[16];
    /*!
     * Application session key
     */
    uint8_t AppSKey[16];
    /*!
     * Downlink counter
     */
    uint32_t DownLinkCounter;
    /*!
     * Reception frequency of the ping slot windows
     */
    uint32_t Frequency;
    /*!
     * Datarate of the ping slot
     */
    int8_t Datarate;
    /*!
     * This parameter is necessary for class b operation. It defines the
     * periodicity of the multicast downlink slots
     */
    uint16_t Periodicity;
    /*!
     * Number of multicast slots. The variable can be
     * calculated as follows:
     * PingNb = 128 / ( 1 << periodicity ), where
     * 0 <= periodicity <= 7
     * This parameter will be calculated automatically.
     */
    uint8_t PingNb;
    /*!
     * Period of the multicast slots. The variable can be
     * calculated as follows:
     * PingPeriod = 4096 / PingNb
     * This parameter will be calculated automatically.
     */
    uint16_t PingPeriod;
    /*!
     * Ping offset of the multicast channel for Class B
     * This parameter will be calculated automatically.
     */
    uint16_t PingOffset;
    /*!
     * Reference pointer to the next multicast channel parameters in the list
     * This parameter will be calculated automatically.
     */
    struct sMulticastParams *Next;
} MulticastParams_t;

/*!
 * LoRaMAC frame types
 *
 * LoRaWAN Specification V1.0.2, chapter 4.2.1, table 1
 */
typedef enum eLoRaMacFrameType {
    /*!
     * LoRaMAC join request frame
     */
    FRAME_TYPE_JOIN_REQ              = 0x00,
    /*!
     * LoRaMAC join accept frame
     */
    FRAME_TYPE_JOIN_ACCEPT           = 0x01,
    /*!
     * LoRaMAC unconfirmed up-link frame
     */
    FRAME_TYPE_DATA_UNCONFIRMED_UP   = 0x02,
    /*!
     * LoRaMAC unconfirmed down-link frame
     */
    FRAME_TYPE_DATA_UNCONFIRMED_DOWN = 0x03,
    /*!
     * LoRaMAC confirmed up-link frame
     */
    FRAME_TYPE_DATA_CONFIRMED_UP     = 0x04,
    /*!
     * LoRaMAC confirmed down-link frame
     */
    FRAME_TYPE_DATA_CONFIRMED_DOWN   = 0x05,
    /*!
     * LoRaMAC RFU frame
     */
    FRAME_TYPE_RFU                   = 0x06,
    /*!
     * LoRaMAC proprietary frame
     */
    FRAME_TYPE_PROPRIETARY           = 0x07,
} LoRaMacFrameType_t;

/*!
 * LoRaMAC mote MAC commands
 *
 * LoRaWAN Specification V1.0.2, chapter 5, table 4
 */
typedef enum eLoRaMacMoteCmd {
    /*!
     * LinkCheckReq
     */
    MOTE_MAC_LINK_CHECK_REQ          = 0x02,
    /*!
     * LinkADRAns
     */
    MOTE_MAC_LINK_ADR_ANS            = 0x03,
    /*!
     * DutyCycleAns
     */
    MOTE_MAC_DUTY_CYCLE_ANS          = 0x04,
    /*!
     * RXParamSetupAns
     */
    MOTE_MAC_RX_PARAM_SETUP_ANS      = 0x05,
    /*!
     * DevStatusAns
     */
    MOTE_MAC_DEV_STATUS_ANS          = 0x06,
    /*!
     * NewChannelAns
     */
    MOTE_MAC_NEW_CHANNEL_ANS         = 0x07,
    /*!
     * RXTimingSetupAns
     */
    MOTE_MAC_RX_TIMING_SETUP_ANS     = 0x08,
    /*!
     * TXParamSetupAns
     */
    MOTE_MAC_TX_PARAM_SETUP_ANS      = 0x09,
    /*!
     * DlChannelAns
     */
    MOTE_MAC_DL_CHANNEL_ANS          = 0x0A,
    /*!
     * DeviceTimeReq
     */
    MOTE_MAC_DEVICE_TIME_REQ         = 0x0D,
    /*!
     * PingSlotInfoReq
     */
    MOTE_MAC_PING_SLOT_INFO_REQ      = 0x10,
    /*!
     * PingSlotFreqAns
     */
    MOTE_MAC_PING_SLOT_FREQ_ANS      = 0x11,
    /*!
     * BeaconTimingReq
     */
    MOTE_MAC_BEACON_TIMING_REQ       = 0x12,
    /*!
     * BeaconFreqAns
     */
    MOTE_MAC_BEACON_FREQ_ANS         = 0x13,
} LoRaMacMoteCmd_t;

/*!
 * LoRaMAC server MAC commands
 *
 * LoRaWAN Specification V1.0.2 chapter 5, table 4
 */
typedef enum eLoRaMacSrvCmd {
    /*!
     * LinkCheckAns
     */
    SRV_MAC_LINK_CHECK_ANS           = 0x02,
    /*!
     * LinkADRReq
     */
    SRV_MAC_LINK_ADR_REQ             = 0x03,
    /*!
     * DutyCycleReq
     */
    SRV_MAC_DUTY_CYCLE_REQ           = 0x04,
    /*!
     * RXParamSetupReq
     */
    SRV_MAC_RX_PARAM_SETUP_REQ       = 0x05,
    /*!
     * DevStatusReq
     */
    SRV_MAC_DEV_STATUS_REQ           = 0x06,
    /*!
     * NewChannelReq
     */
    SRV_MAC_NEW_CHANNEL_REQ          = 0x07,
    /*!
     * RXTimingSetupReq
     */
    SRV_MAC_RX_TIMING_SETUP_REQ      = 0x08,
    /*!
     * NewChannelReq
     */
    SRV_MAC_TX_PARAM_SETUP_REQ       = 0x09,
    /*!
     * DlChannelReq
     */
    SRV_MAC_DL_CHANNEL_REQ           = 0x0A,
    /*!
     * DeviceTimeAns
     */
    SRV_MAC_DEVICE_TIME_ANS          = 0x0D,
    /*!
     * PingSlotInfoAns
     */
    SRV_MAC_PING_SLOT_INFO_ANS       = 0x10,
    /*!
     * PingSlotChannelReq
     */
    SRV_MAC_PING_SLOT_CHANNEL_REQ    = 0x11,
    /*!
     * BeaconTimingAns
     */
    SRV_MAC_BEACON_TIMING_ANS        = 0x12,
    /*!
     * BeaconFreqReq
     */
    SRV_MAC_BEACON_FREQ_REQ          = 0x13,
} LoRaMacSrvCmd_t;

/*!
 * LoRaMAC Battery level indicator
 */
typedef enum eLoRaMacBatteryLevel {
    /*!
     * External power source
     */
    BAT_LEVEL_EXT_SRC                = 0x00,
    /*!
     * Battery level empty
     */
    BAT_LEVEL_EMPTY                  = 0x01,
    /*!
     * Battery level full
     */
    BAT_LEVEL_FULL                   = 0xFE,
    /*!
     * Battery level - no measurement available
     */
    BAT_LEVEL_NO_MEASURE             = 0xFF,
} LoRaMacBatteryLevel_t;

/*!
 * LoRaMAC header field definition (MHDR field)
 *
 * LoRaWAN Specification V1.0.2, chapter 4.2
 */
typedef union uLoRaMacHeader {
    /*!
     * Byte-access to the bits
     */
    uint8_t Value;
    /*!
     * Structure containing single access to header bits
     */
    struct sHdrBits {
        /*!
         * Major version
         */
        uint8_t Major           : 2;
        /*!
         * RFU
         */
        uint8_t RFU             : 3;
        /*!
         * Message type
         */
        uint8_t MType           : 3;
    } Bits;
} LoRaMacHeader_t;

/*!
 * LoRaMAC frame control field definition (FCtrl)
 *
 * LoRaWAN Specification V1.0.2, chapter 4.3.1
 */
typedef union uLoRaMacFrameCtrl {
    /*!
     * Byte-access to the bits
     */
    uint8_t Value;
    /*!
     * Structure containing single access to bits
     */
    struct sCtrlBits {
        /*!
         * Frame options length
         */
        uint8_t FOptsLen        : 4;
        /*!
         * Frame pending bit
         */
        uint8_t FPending        : 1;
        /*!
         * Message acknowledge bit
         */
        uint8_t Ack             : 1;
        /*!
         * ADR acknowledgment request bit
         */
        uint8_t AdrAckReq       : 1;
        /*!
         * ADR control in frame header
         */
        uint8_t Adr             : 1;
    } Bits;
} LoRaMacFrameCtrl_t;

/*!
 * LoRaMAC data structure for a PingSlotInfoReq \ref MLME_PING_SLOT_INFO
 *
 * LoRaWAN Specification
 */
typedef union uPingSlotInfo
{
    /*!
     * Parameter for byte access
     */
    uint8_t Value;
    /*!
     * Structure containing the parameters for the PingSlotInfoReq
     */
    struct sInfoFields
    {
        /*!
         * Periodicity = 0: ping slot every second
         * Periodicity = 7: ping slot every 128 seconds
         */
        uint8_t Periodicity     : 3;
        /*!
         * RFU
         */
        uint8_t RFU             : 5;
    }Fields;
}PingSlotInfo_t;

/*!
 * LoRaMAC data structure for the \ref MLME_BEACON MLME-Indication
 *
 * LoRaWAN Specification
 */
typedef struct sBeaconInfo
{
    /*!
     * Timestamp in seconds since 00:00:00, Sunday 6th of January 1980
     * (start of the GPS epoch) modulo 2^32
     */
    uint32_t Time;
    /*!
     * Frequency
     */
    uint32_t Frequency;
    /*!
     * Datarate
     */
    uint8_t Datarate;
    /*!
     * RSSI
     */
    int16_t Rssi;
    /*!
     * SNR
     */
    uint8_t Snr;
    /*!
     * Data structure for the gateway specific part. The
     * content of the values may differ for each gateway
     */
    struct sGwSpecific
    {
        /*!
         * Info descriptor - can differ for each gateway
         */
        uint8_t InfoDesc;
        /*!
         * Info - can differ for each gateway
         */
        uint8_t Info[6];
    }GwSpecific;
}BeaconInfo_t;

/*!
 * Enumeration containing the status of the operation of a MAC service
 */
typedef enum eLoRaMacEventInfoStatus {
    /*!
     * Service performed successfully
     */
    LORAMAC_EVENT_INFO_STATUS_OK = 0,
    /*!
     * An error occurred during the execution of the service
     */
    LORAMAC_EVENT_INFO_STATUS_ERROR,
    /*!
     * A Tx timeout occurred
     */
    LORAMAC_EVENT_INFO_STATUS_TX_TIMEOUT,
    /*!
         * An Rx timeout occurred on receive window 1
         */
    LORAMAC_EVENT_INFO_STATUS_RX1_TIMEOUT,
    /*!
     * An Rx timeout occurred on receive window 2
     */
    LORAMAC_EVENT_INFO_STATUS_RX2_TIMEOUT,
    /*!
     * An Rx error occurred on receive window 1
     */
    LORAMAC_EVENT_INFO_STATUS_RX1_ERROR,
    /*!
     * An Rx error occurred on receive window 2
     */
    LORAMAC_EVENT_INFO_STATUS_RX2_ERROR,
    /*!
     * An error occurred in the join procedure
     */
    LORAMAC_EVENT_INFO_STATUS_JOIN_FAIL,
    /*!
     * A frame with an invalid downlink counter was received. The
     * downlink counter of the frame was equal to the local copy
     * of the downlink counter of the node.
     */
    LORAMAC_EVENT_INFO_STATUS_DOWNLINK_REPEATED,
    /*!
     * The MAC could not retransmit a frame since the MAC decreased the datarate. The
     * payload size is not applicable for the datarate.
     */
    LORAMAC_EVENT_INFO_STATUS_TX_DR_PAYLOAD_SIZE_ERROR,
    /*!
     * The node has lost MAX_FCNT_GAP or more frames.
     */
    LORAMAC_EVENT_INFO_STATUS_DOWNLINK_TOO_MANY_FRAMES_LOSS,
    /*!
     * An address error occurred
     */
    LORAMAC_EVENT_INFO_STATUS_ADDRESS_FAIL,
    /*!
     * message integrity check failure
     */
    LORAMAC_EVENT_INFO_STATUS_MIC_FAIL,
    /*!
     * ToDo
     */
    LORAMAC_EVENT_INFO_STATUS_MULTICAST_FAIL,
    /*!
     * ToDo
     */
    LORAMAC_EVENT_INFO_STATUS_BEACON_LOCKED,
    /*!
     * ToDo
     */
    LORAMAC_EVENT_INFO_STATUS_BEACON_LOST,
    /*!
     * ToDo
     */
    LORAMAC_EVENT_INFO_STATUS_BEACON_NOT_FOUND,
} LoRaMacEventInfoStatus_t;

/*!
 * LoRaMac tx/rx operation state
 */
typedef union eLoRaMacFlags_t {
    /*!
     * Byte-access to the bits
     */
    uint8_t Value;
    /*!
     * Structure containing single access to bits
     */
    struct sMacFlagBits {
        /*!
         * MCPS-Req pending
         */
        uint8_t McpsReq         : 1;
        /*!
         * MCPS-Ind pending
         */
        uint8_t McpsInd         : 1;
        /*!
         * MCPS-Ind pending. Skip indication to the application layer
         */
        uint8_t McpsIndSkip     : 1;
        /*!
         * MLME-Req pending
         */
        uint8_t MlmeReq         : 1;
        /*!
         * MLME-Ind pending
         */
        uint8_t MlmeInd         : 1;
        /*!
         * MAC cycle done
         */
        uint8_t MacDone         : 1;
    } Bits;
} LoRaMacFlags_t;

/*!
 *
 * \brief   LoRaMAC data services
 *
 * \details The following table list the primitives which are supported by the
 *          specific MAC data service:
 *
 * Name                  | Request | Indication | Response | Confirm
 * --------------------- | :-----: | :--------: | :------: | :-----:
 * \ref MCPS_UNCONFIRMED | YES     | YES        | NO       | YES
 * \ref MCPS_CONFIRMED   | YES     | YES        | NO       | YES
 * \ref MCPS_MULTICAST   | NO      | YES        | NO       | NO
 * \ref MCPS_PROPRIETARY | YES     | YES        | NO       | YES
 *
 * The following table provides links to the function implementations of the
 * related MCPS primitives:
 *
 * Primitive        | Function
 * ---------------- | :---------------------:
 * MCPS-Request     | \ref LoRaMacMlmeRequest
 * MCPS-Confirm     | MacMcpsConfirm in \ref LoRaMacPrimitives_t
 * MCPS-Indication  | MacMcpsIndication in \ref LoRaMacPrimitives_t
 */
typedef enum eMcps {
    /*!
     * Unconfirmed LoRaMAC frame
     */
    MCPS_UNCONFIRMED,
    /*!
     * Confirmed LoRaMAC frame
     */
    MCPS_CONFIRMED,
    /*!
     * Multicast LoRaMAC frame
     */
    MCPS_MULTICAST,
    /*!
     * Proprietary frame
     */
    MCPS_PROPRIETARY,
} Mcps_t;

/*!
 * LoRaMAC MCPS-Request for an unconfirmed frame
 */
typedef struct sMcpsReqUnconfirmed {
    /*!
     * Frame port field. Must be set if the payload is not empty. Use the
     * application specific frame port values: [1...223]
     *
     * LoRaWAN Specification V1.0.2, chapter 4.3.2
     */
    uint8_t fPort;
    /*!
     * Pointer to the buffer of the frame payload
     */
    void *fBuffer;
    /*!
     * Size of the frame payload
     */
    uint16_t fBufferSize;
    /*!
     * Uplink datarate, if ADR is off
     */
    int8_t Datarate;
} McpsReqUnconfirmed_t;

/*!
 * LoRaMAC MCPS-Request for a confirmed frame
 */
typedef struct sMcpsReqConfirmed {
    /*!
     * Frame port field. Must be set if the payload is not empty. Use the
     * application specific frame port values: [1...223]
     *
     * LoRaWAN Specification V1.0.2, chapter 4.3.2
     */
    uint8_t fPort;
    /*!
     * Pointer to the buffer of the frame payload
     */
    void *fBuffer;
    /*!
     * Size of the frame payload
     */
    uint16_t fBufferSize;
    /*!
     * Uplink datarate, if ADR is off
     */
    int8_t Datarate;
    /*!
     * Number of trials to transmit the frame, if the LoRaMAC layer did not
     * receive an acknowledgment. The MAC performs a datarate adaptation,
     * according to the LoRaWAN Specification V1.0.2, chapter 18.4, according
     * to the following table:
     *
     * Transmission nb | Data Rate
     * ----------------|-----------
     * 1 (first)       | DR
     * 2               | DR
     * 3               | max(DR-1,0)
     * 4               | max(DR-1,0)
     * 5               | max(DR-2,0)
     * 6               | max(DR-2,0)
     * 7               | max(DR-3,0)
     * 8               | max(DR-3,0)
     *
     * Note, that if NbTrials is set to 1 or 2, the MAC will not decrease
     * the datarate, in case the LoRaMAC layer did not receive an acknowledgment
     */
    uint8_t NbTrials;
} McpsReqConfirmed_t;

/*!
 * LoRaMAC MCPS-Request for a proprietary frame
 */
typedef struct sMcpsReqProprietary {
    /*!
     * Pointer to the buffer of the frame payload
     */
    void *fBuffer;
    /*!
     * Size of the frame payload
     */
    uint16_t fBufferSize;
    /*!
     * Uplink datarate, if ADR is off
     */
    int8_t Datarate;
} McpsReqProprietary_t;

/*!
 * LoRaMAC MCPS-Request structure
 */
typedef struct sMcpsReq {
    /*!
     * MCPS-Request type
     */
    Mcps_t Type;

    /*!
     * MCPS-Request parameters
     */
    union uMcpsParam {
        /*!
         * MCPS-Request parameters for an unconfirmed frame
         */
        McpsReqUnconfirmed_t Unconfirmed;
        /*!
         * MCPS-Request parameters for a confirmed frame
         */
        McpsReqConfirmed_t Confirmed;
        /*!
         * MCPS-Request parameters for a proprietary frame
         */
        McpsReqProprietary_t Proprietary;
    } Req;
} McpsReq_t;

/*!
 * LoRaMAC MCPS-Confirm
 */
typedef struct sMcpsConfirm {
    /*!
     * Holds the previously performed MCPS-Request
     */
    Mcps_t McpsRequest;
    /*!
     * Status of the operation
     */
    LoRaMacEventInfoStatus_t Status;
    /*!
     * Uplink datarate
     */
    uint8_t Datarate;
    /*!
     * Transmission power
     */
    int8_t TxPower;
    /*!
     * Set if an acknowledgement was received
     */
    bool AckReceived;
    /*!
     * Provides the number of retransmissions
     */
    uint8_t NbRetries;
    /*!
     * The transmission time on air of the frame
     */
    TimerTime_t TxTimeOnAir;
    /*!
     * The uplink counter value related to the frame
     */
    uint32_t UpLinkCounter;
    /*!
     * The uplink channel related to the frame
     */
    uint32_t Channel;
} McpsConfirm_t;

/*!
 * LoRaMAC MCPS-Indication primitive
 */
typedef struct sMcpsIndication {
    /*!
     * MCPS-Indication type
     */
    Mcps_t McpsIndication;
    /*!
     * Status of the operation
     */
    LoRaMacEventInfoStatus_t Status;
    /*!
     * Multicast
     */
    uint8_t Multicast;
    /*!
     * Application port
     */
    uint8_t Port;
    /*!
     * Downlink datarate
     */
    uint8_t RxDatarate;
    /*!
     * RxDone datarate
     */
    uint8_t RxDoneDatarate;
    /*!
     * Frame pending status
     */
    uint8_t FramePending;
    /*!
     * Pointer to the received data stream
     */
    uint8_t *Buffer;
    /*!
     * Size of the received data stream
     */
    uint8_t BufferSize;
    /*!
     * Indicates, if data is available
     */
    bool RxData;
    /*!
     * Rssi of the received packet
     */
    int16_t Rssi;
    /*!
     * Snr of the received packet
     */
    int8_t Snr;
    /*!
     * Receive window
     */
    LoRaMacRxSlot_t RxSlot;
    /*!
     * Set if an acknowledgement was received
     */
    bool AckReceived;
    /*!
     * The downlink counter value for the received frame
     */
    uint32_t DownLinkCounter;
#ifdef CONFIG_LWAN
    bool DevTimeAnsReceived;
    bool LinkCheckAnsReceived;
    bool UplinkNeeded;
#endif    
} McpsIndication_t;

/*!
 * \brief LoRaMAC management services
 *
 * \details The following table list the primitives which are supported by the
 *          specific MAC management service:
 *
 * Name                  | Request | Indication | Response | Confirm
 * --------------------- | :-----: | :--------: | :------: | :-----:
 * \ref MLME_JOIN        | YES     | NO         | NO       | YES
 * \ref MLME_LINK_CHECK  | YES     | NO         | NO       | YES
 * \ref MLME_TXCW        | YES     | NO         | NO       | YES
 * \ref MLME_SCHEDULE_UPLINK    | NO      | YES        | NO       | NO
 *
 * The following table provides links to the function implementations of the
 * related MLME primitives.
 *
 * Primitive        | Function
 * ---------------- | :---------------------:
 * MLME-Request     | \ref LoRaMacMlmeRequest
 * MLME-Confirm     | MacMlmeConfirm in \ref LoRaMacPrimitives_t
 * MLME-Indication  | MacMlmeIndication in \ref LoRaMacPrimitives_t
 */
typedef enum eMlme {
    /*!
     * Initiates the Over-the-Air activation
     *
     * LoRaWAN Specification V1.0.2, chapter 6.2
     */
    MLME_JOIN,
    /*!
     * LinkCheckReq - Connectivity validation
     *
     * LoRaWAN Specification V1.0.2, chapter 5, table 4
     */
    MLME_LINK_CHECK,
    /*!
     * Sets Tx continuous wave mode
     *
     * LoRaWAN end-device certification
     */
    MLME_TXCW,
    /*!
     * Sets Tx continuous wave mode (new LoRa-Alliance CC definition)
     *
     * LoRaWAN end-device certification
     */
    MLME_TXCW_1,
    /*!
     * Indicates that the application shall perform an uplink as
     * soon as possible.
     */
    MLME_SCHEDULE_UPLINK,
    /*!
     * Initiates a DeviceTimeReq
     *
     * LoRaWAN end-device certification
     */
    MLME_DEVICE_TIME,
    /*!
     * The MAC uses this MLME primitive to indicate a beacon reception
     * status.
     *
     * LoRaWAN end-device certification
     */
    MLME_BEACON,
    /*!
     * Initiate a beacon acquisition. The MAC will search for a beacon.
     * It will search for XX_BEACON_INTERVAL milliseconds.
     *
     * LoRaWAN end-device certification
     */
    MLME_BEACON_ACQUISITION,
    /*!
     * Initiates a PingSlotInfoReq
     *
     * LoRaWAN end-device certification
     */
    MLME_PING_SLOT_INFO,
    /*!
     * Initiates a BeaconTimingReq
     *
     * LoRaWAN end-device certification
     */
    MLME_BEACON_TIMING,
    /*!
     * Primitive which indicates that the beacon has been lost
     *
     * \remark The upper layer is required to switch the device class to ClassA
     *
     * LoRaWAN end-device certification
     */
    MLME_BEACON_LOST,
} Mlme_t;

/*!
 * LoRaMAC MLME-Request for the join service
 */
typedef struct sMlmeReqJoin {
    /*!
     * Globally unique end-device identifier
     *
     * LoRaWAN Specification V1.0.2, chapter 6.2.1
     */
    uint8_t *DevEui;
    /*!
     * Application identifier
     *
     * LoRaWAN Specification V1.0.2, chapter 6.1.2
     */
    uint8_t *AppEui;
    /*!
     * AES-128 application key
     *
     * LoRaWAN Specification V1.0.2, chapter 6.2.2
     */
    uint8_t *AppKey;
    /*!
     * Number of trials for the join request.
     */
    uint8_t NbTrials;

#ifdef CONFIG_LINKWAN
    int8_t datarate;
    uint8_t method;
    uint8_t freqband;
#endif
} MlmeReqJoin_t;

/*!
 * LoRaMAC MLME-Request for the ping slot info service
 */
typedef struct sMlmeReqPingSlotInfo
{
    PingSlotInfo_t PingSlot;
}MlmeReqPingSlotInfo_t;

/*!
 * LoRaMAC MLME-Request for Tx continuous wave mode
 */
typedef struct sMlmeReqTxCw {
    /*!
     * Time in seconds while the radio is kept in continuous wave mode
     */
    uint16_t Timeout;
    /*!
     * RF frequency to set (Only used with new way)
     */
    uint32_t Frequency;
    /*!
     * RF output power to set (Only used with new way)
     */
    uint8_t Power;
} MlmeReqTxCw_t;

/*!
 * LoRaMAC MLME-Request structure
 */
typedef struct sMlmeReq {
    /*!
     * MLME-Request type
     */
    Mlme_t Type;

    /*!
     * MLME-Request parameters
     */
    union uMlmeParam {
        /*!
         * MLME-Request parameters for a join request
         */
        MlmeReqJoin_t Join;
        /*!
         * MLME-Request parameters for a ping slot info request
         */
        MlmeReqPingSlotInfo_t PingSlotInfo;
        /*!
         * MLME-Request parameters for Tx continuous mode request
         */
        MlmeReqTxCw_t TxCw;
    } Req;
} MlmeReq_t;

/*!
 * LoRaMAC MLME-Confirm primitive
 */
typedef struct sMlmeConfirm {
    /*!
     * Holds the previously performed MLME-Request
     */
    Mlme_t MlmeRequest;
    /*!
     * Status of the operation
     */
    LoRaMacEventInfoStatus_t Status;
#ifdef CONFIG_LWAN
    /*!
     * Rssi of the received packet
     */
    int16_t Rssi;
    /*!
     * Snr of the received packet
     */
    int8_t Snr;
#endif
    /*!
     * The transmission time on air of the frame
     */
    TimerTime_t TxTimeOnAir;
    /*!
     * Demodulation margin. Contains the link margin [dB] of the last
     * successfully received LinkCheckReq
     */
    uint8_t DemodMargin;
    /*!
     * Number of gateways which received the last LinkCheckReq
     */
    uint8_t NbGateways;
    /*!
     * Provides the number of retransmissions
     */
    uint8_t NbRetries;
    /*!
     * The delay which we have received through the
     * BeaconTimingAns
     */
    TimerTime_t BeaconTimingDelay;
    /*!
     * The channel of the next beacon
     */
    uint8_t BeaconTimingChannel;
} MlmeConfirm_t;

/*!
 * LoRaMAC MLME-Indication primitive
 */
typedef struct sMlmeIndication
{
    /*!
     * Holds the previously performed MLME-Request
     */
    Mlme_t MlmeIndication;
    /*!
     * Status of the operation
     */
    LoRaMacEventInfoStatus_t Status;
    /*!
     * Beacon information. Only valid for \ref MLME_BEACON,
     * status \ref LORAMAC_EVENT_INFO_STATUS_BEACON_LOCKED
     */
    BeaconInfo_t BeaconInfo;
}MlmeIndication_t;
/*!
 * LoRa Mac Information Base (MIB)
 *
 * The following table lists the MIB parameters and the related attributes:
 *
 * Attribute                         | Get | Set
 * --------------------------------- | :-: | :-:
 * \ref MIB_DEVICE_CLASS             | YES | YES
 * \ref MIB_NETWORK_JOINED           | YES | YES
 * \ref MIB_ADR                      | YES | YES
 * \ref MIB_NET_ID                   | YES | YES
 * \ref MIB_DEV_ADDR                 | YES | YES
 * \ref MIB_NWK_SKEY                 | YES | YES
 * \ref MIB_APP_SKEY                 | YES | YES
 * \ref MIB_PUBLIC_NETWORK           | YES | YES
 * \ref MIB_REPEATER_SUPPORT         | YES | YES
 * \ref MIB_CHANNELS                 | YES | NO
 * \ref MIB_RX2_CHANNEL              | YES | YES
 * \ref MIB_CHANNELS_MASK            | YES | YES
 * \ref MIB_CHANNELS_DEFAULT_MASK    | YES | YES
 * \ref MIB_CHANNELS_NB_REP          | YES | YES
 * \ref MIB_MAX_RX_WINDOW_DURATION   | YES | YES
 * \ref MIB_RECEIVE_DELAY_1          | YES | YES
 * \ref MIB_RECEIVE_DELAY_2          | YES | YES
 * \ref MIB_JOIN_ACCEPT_DELAY_1      | YES | YES
 * \ref MIB_JOIN_ACCEPT_DELAY_2      | YES | YES
 * \ref MIB_CHANNELS_DATARATE        | YES | YES
 * \ref MIB_CHANNELS_DEFAULT_DATARATE| YES | YES
 * \ref MIB_CHANNELS_TX_POWER        | YES | YES
 * \ref MIB_CHANNELS_DEFAULT_TX_POWER| YES | YES
 * \ref MIB_UPLINK_COUNTER           | YES | YES
 * \ref MIB_DOWNLINK_COUNTER         | YES | YES
 * \ref MIB_MULTICAST_CHANNEL        | YES | NO
 * \ref MIB_SYSTEM_MAX_RX_ERROR      | YES | YES
 * \ref MIB_MIN_RX_SYMBOLS           | YES | YES
 * \ref MIB_BEACON_INTERVAL                      | YES | YES
 * \ref MIB_BEACON_RESERVED                      | YES | YES
 * \ref MIB_BEACON_GUARD                         | YES | YES
 * \ref MIB_BEACON_WINDOW                        | YES | YES
 * \ref MIB_BEACON_WINDOW_SLOTS                  | YES | YES
 * \ref MIB_PING_SLOT_WINDOW                     | YES | YES
 * \ref MIB_BEACON_SYMBOL_TO_DEFAULT             | YES | YES
 * \ref MIB_BEACON_SYMBOL_TO_EXPANSION_MAX       | YES | YES
 * \ref MIB_PING_SLOT_SYMBOL_TO_EXPANSION_MAX    | YES | YES
 * \ref MIB_BEACON_SYMBOL_TO_EXPANSION_FACTOR    | YES | YES
 * \ref MIB_PING_SLOT_SYMBOL_TO_EXPANSION_FACTOR | YES | YES
 * \ref MIB_MAX_BEACON_LESS_PERIOD               | YES | YES
 * \ref MIB_ANTENNA_GAIN                         | YES | YES
 * \ref MIB_DEFAULT_ANTENNA_GAIN                 | YES | YES
 * \ref MIB_FREQ_BAND                | YES | NO
 *
 * The following table provides links to the function implementations of the
 * related MIB primitives:
 *
 * Primitive        | Function
 * ---------------- | :---------------------:
 * MIB-Set          | \ref LoRaMacMibSetRequestConfirm
 * MIB-Get          | \ref LoRaMacMibGetRequestConfirm
 */
typedef enum eMib {
    /*!
     * LoRaWAN device class
     *
     * LoRaWAN Specification V1.0.2
     */
    MIB_DEVICE_CLASS,
    /*!
     * LoRaWAN Network joined attribute
     *
     * LoRaWAN Specification V1.0.2
     */
    MIB_NETWORK_JOINED,
    /*!
     * Adaptive data rate
     *
     * LoRaWAN Specification V1.0.2, chapter 4.3.1.1
     *
     * [true: ADR enabled, false: ADR disabled]
     */
    MIB_ADR,
    /*!
     * Network identifier
     *
     * LoRaWAN Specification V1.0.2, chapter 6.1.1
     */
    MIB_NET_ID,
    /*!
     * End-device address
     *
     * LoRaWAN Specification V1.0.2, chapter 6.1.1
     */
    MIB_DEV_ADDR,
    /*!
     * Network session key
     *
     * LoRaWAN Specification V1.0.2, chapter 6.1.3
     */
    MIB_NWK_SKEY,
    /*!
     * Application session key
     *
     * LoRaWAN Specification V1.0.2, chapter 6.1.4
     */
    MIB_APP_SKEY,
    /*!
     * Set the network type to public or private
     *
     * LoRaWAN Regional Parameters V1.0.2rB
     *
     * [true: public network, false: private network]
     */
    MIB_PUBLIC_NETWORK,
    /*!
     * Support the operation with repeaters
     *
     * LoRaWAN Regional Parameters V1.0.2rB
     *
     * [true: repeater support enabled, false: repeater support disabled]
     */
    MIB_REPEATER_SUPPORT,
    /*!
     * Communication channels. A get request will return a
     * pointer which references the first entry of the channel list. The
     * list is of size LORA_MAX_NB_CHANNELS
     *
     * LoRaWAN Regional Parameters V1.0.2rB
     */
    MIB_CHANNELS,
    /*!
     * Set receive window 2 channel
     *
     * LoRaWAN Specification V1.0.2, chapter 3.3.1
     */
    MIB_RX2_CHANNEL,
    /*!
     * Set receive window 2 channel
     *
     * LoRaWAN Specification V1.0.2, chapter 3.3.2
     */
    MIB_RX2_DEFAULT_CHANNEL,
    /*!
     * LoRaWAN channels mask
     *
     * LoRaWAN Regional Parameters V1.0.2rB
     */
    MIB_CHANNELS_MASK,
    /*!
     * LoRaWAN default channels mask
     *
     * LoRaWAN Regional Parameters V1.0.2rB
     */
    MIB_CHANNELS_DEFAULT_MASK,
    /*!
     * Set the number of repetitions on a channel
     *
     * LoRaWAN Specification V1.0.2, chapter 5.2
     */
    MIB_CHANNELS_NB_REP,
    /*!
     * Maximum receive window duration in [ms]
     *
     * LoRaWAN Specification V1.0.2, chapter 3.3.3
     */
    MIB_MAX_RX_WINDOW_DURATION,
    /*!
     * Receive delay 1 in [ms]
     *
     * LoRaWAN Regional Parameters V1.0.2rB
     */
    MIB_RECEIVE_DELAY_1,
    /*!
     * Receive delay 2 in [ms]
     *
     * LoRaWAN Regional Parameters V1.0.2rB
     */
    MIB_RECEIVE_DELAY_2,
    /*!
     * Join accept delay 1 in [ms]
     *
     * LoRaWAN Regional Parameters V1.0.2rB
     */
    MIB_JOIN_ACCEPT_DELAY_1,
    /*!
     * Join accept delay 2 in [ms]
     *
     * LoRaWAN Regional Parameters V1.0.2rB
     */
    MIB_JOIN_ACCEPT_DELAY_2,
    /*!
     * Default Data rate of a channel
     *
     * LoRaWAN Regional Parameters V1.0.2rB
     *
     * The allowed ranges are region specific. Please refer to \ref DR_0 to \ref DR_15 for details.
     */
    MIB_CHANNELS_DEFAULT_DATARATE,
    /*!
     * Data rate of a channel
     *
     * LoRaWAN Regional Parameters V1.0.2rB
     *
     * The allowed ranges are region specific. Please refer to \ref DR_0 to \ref DR_15 for details.
     */
    MIB_CHANNELS_DATARATE,
    /*!
     * Transmission power of a channel
     *
     * LoRaWAN Regional Parameters V1.0.2rB
     *
     * The allowed ranges are region specific. Please refer to \ref TX_POWER_0 to \ref TX_POWER_15 for details.
     */
    MIB_CHANNELS_TX_POWER,
    /*!
     * Transmission power of a channel
     *
     * LoRaWAN Regional Parameters V1.0.2rB
     *
     * The allowed ranges are region specific. Please refer to \ref TX_POWER_0 to \ref TX_POWER_15 for details.
     */
    MIB_CHANNELS_DEFAULT_TX_POWER,
    /*!
     * LoRaWAN Up-link counter
     *
     * LoRaWAN Specification V1.0.2, chapter 4.3.1.5
     */
    MIB_UPLINK_COUNTER,
    /*!
     * LoRaWAN Down-link counter
     *
     * LoRaWAN Specification V1.0.2, chapter 4.3.1.5
     */
    MIB_DOWNLINK_COUNTER,
    /*!
     * Multicast channels. A get request will return a pointer to the first
     * entry of the multicast channel linked list. If the pointer is equal to
     * NULL, the list is empty.
     */
    MIB_MULTICAST_CHANNEL,
    MIB_MULTICAST_CHANNEL_DEL,
    /*!
     * System overall timing error in milliseconds.
     * [-SystemMaxRxError : +SystemMaxRxError]
     * Default: +/-10 ms
     */
    MIB_SYSTEM_MAX_RX_ERROR,
    /*!
     * Minimum required number of symbols to detect an Rx frame
     * Default: 6 symbols
     */
    MIB_MIN_RX_SYMBOLS,
    /*!
     * Antenna gain of the node. Default value is region specific.
     * The antenna gain is used to calculate the TX power of the node.
     * The formula is:
     * radioTxPower = ( int8_t )floor( maxEirp - antennaGain )
     */
    MIB_ANTENNA_GAIN,
    /*!
     * Default antenna gain of the node. Default value is region specific.
     * The antenna gain is used to calculate the TX power of the node.
     * The formula is:
     * radioTxPower = ( int8_t )floor( maxEirp - antennaGain )
     */
    MIB_DEFAULT_ANTENNA_GAIN,
    /*!
     * Beacon interval in ms
     */
    MIB_BEACON_INTERVAL,
    /*!
     * Beacon reserved time in ms
     */
    MIB_BEACON_RESERVED,
    /*!
     * Beacon guard time in ms
     */
    MIB_BEACON_GUARD,
    /*!
     * Beacon window time in ms
     */
    MIB_BEACON_WINDOW,
    /*!
     * Beacon window time in number of slots
     */
    MIB_BEACON_WINDOW_SLOTS,
    /*!
     * Ping slot length time in ms
     */
    MIB_PING_SLOT_WINDOW,
    /*!
     * Default symbol timeout for beacons and ping slot windows
     */
    MIB_BEACON_SYMBOL_TO_DEFAULT,
    /*!
     * Maximum symbol timeout for beacons
     */
    MIB_BEACON_SYMBOL_TO_EXPANSION_MAX,
    /*!
     * Maximum symbol timeout for ping slots
     */
    MIB_PING_SLOT_SYMBOL_TO_EXPANSION_MAX,
    /*!
     * Symbol expansion value for beacon windows in case of beacon
     * loss in symbols
     */
    MIB_BEACON_SYMBOL_TO_EXPANSION_FACTOR,
    /*!
     * Symbol expansion value for ping slot windows in case of beacon
     * loss in symbols
     */
    MIB_PING_SLOT_SYMBOL_TO_EXPANSION_FACTOR,
    /*!
     * Maximum allowed beacon less time in ms
     */
    MIB_MAX_BEACON_LESS_PERIOD,
    /*!
     * Ping slot data rate
     *
     * LoRaWAN Regional Parameters V1.0.2rB
     *
     * The allowed ranges are region specific. Please refer to \ref DR_0 to \ref DR_15 for details.
     */
    MIB_PING_SLOT_DATARATE,
    
#ifdef CONFIG_LWAN
    MIB_RX1_DATARATE_OFFSET,
    MIB_MAC_STATE,
#endif  

#ifdef CONFIG_LINKWAN
    MIB_FREQ_BAND
#endif
} Mib_t;

/*!
 * LoRaMAC MIB parameters
 */
typedef union uMibParam {
    /*!
     * LoRaWAN device class
     *
     * Related MIB type: \ref MIB_DEVICE_CLASS
     */
    DeviceClass_t Class;
    /*!
     * LoRaWAN network joined attribute
     *
     * Related MIB type: \ref MIB_NETWORK_JOINED
     */
    bool IsNetworkJoined;
    /*!
     * Activation state of ADR
     *
     * Related MIB type: \ref MIB_ADR
     */
    bool AdrEnable;
    /*!
     * Network identifier
     *
     * Related MIB type: \ref MIB_NET_ID
     */
    uint32_t NetID;
    /*!
     * End-device address
     *
     * Related MIB type: \ref MIB_DEV_ADDR
     */
    uint32_t DevAddr;
    /*!
     * Network session key
     *
     * Related MIB type: \ref MIB_NWK_SKEY
     */
    uint8_t *NwkSKey;
    /*!
     * Application session key
     *
     * Related MIB type: \ref MIB_APP_SKEY
     */
    uint8_t *AppSKey;
    /*!
     * Enable or disable a public network
     *
     * Related MIB type: \ref MIB_PUBLIC_NETWORK
     */
    bool EnablePublicNetwork;
    /*!
     * Enable or disable repeater support
     *
     * Related MIB type: \ref MIB_REPEATER_SUPPORT
     */
    bool EnableRepeaterSupport;
    /*!
     * LoRaWAN Channel
     *
     * Related MIB type: \ref MIB_CHANNELS
     */
    ChannelParams_t *ChannelList;
    /*!
    * Channel for the receive window 2
    *
    * Related MIB type: \ref MIB_RX2_CHANNEL
    */
    Rx2ChannelParams_t Rx2Channel;
    /*!
    * Channel for the receive window 2
    *
    * Related MIB type: \ref MIB_RX2_DEFAULT_CHANNEL
    */
    Rx2ChannelParams_t Rx2DefaultChannel;
    /*!
     * Channel mask
     *
     * Related MIB type: \ref MIB_CHANNELS_MASK
     */
    uint16_t *ChannelsMask;
    /*!
     * Default channel mask
     *
     * Related MIB type: \ref MIB_CHANNELS_DEFAULT_MASK
     */
    uint16_t *ChannelsDefaultMask;
    /*!
     * Number of frame repetitions
     *
     * Related MIB type: \ref MIB_CHANNELS_NB_REP
     */
    uint8_t ChannelNbRep;
    /*!
     * Maximum receive window duration
     *
     * Related MIB type: \ref MIB_MAX_RX_WINDOW_DURATION
     */
    uint32_t MaxRxWindow;
    /*!
     * Receive delay 1
     *
     * Related MIB type: \ref MIB_RECEIVE_DELAY_1
     */
    uint32_t ReceiveDelay1;
    /*!
     * Receive delay 2
     *
     * Related MIB type: \ref MIB_RECEIVE_DELAY_2
     */
    uint32_t ReceiveDelay2;
    /*!
     * Join accept delay 1
     *
     * Related MIB type: \ref MIB_JOIN_ACCEPT_DELAY_1
     */
    uint32_t JoinAcceptDelay1;
    /*!
     * Join accept delay 2
     *
     * Related MIB type: \ref MIB_JOIN_ACCEPT_DELAY_2
     */
    uint32_t JoinAcceptDelay2;
    /*!
     * Channels data rate
     *
     * Related MIB type: \ref MIB_CHANNELS_DEFAULT_DATARATE
     */
    int8_t ChannelsDefaultDatarate;
    /*!
     * Channels data rate
     *
     * Related MIB type: \ref MIB_CHANNELS_DATARATE
     */
    int8_t ChannelsDatarate;
    /*!
     * Channels TX power
     *
     * Related MIB type: \ref MIB_CHANNELS_DEFAULT_TX_POWER
     */
    int8_t ChannelsDefaultTxPower;
    /*!
     * Channels TX power
     *
     * Related MIB type: \ref MIB_CHANNELS_TX_POWER
     */
    int8_t ChannelsTxPower;
    /*!
     * LoRaWAN Up-link counter
     *
     * Related MIB type: \ref MIB_UPLINK_COUNTER
     */
    uint32_t UpLinkCounter;
    /*!
     * LoRaWAN Down-link counter
     *
     * Related MIB type: \ref MIB_DOWNLINK_COUNTER
     */
    uint32_t DownLinkCounter;
    /*!
     * Multicast channel
     *
     * Related MIB type: \ref MIB_MULTICAST_CHANNEL
     */
    MulticastParams_t *MulticastList;
    /*!
     * System overall timing error in milliseconds.
     *
     * Related MIB type: \ref MIB_SYSTEM_MAX_RX_ERROR
     */
    uint32_t SystemMaxRxError;
    /*!
     * Minimum required number of symbols to detect an Rx frame
     *
     * Related MIB type: \ref MIB_MIN_RX_SYMBOLS
     */
    uint8_t MinRxSymbols;
    /*!
     * Antenna gain
     *
     * Related MIB type: \ref MIB_ANTENNA_GAIN
     */
    float AntennaGain;
    /*!
     * Default antenna gain
     *
     * Related MIB type: \ref MIB_DEFAULT_ANTENNA_GAIN
     */
    float DefaultAntennaGain;
    /*!
     * Beacon interval in ms
     *
     * Related MIB type: \ref MIB_BEACON_INTERVAL
     */
    uint32_t BeaconInterval;
    /*!
     * Beacon reserved time in ms
     *
     * Related MIB type: \ref MIB_BEACON_RESERVED
     */
    uint32_t BeaconReserved;
    /*!
     * Beacon guard time in ms
     *
     * Related MIB type: \ref MIB_BEACON_GUARD
     */
    uint32_t BeaconGuard;
    /*!
     * Beacon window time in ms
     *
     * Related MIB type: \ref MIB_BEACON_WINDOW
     */
    uint32_t BeaconWindow;
    /*!
     * Beacon window time in number of slots
     *
     * Related MIB type: \ref MIB_BEACON_WINDOW_SLOTS
     */
    uint32_t BeaconWindowSlots;
    /*!
     * Ping slot length time in ms
     *
     * Related MIB type: \ref MIB_PING_SLOT_WINDOW
     */
    uint32_t PingSlotWindow;
    /*!
     * Default symbol timeout for beacons and ping slot windows
     *
     * Related MIB type: \ref MIB_BEACON_SYMBOL_TO_DEFAULT
     */
    uint32_t BeaconSymbolToDefault;
    /*!
     * Maximum symbol timeout for beacons
     *
     * Related MIB type: \ref MIB_BEACON_SYMBOL_TO_EXPANSION_MAX
     */
    uint32_t BeaconSymbolToExpansionMax;
    /*!
     * Maximum symbol timeout for ping slots
     *
     * Related MIB type: \ref MIB_PING_SLOT_SYMBOL_TO_EXPANSION_MAX
     */
    uint32_t PingSlotSymbolToExpansionMax;
    /*!
     * Symbol expansion value for beacon windows in case of beacon
     * loss in symbols
     *
     * Related MIB type: \ref MIB_BEACON_SYMBOL_TO_EXPANSION_FACTOR
     */
    uint32_t BeaconSymbolToExpansionFactor;
    /*!
     * Symbol expansion value for ping slot windows in case of beacon
     * loss in symbols
     *
     * Related MIB type: \ref MIB_PING_SLOT_SYMBOL_TO_EXPANSION_FACTOR
     */
    uint32_t PingSlotSymbolToExpansionFactor;
    /*!
     * Maximum allowed beacon less time in ms
     *
     * Related MIB type: \ref MIB_MAX_BEACON_LESS_PERIOD
     */
    uint32_t MaxBeaconLessPeriod;
    /*!
     * Ping slots data rate
     *
     * Related MIB type: \ref MIB_PING_SLOT_DATARATE
     */
    int8_t PingSlotDatarate;
    
#ifdef CONFIG_LWAN
    uint8_t Rx1DrOffset;
    uint32_t LoRaMacState;
#endif

#ifdef CONFIG_LINKWAN
    uint32_t freqband;
#endif
} MibParam_t;

/*!
 * LoRaMAC MIB-RequestConfirm structure
 */
typedef struct eMibRequestConfirm {
    /*!
     * MIB-Request type
     */
    Mib_t Type;

    /*!
     * MLME-RequestConfirm parameters
     */
    MibParam_t Param;
} MibRequestConfirm_t;

/*!
 * LoRaMAC tx information
 */
typedef struct sLoRaMacTxInfo {
    /*!
     * Defines the size of the applicative payload which can be processed
     */
    uint8_t MaxPossiblePayload;
    /*!
     * The current payload size, dependent on the current datarate
     */
    uint8_t CurrentPayloadSize;
} LoRaMacTxInfo_t;

/*!
 * LoRaMAC Status
 */
typedef enum eLoRaMacStatus {
    /*!
     * Service started successfully
     */
    LORAMAC_STATUS_OK,
    /*!
     * Service not started - LoRaMAC is busy
     */
    LORAMAC_STATUS_BUSY,
    /*!
     * Service unknown
     */
    LORAMAC_STATUS_SERVICE_UNKNOWN,
    /*!
     * Service not started - invalid parameter
     */
    LORAMAC_STATUS_PARAMETER_INVALID,
    /*!
     * Service not started - invalid frequency
     */
    LORAMAC_STATUS_FREQUENCY_INVALID,
    /*!
     * Service not started - invalid datarate
     */
    LORAMAC_STATUS_DATARATE_INVALID,
    /*!
     * Service not started - invalid frequency and datarate
     */
    LORAMAC_STATUS_FREQ_AND_DR_INVALID,
    /*!
     * Service not started - the device is not in a LoRaWAN
     */
    LORAMAC_STATUS_NO_NETWORK_JOINED,
    /*!
     * Service not started - payload length error
     */
    LORAMAC_STATUS_LENGTH_ERROR,
    /*!
     * Service not started - the device is switched off
     */
    LORAMAC_STATUS_DEVICE_OFF,
    /*!
     * Service not started - the specified region is not supported
     * or not activated with preprocessor definitions.
     */
    LORAMAC_STATUS_REGION_NOT_SUPPORTED,
    /*!
     * ToDo
     */
    LORAMAC_STATUS_DUTYCYCLE_RESTRICTED,
     /*!
      * ToDo
      */
    LORAMAC_STATUS_NO_CHANNEL_FOUND,
     /*!
      * ToDo
      */
    LORAMAC_STATUS_NO_FREE_CHANNEL_FOUND,
     /*!
      * ToDo
      */
    LORAMAC_STATUS_BUSY_BEACON_RESERVED_TIME,
     /*!
      * ToDo
      */
    LORAMAC_STATUS_BUSY_PING_SLOT_WINDOW_TIME,
     /*!
      * ToDo
      */
    LORAMAC_STATUS_BUSY_UPLINK_COLLISION
} LoRaMacStatus_t;

/*!
 * LoRaMAC region enumeration
 */
typedef enum eLoRaMacRegion_t {
    /*!
     * AS band on 923MHz
     */
    LORAMAC_REGION_AS923,
    /*!
     * Australian band on 915MHz
     */
    LORAMAC_REGION_AU915,
    /*!
     * Chinese band on 470MHz
     */
    LORAMAC_REGION_CN470,
    /*!
     * Chinese band on 779MHz
     */
    LORAMAC_REGION_CN779,
    /*!
     * European band on 433MHz
     */
    LORAMAC_REGION_EU433,
    /*!
     * European band on 868MHz
     */
    LORAMAC_REGION_EU868,
    /*!
     * South korean band on 920MHz
     */
    LORAMAC_REGION_KR920,
    /*!
     * India band on 865MHz
     */
    LORAMAC_REGION_IN865,
    /*!
     * North american band on 915MHz
     */
    LORAMAC_REGION_US915,
    /*!
     * North american band on 915MHz with a maximum of 16 channels
     */
    LORAMAC_REGION_US915_HYBRID,
    /*!
     * Australian band on 915MHz Subband 2
     */
    LORAMAC_REGION_AU915_SB2,
    /*!
     * AS band on 922.0-923.4MHz
     */
    LORAMAC_REGION_AS923_AS1,
    /*!
     * AS band on 923.2-924.6MHz
     */
    LORAMAC_REGION_AS923_AS2,
    /*!
     * Russia band on 864MHz
     */
    LORAMAC_REGION_RU864,
} LoRaMacRegion_t;

/*!
 * LoRaMAC events structure
 * Used to notify upper layers of MAC events
 */
typedef struct sLoRaMacPrimitives {
    /*!
     * \brief   MCPS-Confirm primitive
     *
     * \param   [OUT] MCPS-Confirm parameters
     */
    void ( *MacMcpsConfirm )( McpsConfirm_t *McpsConfirm );
    /*!
     * \brief   MCPS-Indication primitive
     *
     * \param   [OUT] MCPS-Indication parameters
     */
    void ( *MacMcpsIndication )( McpsIndication_t *McpsIndication );
    /*!
     * \brief   MLME-Confirm primitive
     *
     * \param   [OUT] MLME-Confirm parameters
     */
    void ( *MacMlmeConfirm )( MlmeConfirm_t *MlmeConfirm );
    /*!
     * \brief   MLME-Indication primitive
     *
     * \param   [OUT] MLME-Indication parameters
     */
    void ( *MacMlmeIndication )( MlmeIndication_t *MlmeIndication );
} LoRaMacPrimitives_t;

/*!
 * LoRaMAC callback structure
 */
typedef struct sLoRaMacCallback {
    /*!
     * \brief   Measures the battery level
     *
     * \retval  Battery level [0: node is connected to an external
     *          power source, 1..254: battery level, where 1 is the minimum
     *          and 254 is the maximum value, 255: the node was not able
     *          to measure the battery level]
     */
    uint8_t ( *GetBatteryLevel )( void );
    /*!
     * \brief   Measures the temperature level
     *
     * \retval  Temperature level
     */
    float ( *GetTemperatureLevel )( void );
} LoRaMacCallback_t;

/*!
 * LoRaMAC Max EIRP (dBm) table
 */
static const uint8_t LoRaMacMaxEirpTable[] = { 8, 10, 12, 13, 14, 16, 18, 20, 21, 24, 26, 27, 29, 30, 33, 36 };



/*!
 * \brief   LoRaMAC layer initialization
 *
 * \details In addition to the initialization of the LoRaMAC layer, this
 *          function initializes the callback primitives of the MCPS and
 *          MLME services. Every data field of \ref LoRaMacPrimitives_t must be
 *          set to a valid callback function.
 *
 * \param   [IN] primitives - Pointer to a structure defining the LoRaMAC
 *                            event functions. Refer to \ref LoRaMacPrimitives_t.
 *
 * \param   [IN] events - Pointer to a structure defining the LoRaMAC
 *                        callback functions. Refer to \ref LoRaMacCallback_t.
 *
 * \param   [IN] region - The region to start.
 *
 * \retval  LoRaMacStatus_t Status of the operation. Possible returns are:
 *          returns are:
 *          \ref LORAMAC_STATUS_OK,
 *          \ref LORAMAC_STATUS_PARAMETER_INVALID,
 *          \ref LORAMAC_STATUS_REGION_NOT_SUPPORTED.
 */
LoRaMacStatus_t LoRaMacInitialization( LoRaMacPrimitives_t *primitives, LoRaMacCallback_t *callbacks,
                                       LoRaMacRegion_t region );

/*!
 * \brief   Queries the LoRaMAC if it is possible to send the next frame with
 *          a given payload size. The LoRaMAC takes scheduled MAC commands into
 *          account and reports, when the frame can be send or not.
 *
 * \param   [IN] size - Size of applicative payload to be send next
 *
 * \param   [OUT] txInfo - The structure \ref LoRaMacTxInfo_t contains
 *                         information about the actual maximum payload possible
 *                         ( according to the configured datarate or the next
 *                         datarate according to ADR ), and the maximum frame
 *                         size, taking the scheduled MAC commands into account.
 *
 * \retval  LoRaMacStatus_t Status of the operation. When the parameters are
 *          not valid, the function returns \ref LORAMAC_STATUS_PARAMETER_INVALID.
 *          In case of a length error caused by the applicative payload in combination
 *          with the MAC commands, the function returns \ref LORAMAC_STATUS_LENGTH_ERROR.
 *          Please note that if the size of the MAC commands which are in the queue do
 *          not fit into the payload size on the related datarate, the LoRaMAC will
 *          omit the MAC commands.
 *          In case the query is valid, and the LoRaMAC is able to send the frame,
 *          the function returns \ref LORAMAC_STATUS_OK.
 */
LoRaMacStatus_t LoRaMacQueryTxPossible( uint8_t size, LoRaMacTxInfo_t *txInfo );

/*!
 * \brief   LoRaMAC channel add service
 *
 * \details Adds a new channel to the channel list and activates the id in
 *          the channel mask. Please note that this functionality is not available
 *          on all regions. Information about allowed ranges are available at the LoRaWAN Regional Parameters V1.0.2rB
 *
 * \param   [IN] id - Id of the channel.
 *
 * \param   [IN] params - Channel parameters to set.
 *
 * \retval  LoRaMacStatus_t Status of the operation. Possible returns are:
 *          \ref LORAMAC_STATUS_OK,
 *          \ref LORAMAC_STATUS_BUSY,
 *          \ref LORAMAC_STATUS_PARAMETER_INVALID.
 */
LoRaMacStatus_t LoRaMacChannelAdd( uint8_t id, ChannelParams_t params );

/*!
 * \brief   LoRaMAC channel remove service
 *
 * \details Deactivates the id in the channel mask.
 *
 * \param   [IN] id - Id of the channel.
 *
 * \retval  LoRaMacStatus_t Status of the operation. Possible returns are:
 *          \ref LORAMAC_STATUS_OK,
 *          \ref LORAMAC_STATUS_BUSY,
 *          \ref LORAMAC_STATUS_PARAMETER_INVALID.
 */
LoRaMacStatus_t LoRaMacChannelRemove( uint8_t id );

/*!
 * \brief   LoRaMAC multicast channel link service
 *
 * \details Links a multicast channel into the linked list.
 *
 * \param   [IN] channelParam - Multicast channel parameters to link.
 *
 * \retval  LoRaMacStatus_t Status of the operation. Possible returns are:
 *          \ref LORAMAC_STATUS_OK,
 *          \ref LORAMAC_STATUS_BUSY,
 *          \ref LORAMAC_STATUS_PARAMETER_INVALID.
 */
LoRaMacStatus_t LoRaMacMulticastChannelLink( MulticastParams_t *channelParam );

/*!
 * \brief   LoRaMAC multicast channel unlink service
 *
 * \details Unlinks a multicast channel from the linked list.
 *
 * \param   [IN] channelParam - Multicast channel parameters to unlink.
 *
 * \retval  LoRaMacStatus_t Status of the operation. Possible returns are:
 *          \ref LORAMAC_STATUS_OK,
 *          \ref LORAMAC_STATUS_BUSY,
 *          \ref LORAMAC_STATUS_PARAMETER_INVALID.
 */
LoRaMacStatus_t LoRaMacMulticastChannelUnlink( MulticastParams_t *channelParam );

/*!
 * \brief   LoRaMAC MIB-Get
 *
 * \details The mac information base service to get attributes of the LoRaMac
 *          layer.
 *
 *          The following code-snippet shows how to use the API to get the
 *          parameter AdrEnable, defined by the enumeration type
 *          \ref MIB_ADR.
 * \code
 * MibRequestConfirm_t mibReq;
 * mibReq.Type = MIB_ADR;
 *
 * if( LoRaMacMibGetRequestConfirm( &mibReq ) == LORAMAC_STATUS_OK )
 * {
 *   // LoRaMAC updated the parameter mibParam.AdrEnable
 * }
 * \endcode
 *
 * \param   [IN] mibRequest - MIB-GET-Request to perform. Refer to \ref MibRequestConfirm_t.
 *
 * \retval  LoRaMacStatus_t Status of the operation. Possible returns are:
 *          \ref LORAMAC_STATUS_OK,
 *          \ref LORAMAC_STATUS_SERVICE_UNKNOWN,
 *          \ref LORAMAC_STATUS_PARAMETER_INVALID.
 */
LoRaMacStatus_t LoRaMacMibGetRequestConfirm( MibRequestConfirm_t *mibGet );

/*!
 * \brief   LoRaMAC MIB-Set
 *
 * \details The mac information base service to set attributes of the LoRaMac
 *          layer.
 *
 *          The following code-snippet shows how to use the API to set the
 *          parameter AdrEnable, defined by the enumeration type
 *          \ref MIB_ADR.
 *
 * \code
 * MibRequestConfirm_t mibReq;
 * mibReq.Type = MIB_ADR;
 * mibReq.Param.AdrEnable = true;
 *
 * if( LoRaMacMibGetRequestConfirm( &mibReq ) == LORAMAC_STATUS_OK )
 * {
 *   // LoRaMAC updated the parameter
 * }
 * \endcode
 *
 * \param   [IN] mibRequest - MIB-SET-Request to perform. Refer to \ref MibRequestConfirm_t.
 *
 * \retval  LoRaMacStatus_t Status of the operation. Possible returns are:
 *          \ref LORAMAC_STATUS_OK,
 *          \ref LORAMAC_STATUS_BUSY,
 *          \ref LORAMAC_STATUS_SERVICE_UNKNOWN,
 *          \ref LORAMAC_STATUS_PARAMETER_INVALID.
 */
LoRaMacStatus_t LoRaMacMibSetRequestConfirm( MibRequestConfirm_t *mibSet );

/*!
 * \brief   LoRaMAC MLME-Request
 *
 * \details The Mac layer management entity handles management services. The
 *          following code-snippet shows how to use the API to perform a
 *          network join request.
 *
 * \code
 * static uint8_t DevEui[] =
 * {
 *   0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00
 * };
 * static uint8_t AppEui[] =
 * {
 *   0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00
 * };
 * static uint8_t AppKey[] =
 * {
 *   0x2B, 0x7E, 0x15, 0x16, 0x28, 0xAE, 0xD2, 0xA6,
 *   0xAB, 0xF7, 0x15, 0x88, 0x09, 0xCF, 0x4F, 0x3C
 * };
 *
 * MlmeReq_t mlmeReq;
 * mlmeReq.Type = MLME_JOIN;
 * mlmeReq.Req.Join.DevEui = DevEui;
 * mlmeReq.Req.Join.AppEui = AppEui;
 * mlmeReq.Req.Join.AppKey = AppKey;
 *
 * if( LoRaMacMlmeRequest( &mlmeReq ) == LORAMAC_STATUS_OK )
 * {
 *   // Service started successfully. Waiting for the Mlme-Confirm event
 * }
 * \endcode
 *
 * \param   [IN] mlmeRequest - MLME-Request to perform. Refer to \ref MlmeReq_t.
 *
 * \retval  LoRaMacStatus_t Status of the operation. Possible returns are:
 *          \ref LORAMAC_STATUS_OK,
 *          \ref LORAMAC_STATUS_BUSY,
 *          \ref LORAMAC_STATUS_SERVICE_UNKNOWN,
 *          \ref LORAMAC_STATUS_PARAMETER_INVALID,
 *          \ref LORAMAC_STATUS_NO_NETWORK_JOINED,
 *          \ref LORAMAC_STATUS_LENGTH_ERROR,
 *          \ref LORAMAC_STATUS_DEVICE_OFF.
 */
LoRaMacStatus_t LoRaMacMlmeRequest( MlmeReq_t *mlmeRequest );

/*!
 * LoRaMac internal states
 */
enum eLoRaMacState {
    LORAMAC_IDLE          = 0x00000000,
    LORAMAC_TX_RUNNING    = 0x00000001,
    LORAMAC_RX            = 0x00000002,
    LORAMAC_ACK_REQ       = 0x00000004,
    LORAMAC_ACK_RETRY     = 0x00000008,
    LORAMAC_TX_DELAYED    = 0x00000010,
    LORAMAC_TX_CONFIG     = 0x00000020,
    LORAMAC_RX_ABORT      = 0x00000040,
    LORAMAC_CAD_RUNNING   = 0x00000080,
};


/*!
 * \brief   LoRaMAC MCPS-Request
 *
 * \details The Mac Common Part Sublayer handles data services. The following
 *          code-snippet shows how to use the API to send an unconfirmed
 *          LoRaMAC frame.
 *
 * \code
 * uint8_t myBuffer[] = { 1, 2, 3 };
 *
 * McpsReq_t mcpsReq;
 * mcpsReq.Type = MCPS_UNCONFIRMED;
 * mcpsReq.Req.Unconfirmed.fPort = 1;
 * mcpsReq.Req.Unconfirmed.fBuffer = myBuffer;
 * mcpsReq.Req.Unconfirmed.fBufferSize = sizeof( myBuffer );
 *
 * if( LoRaMacMcpsRequest( &mcpsReq ) == LORAMAC_STATUS_OK )
 * {
 *   // Service started successfully. Waiting for the MCPS-Confirm event
 * }
 * \endcode
 *
 * \param   [IN] mcpsRequest - MCPS-Request to perform. Refer to \ref McpsReq_t.
 *
 * \retval  LoRaMacStatus_t Status of the operation. Possible returns are:
 *          \ref LORAMAC_STATUS_OK,
 *          \ref LORAMAC_STATUS_BUSY,
 *          \ref LORAMAC_STATUS_SERVICE_UNKNOWN,
 *          \ref LORAMAC_STATUS_PARAMETER_INVALID,
 *          \ref LORAMAC_STATUS_NO_NETWORK_JOINED,
 *          \ref LORAMAC_STATUS_LENGTH_ERROR,
 *          \ref LORAMAC_STATUS_DEVICE_OFF.
 */
LoRaMacStatus_t LoRaMacMcpsRequest( McpsReq_t *mcpsRequest );

extern	void turnOnRGB(uint32_t color,uint32_t time);
extern	void turnOffRGB(void);


#ifdef __cplusplus
}
#endif

#include "region/Region.h"

/*! \} defgroup LORAMAC */

#endif // __LORAMAC_H__
