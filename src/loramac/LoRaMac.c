/*
 * Copyright (C) 2015-2017 Alibaba Group Holding Limited
 */

/*
 / _____)             _              | |
( (____  _____ ____ _| |_ _____  ____| |__
 \____ \| ___ |    (_   _) ___ |/ ___)  _ \
 _____) ) ____| | | || |_| ____( (___| | | |
(______/|_____)_|_|_| \__)_____)\____)_| |_|
    (C)2013 Semtech
 ___ _____ _   ___ _  _____ ___  ___  ___ ___
/ __|_   _/_\ / __| |/ / __/ _ \| _ \/ __| __|
\__ \ | |/ _ \ (__| ' <| _| (_) |   / (__| _|
|___/ |_/_/ \_\___|_|\_\_| \___/|_|_\\___|___|
embedded.connectivity.solutions===============

Description: LoRa MAC layer implementation

License: Revised BSD License, see LICENSE.TXT file include in the project

Maintainer: Miguel Luis ( Semtech ), Gregory Cristian ( Semtech ) and Daniel Jaeckle ( STACKFORCE )
*/


#include <stdbool.h>
#include <string.h>
#include <stdint.h>
#include "../loramac/LoRaMac.h"
#include "region/Region.h"
#include "../loramac/LoRaMacClassB.h"
#include "../loramac/LoRaMacCrypto.h"
#include "../driver/debug.h"
#include "../loramac/LoRaMacTest.h"
#include "../loramac/LoRaMacConfirmQueue.h"
#include "esp_attr.h" 
#include "esp_sleep.h"
//#define CONFIG_LORA_CAD

#ifdef CONFIG_LORA_VERIFY
extern bool g_lora_debug;
TimerTime_t mcps_start_time;
#endif
/*!
 * Maximum PHY layer payload size
 */
#define LORAMAC_PHY_MAXPAYLOAD                      255

/*!
 * Maximum MAC commands buffer size
 */
#define LORA_MAC_COMMAND_MAX_LENGTH                 128

/*!
 * Maximum length of the fOpts field
 */
#define LORA_MAC_COMMAND_MAX_FOPTS_LENGTH           15

/*!
 * LoRaMac region.
 */
RTC_DATA_ATTR LoRaMacRegion_t LoRaMacRegion;

/*!
 * LoRaMac duty cycle for the back-off procedure during the first hour.
 */
#define BACKOFF_DC_1_HOUR                           100

/*!
 * LoRaMac duty cycle for the back-off procedure during the next 10 hours.
 */
#define BACKOFF_DC_10_HOURS                         1000

/*!
 * LoRaMac duty cycle for the back-off procedure during the next 24 hours.
 */
#define BACKOFF_DC_24_HOURS                         10000

#ifdef CONFIG_LORA_CAD
#define LORA_CAD_CNT_MAX 8    //send frame after LORA_CAD_CNT_MAX times CAD
#define LORA_CAD_SYMBOLS 8   //CAD symbols 
#define LORA_CAD_DELAY  1000   //delay 1s after CAD
#endif 
#ifdef CLASS_A_WOTA
bool wota_CadStarted = false;
bool wota_CadTimerStarted = false;
bool wota_CadEnable = false;
uint32_t wota_cycle_time = 1000;
uint32_t wota_max_rxtime = 2000;
int8_t wota_dr = DR_0;
uint32_t wota_freq = 505300000;
#endif

/*!
 * Device IEEE EUI
 */
RTC_DATA_ATTR static uint8_t *LoRaMacDevEui;

/*!
 * Application IEEE EUI
 */
RTC_DATA_ATTR static uint8_t *LoRaMacAppEui;

/*!
 * AES encryption/decryption cipher application key
 */
RTC_DATA_ATTR uint8_t *LoRaMacAppKey;

/*!
 * AES encryption/decryption cipher network session key
 */
RTC_DATA_ATTR static uint8_t LoRaMacNwkSKey[] = {
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00
};

/*!
 * AES encryption/decryption cipher application session key
 */
RTC_DATA_ATTR static uint8_t LoRaMacAppSKey[] = {
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00
};

/*!
 * Device nonce is a random value extracted by issuing a sequence of RSSI
 * measurements
 */
RTC_DATA_ATTR uint16_t LoRaMacDevNonce;

/*!
 * Network ID ( 3 bytes )
 */
RTC_DATA_ATTR static uint32_t LoRaMacNetID;

/*!
 * Mote Address
 */
RTC_DATA_ATTR static uint32_t LoRaMacDevAddr;

/*!
 * Multicast channels linked list
 */
RTC_DATA_ATTR static MulticastParams_t *MulticastChannels = NULL;

/*!
 * Actual device class
 */
RTC_DATA_ATTR static DeviceClass_t LoRaMacDeviceClass;

/*!
 * Indicates if the node is connected to a private or public network
 */
RTC_DATA_ATTR static bool PublicNetwork;

/*!
 * Buffer containing the data to be sent or received.
 */
RTC_DATA_ATTR static uint8_t LoRaMacBuffer[LORAMAC_PHY_MAXPAYLOAD];

/*!
 * Length of packet in LoRaMacBuffer
 */
RTC_DATA_ATTR static uint16_t LoRaMacBufferPktLen = 0;

/*!
 * Length of the payload in LoRaMacBuffer
 */
RTC_DATA_ATTR static uint8_t LoRaMacTxPayloadLen = 0;

/*!
 * Buffer containing the upper layer data.
 */
RTC_DATA_ATTR static uint8_t LoRaMacRxPayload[LORAMAC_PHY_MAXPAYLOAD];

/*!
 * LoRaMAC frame counter. Each time a packet is sent the counter is incremented.
 * Only the 16 LSB bits are sent
 */
RTC_DATA_ATTR uint32_t UpLinkCounter = 0;

/*!
 * LoRaMAC frame counter. Each time a packet is received the counter is incremented.
 * Only the 16 LSB bits are received
 */
RTC_DATA_ATTR uint32_t DownLinkCounter = 0;

/*!
 * IsPacketCounterFixed enables the MIC field tests by fixing the
 * UpLinkCounter value
 */
RTC_DATA_ATTR static bool IsUpLinkCounterFixed = false;

/*!
 * Used for test purposes. Disables the opening of the reception windows.
 */
RTC_DATA_ATTR static bool IsRxWindowsEnabled = true;

/*!
 * Indicates if the MAC layer has already joined a network.
 */
RTC_DATA_ATTR bool IsLoRaMacNetworkJoined = false;

/*!
 * LoRaMac ADR control status
 */
RTC_DATA_ATTR static bool AdrCtrlOn = false;

/*!
 * Counts the number of missed ADR acknowledgements
 */
RTC_DATA_ATTR static uint32_t AdrAckCounter = 0;

/*!
 * If the node has sent a FRAME_TYPE_DATA_CONFIRMED_UP this variable indicates
 * if the nodes needs to manage the server acknowledgement.
 */
RTC_DATA_ATTR static bool NodeAckRequested = false;

/*!
 * If the server has sent a FRAME_TYPE_DATA_CONFIRMED_DOWN this variable indicates
 * if the ACK bit must be set for the next transmission
 */
RTC_DATA_ATTR static bool SrvAckRequested = false;

/*!
 * Indicates if the MAC layer wants to send MAC commands
 */
RTC_DATA_ATTR static bool MacCommandsInNextTx = false;

/*!
 * Contains the current MacCommandsBuffer index
 */
RTC_DATA_ATTR static uint8_t MacCommandsBufferIndex = 0;

/*!
 * Contains the current MacCommandsBuffer index for MAC commands to repeat
 */
RTC_DATA_ATTR static uint8_t MacCommandsBufferToRepeatIndex = 0;

/*!
 * Buffer containing the MAC layer commands
 */
RTC_DATA_ATTR static uint8_t MacCommandsBuffer[LORA_MAC_COMMAND_MAX_LENGTH];

/*!
 * Buffer containing the MAC layer commands which must be repeated
 */
RTC_DATA_ATTR static uint8_t MacCommandsBufferToRepeat[LORA_MAC_COMMAND_MAX_LENGTH];

/*!
 * LoRaMac parameters
 */
RTC_DATA_ATTR LoRaMacParams_t LoRaMacParams;

/*!
 * LoRaMac default parameters
 */
RTC_DATA_ATTR LoRaMacParams_t LoRaMacParamsDefaults;

/*!
 * Uplink messages repetitions counter
 */
RTC_DATA_ATTR static uint8_t ChannelsNbRepCounter = 0;

/*!
 * Maximum duty cycle
 * \remark Possibility to shutdown the device.
 */
RTC_DATA_ATTR static uint8_t MaxDCycle = 0;

/*!
 * Aggregated duty cycle management
 */
RTC_DATA_ATTR static uint16_t AggregatedDCycle;
RTC_DATA_ATTR static TimerTime_t AggregatedLastTxDoneTime;
RTC_DATA_ATTR static TimerTime_t AggregatedTimeOff;

/*!
 * Enables/Disables duty cycle management (Test only)
 */
RTC_DATA_ATTR static bool DutyCycleOn;

/*!
 * Current channel index
 */
RTC_DATA_ATTR static uint8_t Channel;

/*!
 * Current channel index
 */
RTC_DATA_ATTR static uint8_t LastTxChannel;

/*!
 * Set to true, if the last uplink was a join request
 */
RTC_DATA_ATTR static bool LastTxIsJoinRequest;

/*!
 * Stores the time at LoRaMac initialization.
 *
 * \remark Used for the BACKOFF_DC computation.
 */
RTC_DATA_ATTR static TimerTime_t LoRaMacInitializationTime = 0;

RTC_DATA_ATTR static TimerSysTime_t LastTxSysTime = { 0 };


/*!
 * LoRaMac internal state
 */
RTC_DATA_ATTR uint32_t LoRaMacState = LORAMAC_IDLE;

/*!
 * LoRaMac timer used to check the LoRaMacState (runs every second)
 */
RTC_DATA_ATTR static TimerEvent_t MacStateCheckTimer;

/*!
 * LoRaMac upper layer event functions
 */
RTC_DATA_ATTR static LoRaMacPrimitives_t *LoRaMacPrimitives;

/*!
 * LoRaMac upper layer callback functions
 */
RTC_DATA_ATTR static LoRaMacCallback_t *LoRaMacCallbacks;

/*!
 * Radio events function pointer
 */
RTC_DATA_ATTR static RadioEvents_t RadioEvents;

/*!
 * LoRaMac duty cycle delayed Tx timer
 */
RTC_DATA_ATTR TimerEvent_t TxDelayedTimer;

#ifdef CONFIG_LORA_CAD
RTC_DATA_ATTR static TimerEvent_t TxImmediateTimer;
RTC_DATA_ATTR static uint8_t g_lora_cad_cnt = 1;    
#endif

#ifdef CLASS_A_WOTA
RTC_DATA_ATTR TimerEvent_t wota_CadTimer;
#endif
/*!
 * LoRaMac reception windows timers
 */
RTC_DATA_ATTR TimerEvent_t RxWindowTimer1;
RTC_DATA_ATTR TimerEvent_t RxWindowTimer2;

/*!
 * LoRaMac reception windows delay
 * \remark normal frame: RxWindowXDelay = ReceiveDelayX - RADIO_WAKEUP_TIME
 *         join frame  : RxWindowXDelay = JoinAcceptDelayX - RADIO_WAKEUP_TIME
 */
RTC_DATA_ATTR uint32_t RxWindow1Delay;
RTC_DATA_ATTR uint32_t RxWindow2Delay;

/*!
 * LoRaMac Rx windows configuration
 */
RTC_DATA_ATTR static RxConfigParams_t RxWindow1Config;
RTC_DATA_ATTR static RxConfigParams_t RxWindow2Config;
#ifdef CLASS_A_WOTA
RTC_DATA_ATTR static RxConfigParams_t RxWindow3Config;
#endif
/*!
 * Acknowledge timeout timer. Used for packet retransmissions.
 */
RTC_DATA_ATTR TimerEvent_t AckTimeoutTimer;

/*!
 * Number of trials to get a frame acknowledged
 */
RTC_DATA_ATTR uint8_t AckTimeoutRetries = 1;

/*!
 * Number of trials to get a frame acknowledged
 */
RTC_DATA_ATTR uint8_t AckTimeoutRetriesCounter = 1;

/*!
 * Indicates if the AckTimeout timer has expired or not
 */
RTC_DATA_ATTR bool AckTimeoutRetry = false;

/*!
 * Last transmission time on air
 */
RTC_DATA_ATTR TimerTime_t TxTimeOnAir = 0;

/*!
 * Number of trials for the Join Request
 */
RTC_DATA_ATTR uint8_t JoinRequestTrials;

/*!
 * Maximum number of trials for the Join Request
 */
RTC_DATA_ATTR uint8_t MaxJoinRequestTrials;

/*!
 * Structure to hold an MCPS indication data.
 */
RTC_DATA_ATTR McpsIndication_t McpsIndication;

/*!
 * Structure to hold MCPS confirm data.
 */
RTC_DATA_ATTR McpsConfirm_t McpsConfirm;

/*!
 * Structure to hold MLME indication data.
 */
RTC_DATA_ATTR MlmeIndication_t MlmeIndication;

/*!
 * Structure to hold MLME confirm data.
 */
RTC_DATA_ATTR MlmeConfirm_t MlmeConfirm;

/*!
 * Holds the current rx window slot
 */
RTC_DATA_ATTR LoRaMacRxSlot_t RxSlot;

/*!
 * LoRaMac tx/rx operation state
 */
RTC_DATA_ATTR LoRaMacFlags_t LoRaMacFlags;
RTC_DATA_ATTR bool lora_txing=false;

#ifdef CONFIG_LWAN
RTC_DATA_ATTR static bool DownLinkFramePending = false;    
#endif    

/*!
 * \brief Function to be executed on Radio Tx Done event
 */
static void OnRadioTxDone( void );

/*!
 * \brief This function prepares the MAC to abort the execution of function
 *        OnRadioRxDone in case of a reception error.
 */
static void PrepareRxDoneAbort( void );

/*!
 * \brief Function to be executed on Radio Rx Done event
 */
void OnRadioRxDone( uint8_t *payload, uint16_t size, int16_t rssi, int8_t snr );

/*!
 * \brief Function executed on Radio Tx Timeout event
 */
static void OnRadioTxTimeout( void );

/*!
 * \brief Function executed on Radio Rx error event
 */
static void OnRadioRxError( void );

/*!
 * \brief Function executed on Radio Rx Timeout event
 */
static void OnRadioRxTimeout( void );

static void OnRadioCadDone( bool channelActivityDetected );
#ifdef CONFIG_LORA_CAD
static void beforeTxCadDone( bool channelActivityDetected );
static void OnTxImmediateTimerEvent( void );
#endif 
#ifdef CLASS_A_WOTA
static void wota_CadDone( bool channelActivityDetected );
#endif

/*!
 * \brief Function executed on Resend Frame timer event.
 */
static void OnMacStateCheckTimerEvent( void );

/*!
 * \brief Function executed on duty cycle delayed Tx  timer event
 */
static void OnTxDelayedTimerEvent( void );

/*!
 * \brief Function executed on first Rx window timer event
 */
static void OnRxWindow1TimerEvent( void );

/*!
 * \brief Function executed on second Rx window timer event
 */
static void OnRxWindow2TimerEvent( void );

/*!
 * \brief Check if the OnAckTimeoutTimer has do be disabled. If so, the
 *        function disables it.
 *
 * \param [IN] nodeAckRequested Set to true, if the node has requested an ACK
 * \param [IN] class The device class
 * \param [IN] ackReceived Set to true, if the node has received an ACK
 * \param [IN] ackTimeoutRetriesCounter Retries counter for confirmed uplinks
 * \param [IN] ackTimeoutRetries Maximum retries for confirmed uplinks
 */
static void CheckToDisableAckTimeout( bool nodeAckRequested, DeviceClass_t devClass, bool ackReceived,
                                      uint8_t ackTimeoutRetriesCounter, uint8_t ackTimeoutRetries );

/*!
 * \brief Function executed on AckTimeout timer event
 */
static void OnAckTimeoutTimerEvent( void );

/*!
 * \brief Initializes and opens the reception window
 *
 * \param [IN] rxContinuous Set to true, if the RX is in continuous mode
 * \param [IN] maxRxWindow Maximum RX window timeout
 */
static void RxWindowSetup( bool rxContinuous, uint32_t maxRxWindow );

/*!
 * \brief Verifies if sticky MAC commands are pending.
 *
 * \retval [true: sticky MAC commands pending, false: No MAC commands pending]
 */
static bool IsStickyMacCommandPending( void );

/*!
 * \brief Configures the events to trigger an MLME-Indication with
 *        a MLME type of MLME_SCHEDULE_UPLINK.
 */
static void SetMlmeScheduleUplinkIndication( void );

/*!
 * \brief Switches the device class
 *
 * \param [IN] deviceClass Device class to switch to
 */
static LoRaMacStatus_t SwitchClass( DeviceClass_t deviceClass );

/*!
 * \brief Adds a new MAC command to be sent.
 *
 * \Remark MAC layer internal function
 *
 * \param [in] cmd MAC command to be added
 *                 [MOTE_MAC_LINK_CHECK_REQ,
 *                  MOTE_MAC_LINK_ADR_ANS,
 *                  MOTE_MAC_DUTY_CYCLE_ANS,
 *                  MOTE_MAC_RX2_PARAM_SET_ANS,
 *                  MOTE_MAC_DEV_STATUS_ANS
 *                  MOTE_MAC_NEW_CHANNEL_ANS]
 * \param [in] p1  1st parameter ( optional depends on the command )
 * \param [in] p2  2nd parameter ( optional depends on the command )
 *
 * \retval status  Function status [0: OK, 1: Unknown command, 2: Buffer full]
 */
static LoRaMacStatus_t AddMacCommand( uint8_t cmd, uint8_t p1, uint8_t p2 );

/*!
 * \brief Parses the MAC commands which must be repeated.
 *
 * \Remark MAC layer internal function
 *
 * \param [IN] cmdBufIn  Buffer which stores the MAC commands to send
 * \param [IN] length  Length of the input buffer to parse
 * \param [OUT] cmdBufOut  Buffer which stores the MAC commands which must be
 *                         repeated.
 *
 * \retval Size of the MAC commands to repeat.
 */
static uint8_t ParseMacCommandsToRepeat( uint8_t *cmdBufIn, uint8_t length, uint8_t *cmdBufOut );

/*!
 * \brief Validates if the payload fits into the frame, taking the datarate
 *        into account.
 *
 * \details Refer to chapter 4.3.2 of the LoRaWAN specification, v1.0
 *
 * \param lenN Length of the application payload. The length depends on the
 *             datarate and is region specific
 *
 * \param datarate Current datarate
 *
 * \param fOptsLen Length of the fOpts field
 *
 * \retval [false: payload does not fit into the frame, true: payload fits into
 *          the frame]
 */
static bool ValidatePayloadLength( uint8_t lenN, int8_t datarate, uint8_t fOptsLen );

/*!
 * \brief Decodes MAC commands in the fOpts field and in the payload
 *
 * \param [IN] payload      A pointer to the payload
 * \param [IN] macIndex     The index of the payload where the MAC commands start
 * \param [IN] commandsSize The size of the MAC commands
 * \param [IN] snr          The SNR value  of the frame
 * \param [IN] rxSlot       The RX slot where the frame was received
 */
static void ProcessMacCommands( uint8_t *payload, uint8_t macIndex, uint8_t commandsSize, uint8_t snr, LoRaMacRxSlot_t rxSlot );

/*!
 * \brief LoRaMAC layer generic send frame
 *
 * \param [IN] macHdr      MAC header field
 * \param [IN] fPort       MAC payload port
 * \param [IN] fBuffer     MAC data buffer to be sent
 * \param [IN] fBufferSize MAC data buffer size
 * \retval status          Status of the operation.
 */
LoRaMacStatus_t Send( LoRaMacHeader_t *macHdr, uint8_t fPort, void *fBuffer, uint16_t fBufferSize );

/*!
 * \brief LoRaMAC layer frame buffer initialization
 *
 * \param [IN] macHdr      MAC header field
 * \param [IN] fCtrl       MAC frame control field
 * \param [IN] fOpts       MAC commands buffer
 * \param [IN] fPort       MAC payload port
 * \param [IN] fBuffer     MAC data buffer to be sent
 * \param [IN] fBufferSize MAC data buffer size
 * \retval status          Status of the operation.
 */
LoRaMacStatus_t PrepareFrame( LoRaMacHeader_t *macHdr, LoRaMacFrameCtrl_t *fCtrl, uint8_t fPort, void *fBuffer,
                              uint16_t fBufferSize );

/*
 * \brief Schedules the frame according to the duty cycle
 *
 * \retval Status of the operation
 */
static LoRaMacStatus_t ScheduleTx( void );

/*
 * \brief Calculates the back-off time for the band of a channel.
 *
 * \param [IN] channel     The last Tx channel index
 */
static void CalculateBackOff( uint8_t channel );

/*!
 * \brief LoRaMAC layer prepared frame buffer transmission with channel specification
 *
 * \remark PrepareFrame must be called at least once before calling this
 *         function.
 *
 * \param [IN] channel     Channel to transmit on
 * \retval status          Status of the operation.
 */
LoRaMacStatus_t SendFrameOnChannel( uint8_t channel );

/*!
 * \brief Sets the radio in continuous transmission mode
 *
 * \remark Uses the radio parameters set on the previous transmission.
 *
 * \param [IN] timeout     Time in seconds while the radio is kept in continuous wave mode
 * \retval status          Status of the operation.
 */
LoRaMacStatus_t SetTxContinuousWave( uint16_t timeout );

/*!
 * \brief Sets the radio in continuous transmission mode
 *
 * \remark Uses the radio parameters set on the previous transmission.
 *
 * \param [IN] timeout     Time in seconds while the radio is kept in continuous wave mode
 * \param [IN] frequency   RF frequency to be set.
 * \param [IN] power       RF output power to be set.
 * \retval status          Status of the operation.
 */
LoRaMacStatus_t SetTxContinuousWave1( uint16_t timeout, uint32_t frequency, uint8_t power );

/*!
 * \brief Sets the network to public or private. Updates the sync byte.
 *
 * \param [IN] enable if true, it enables a public network
 */
//static void SetPublicNetwork( bool enable );

/*!
 * \brief Resets MAC specific parameters to default
 */
static void ResetMacParameters( void );

/*!
 * \brief Resets MAC specific parameters to default
 *
 * \param [IN] fPort     The fPort
 *
 * \retval [false: fPort not allowed, true: fPort allowed]
 */
static bool IsFPortAllowed( uint8_t fPort );

/*!
 * \brief Opens up a continuous RX 2 window. This is used for
 *        class c devices.
 */
static void OpenContinuousRx2Window( void );
extern uint64_t esp32deepsleepWaketime;
static void OnRadioTxDone( void )
{
	DIO_PRINTF("Event : Tx Done\r\n");
	lora_txing=false;
    GetPhyParams_t getPhy;
    PhyParam_t phyParam;
    SetBandTxDoneParams_t txDone;
    TimerTime_t curTime = TimerGetCurrentTime( );
    LastTxSysTime = TimerGetSysTime( );

    // Setup timers
    if ( IsRxWindowsEnabled == true ) {
        TimerSetValue( &RxWindowTimer1, RxWindow1Delay );
        TimerStart( &RxWindowTimer1 );
        if ( LoRaMacDeviceClass != CLASS_C ) {
            TimerSetValue( &RxWindowTimer2, RxWindow2Delay );
            TimerStart( &RxWindowTimer2 );
        }
        if ( ( LoRaMacDeviceClass == CLASS_C ) || ( NodeAckRequested == true ) ) {
            getPhy.Attribute = PHY_ACK_TIMEOUT;
            phyParam = RegionGetPhyParam( LoRaMacRegion, &getPhy );
            TimerSetValue( &AckTimeoutTimer, RxWindow2Delay + phyParam.Value );
            TimerStart( &AckTimeoutTimer );
        }
    } else {
        McpsConfirm.Status = LORAMAC_EVENT_INFO_STATUS_OK;
        LoRaMacConfirmQueueSetStatusCmn( LORAMAC_EVENT_INFO_STATUS_RX2_TIMEOUT );

        if ( LoRaMacFlags.Value == 0 ) {
            LoRaMacFlags.Bits.McpsReq = 1;
        }
        LoRaMacFlags.Bits.MacDone = 1;
    }

    if( LoRaMacDeviceClass != CLASS_C )
    {
        Radio.Sleep( );
    }
    else
    {
        OpenContinuousRx2Window( );
    }

    // Verify if the last uplink was a join request
    if ( ( LoRaMacFlags.Bits.MlmeReq == 1 ) && ( LoRaMacConfirmQueueIsCmdActive( MLME_JOIN ) == true ) ) {
        LastTxIsJoinRequest = true;
    } else {
        LastTxIsJoinRequest = false;
    }

    // Store last Tx channel
    LastTxChannel = Channel;
    // Update last tx done time for the current channel
    txDone.Channel = Channel;
    txDone.Joined = IsLoRaMacNetworkJoined;
    txDone.LastTxDoneTime = curTime;
    RegionSetBandTxDone( LoRaMacRegion, &txDone );
    // Update Aggregated last tx done time
    AggregatedLastTxDoneTime = curTime;

    if ( NodeAckRequested == false ) {
        McpsConfirm.Status = LORAMAC_EVENT_INFO_STATUS_OK;
#ifdef CONFIG_LWAN        
        McpsConfirm.NbRetries++;
#endif    
        ChannelsNbRepCounter++;
    }
#ifdef CONFIG_LORA_VERIFY
    if (g_lora_debug) {
        PRINTF_RAW("The trasaction consume %llu time(ms)\r\n", curTime - mcps_start_time);
    }
#endif
#ifdef CONFIG_LWAN
    lwan_dev_status_set(DEVICE_STATUS_SEND_PASS);
#endif

#if (LoraWan_RGB==1)
		turnOffRGB();
#endif

}

static void PrepareRxDoneAbort( void )
{
  lora_txing=false;
    LoRaMacState |= LORAMAC_RX_ABORT;

    if ( NodeAckRequested ) {
        OnAckTimeoutTimerEvent( );
    }

    LoRaMacFlags.Bits.McpsInd = 1;
    LoRaMacFlags.Bits.MacDone = 1;

    // Trig OnMacCheckTimerEvent call as soon as possible
    TimerSetValue( &MacStateCheckTimer, 1 );
    TimerStart( &MacStateCheckTimer );
    //OnMacStateCheckTimerEvent();
}

void OnRadioRxDone( uint8_t *payload, uint16_t size, int16_t rssi, int8_t snr )
{
  lora_txing=false;
	DIO_PRINTF("Event : Rx Done\r\n");
#ifdef CLASS_A_WOTA
	if(wota_CadStarted)
		wota_CadStarted = false;
#endif
	uint8_t * temp = payload;
    LoRaMacHeader_t macHdr;
    LoRaMacFrameCtrl_t fCtrl;
    ApplyCFListParams_t applyCFList;
    GetPhyParams_t getPhy;
    PhyParam_t phyParam;

    uint8_t pktHeaderLen = 0;
    uint32_t address = 0;
    uint8_t appPayloadStartIndex = 0;
    uint8_t port = 0xFF;
    uint8_t frameLen = 0;
    uint32_t mic = 0;
    uint32_t micRx = 0;

    uint16_t sequenceCounter = 0;
    uint16_t sequenceCounterPrev = 0;
    uint16_t sequenceCounterDiff = 0;
    uint32_t downLinkCounter = 0;

    MulticastParams_t *curMulticastParams = NULL;
    uint8_t *nwkSKey = LoRaMacNwkSKey;
    uint8_t *appSKey = LoRaMacAppSKey;

    uint8_t multicast = 0;

    bool isMicOk = false;

    McpsConfirm.AckReceived = false;
#ifdef CONFIG_LWAN  
    MlmeConfirm.Rssi = rssi;
    MlmeConfirm.Snr = snr;
    McpsIndication.DevTimeAnsReceived = false;
    McpsIndication.LinkCheckAnsReceived = false;
    McpsIndication.UplinkNeeded = false;
#endif    
    McpsIndication.Rssi = rssi;
    McpsIndication.Snr = snr;
    McpsIndication.RxSlot = RxSlot;
    McpsIndication.Port = 0;
    McpsIndication.Multicast = 0;
    McpsIndication.FramePending = 0;
    McpsIndication.Buffer = NULL;
    McpsIndication.BufferSize = 0;
    McpsIndication.RxData = false;
    McpsIndication.AckReceived = false;
    McpsIndication.DownLinkCounter = 0;
    McpsIndication.McpsIndication = MCPS_UNCONFIRMED;

    Radio.Sleep( );
    TimerStop( &RxWindowTimer2 );
    
    // This function must be called even if we are not in class b mode yet.
    if( LoRaMacClassBRxBeacon( payload, size ) == true )
    {
        MlmeIndication.BeaconInfo.Rssi = rssi;
        MlmeIndication.BeaconInfo.Snr = snr;
#ifdef LORAMAC_CLASSB_TESTCASE
        DBG_PRINTF("receive beacon\r\n");
#endif
        return;
    }
    // Check if we expect a ping or a multicast slot.
    if( LoRaMacDeviceClass == CLASS_B )
    {
        if( LoRaMacClassBIsPingExpected( ) == true )
        {
            LoRaMacClassBSetPingSlotState( PINGSLOT_STATE_SET_TIMER );
            LoRaMacClassBPingSlotTimerEvent( );
            McpsIndication.RxSlot = RX_SLOT_WIN_PING_SLOT;
        }
        else if( LoRaMacClassBIsMulticastExpected( ) == true )
        {
            LoRaMacClassBSetMulticastSlotState( PINGSLOT_STATE_SET_TIMER );
            LoRaMacClassBMulticastSlotTimerEvent( );
            McpsIndication.RxSlot = RX_SLOT_WIN_MULTICAST_SLOT;
        }
    }

    macHdr.Value = payload[pktHeaderLen++];
    switch ( macHdr.Bits.MType ) {
        case FRAME_TYPE_JOIN_ACCEPT:
            if ( IsLoRaMacNetworkJoined == true ) {
                McpsIndication.Status = LORAMAC_EVENT_INFO_STATUS_ERROR;
                PrepareRxDoneAbort( );
                return;
            }
            LoRaMacJoinDecrypt( payload + 1, size - 1, LoRaMacAppKey, LoRaMacRxPayload + 1 );

            LoRaMacRxPayload[0] = macHdr.Value;

            LoRaMacJoinComputeMic( LoRaMacRxPayload, size - LORAMAC_MFR_LEN, LoRaMacAppKey, &mic );

            micRx |= ( uint32_t )LoRaMacRxPayload[size - LORAMAC_MFR_LEN];
            micRx |= ( ( uint32_t )LoRaMacRxPayload[size - LORAMAC_MFR_LEN + 1] << 8 );
            micRx |= ( ( uint32_t )LoRaMacRxPayload[size - LORAMAC_MFR_LEN + 2] << 16 );
            micRx |= ( ( uint32_t )LoRaMacRxPayload[size - LORAMAC_MFR_LEN + 3] << 24 );
            if( LoRaMacConfirmQueueIsCmdActive( MLME_JOIN ) == true )
            {
                if( micRx == mic ) {
                    LoRaMacJoinComputeSKeys( LoRaMacAppKey, LoRaMacRxPayload + 1, LoRaMacDevNonce, LoRaMacNwkSKey, LoRaMacAppSKey );

                    LoRaMacNetID = ( uint32_t )LoRaMacRxPayload[4];
                    LoRaMacNetID |= ( ( uint32_t )LoRaMacRxPayload[5] << 8 );
                    LoRaMacNetID |= ( ( uint32_t )LoRaMacRxPayload[6] << 16 );

                    LoRaMacDevAddr = ( uint32_t )LoRaMacRxPayload[7];
                    LoRaMacDevAddr |= ( ( uint32_t )LoRaMacRxPayload[8] << 8 );
                    LoRaMacDevAddr |= ( ( uint32_t )LoRaMacRxPayload[9] << 16 );
                    LoRaMacDevAddr |= ( ( uint32_t )LoRaMacRxPayload[10] << 24 );

                    // DLSettings
                    LoRaMacParams.Rx1DrOffset = ( LoRaMacRxPayload[11] >> 4 ) & 0x07;
                    LoRaMacParams.Rx2Channel.Datarate = LoRaMacRxPayload[11] & 0x0F;

                    // RxDelay
                    LoRaMacParams.ReceiveDelay1 = ( LoRaMacRxPayload[12] & 0x0F );
                    if( LoRaMacParams.ReceiveDelay1 == 0 ) {
                        LoRaMacParams.ReceiveDelay1 = 1;
                    }
                    LoRaMacParams.ReceiveDelay1 *= 1000;
                    LoRaMacParams.ReceiveDelay2 = LoRaMacParams.ReceiveDelay1 + 1000;
#ifdef CONFIG_LORA_VERIFY
                	if (g_lora_debug) {
                    	PRINTF_RAW("Rx1DrOffset:%u Rx2Channel.Datarate:%u ReceiveDelay1:%u\r\n",
                            	LoRaMacParams.Rx1DrOffset, LoRaMacParams.Rx2Channel.Datarate, (unsigned int)LoRaMacParams.ReceiveDelay1);
                	}
#endif
                    // Apply CF list
                    applyCFList.Payload = &LoRaMacRxPayload[13];
                    // Size of the regular payload is 12. Plus 1 byte MHDR and 4 bytes MIC
                    applyCFList.Size = size - 17;

                    RegionApplyCFList( LoRaMacRegion, &applyCFList );

                    LoRaMacConfirmQueueSetStatus( LORAMAC_EVENT_INFO_STATUS_OK, MLME_JOIN );
                    IsLoRaMacNetworkJoined = true;
#ifndef ESP_PLATFORM
                    saveNetInfo(temp, size);
#endif
                	//Joined save its DR using LoRaMacParams.ChannelsDatarate, if set it will be default
                	LoRaMacParams.ChannelsDatarate = LoRaMacParamsDefaults.ChannelsDatarate;
                	//printf("LoRaMacParams.ChannelsDatarate %d\r\n",LoRaMacParams.ChannelsDatarate);
            	} else {
                    LoRaMacConfirmQueueSetStatus( LORAMAC_EVENT_INFO_STATUS_JOIN_FAIL, MLME_JOIN );
                }
            }
            break;
        case FRAME_TYPE_DATA_CONFIRMED_DOWN:
        case FRAME_TYPE_DATA_UNCONFIRMED_DOWN: {
            // Check if the received payload size is valid
            getPhy.UplinkDwellTime = LoRaMacParams.DownlinkDwellTime;
            getPhy.Datarate = McpsIndication.RxDatarate;
            McpsIndication.RxDoneDatarate = McpsIndication.RxDatarate;
            getPhy.Attribute = PHY_MAX_PAYLOAD;

            // Get the maximum payload length
            if( LoRaMacParams.RepeaterSupport == true ) {
                getPhy.Attribute = PHY_MAX_PAYLOAD_REPEATER;
            }
            phyParam = RegionGetPhyParam( LoRaMacRegion, &getPhy );
            if ( MAX( 0, ( int16_t )( ( int16_t )size - ( int16_t )LORA_MAC_FRMPAYLOAD_OVERHEAD ) ) > phyParam.Value ) {
                McpsIndication.Status = LORAMAC_EVENT_INFO_STATUS_ERROR;
                PrepareRxDoneAbort( );
                return;
            }

            address = payload[pktHeaderLen++];
            address |= ( (uint32_t)payload[pktHeaderLen++] << 8 );
            address |= ( (uint32_t)payload[pktHeaderLen++] << 16 );
            address |= ( (uint32_t)payload[pktHeaderLen++] << 24 );

            fCtrl.Value = payload[pktHeaderLen++];

            if ( address != LoRaMacDevAddr ) {
                curMulticastParams = MulticastChannels;
                while ( curMulticastParams != NULL ) {
                    if ( address == curMulticastParams->Address ) {
                        multicast = 1;
                        nwkSKey = curMulticastParams->NwkSKey;
                        appSKey = curMulticastParams->AppSKey;
                        downLinkCounter = curMulticastParams->DownLinkCounter;
                        break;
                    }
                    curMulticastParams = curMulticastParams->Next;
                }
                if ( multicast == 0 ) {
                    // We are not the destination of this frame.
                    McpsIndication.Status = LORAMAC_EVENT_INFO_STATUS_ADDRESS_FAIL;
                    PrepareRxDoneAbort( );
                    return;
                }
                if( ( macHdr.Bits.MType != FRAME_TYPE_DATA_UNCONFIRMED_DOWN ) ||
                        ( fCtrl.Bits.Ack == 1 ) ||
                        ( fCtrl.Bits.AdrAckReq == 1 ) ) {
                    // Wrong multicast message format. Refer to chapter 11.2.2 of the specification
                    McpsIndication.Status = LORAMAC_EVENT_INFO_STATUS_MULTICAST_FAIL;
                    PrepareRxDoneAbort( );
                    return;
                }
            } else {
                multicast = 0;
                nwkSKey = LoRaMacNwkSKey;
                appSKey = LoRaMacAppSKey;
                downLinkCounter = DownLinkCounter;
            }

            sequenceCounter = ( uint16_t )payload[pktHeaderLen++];
            sequenceCounter |= ( uint16_t )payload[pktHeaderLen++] << 8;

            appPayloadStartIndex = 8 + fCtrl.Bits.FOptsLen;
            micRx |= ( uint32_t )payload[size - LORAMAC_MFR_LEN];
            micRx |= ( ( uint32_t )payload[size - LORAMAC_MFR_LEN + 1] << 8 );
            micRx |= ( ( uint32_t )payload[size - LORAMAC_MFR_LEN + 2] << 16 );
            micRx |= ( ( uint32_t )payload[size - LORAMAC_MFR_LEN + 3] << 24 );

            sequenceCounterPrev = ( uint16_t )downLinkCounter;
            sequenceCounterDiff = ( sequenceCounter - sequenceCounterPrev );

            if ( sequenceCounterDiff < ( 1 << 15 ) ) {
                downLinkCounter += sequenceCounterDiff;
                LoRaMacComputeMic( payload, size - LORAMAC_MFR_LEN, nwkSKey, address, DOWN_LINK, downLinkCounter, &mic );
                if ( micRx == mic ) {
                    isMicOk = true;
                }
            } else {
                // check for sequence roll-over
                uint32_t  downLinkCounterTmp = downLinkCounter + 0x10000 + ( int16_t )sequenceCounterDiff;
                LoRaMacComputeMic( payload, size - LORAMAC_MFR_LEN, nwkSKey, address, DOWN_LINK, downLinkCounterTmp, &mic );
                if ( micRx == mic ) {
                    isMicOk = true;
                    downLinkCounter = downLinkCounterTmp;
                }
            }

            // Check for a the maximum allowed counter difference
            getPhy.Attribute = PHY_MAX_FCNT_GAP;
            phyParam = RegionGetPhyParam( LoRaMacRegion, &getPhy );
            if ( sequenceCounterDiff >= phyParam.Value ) {
                McpsIndication.Status = LORAMAC_EVENT_INFO_STATUS_DOWNLINK_TOO_MANY_FRAMES_LOSS;
                McpsIndication.DownLinkCounter = downLinkCounter;
                PrepareRxDoneAbort( );
                return;
            }

            if ( isMicOk == true ) {
                McpsIndication.Status = LORAMAC_EVENT_INFO_STATUS_OK;
                McpsIndication.Multicast = multicast;
                McpsIndication.FramePending = fCtrl.Bits.FPending;
                McpsIndication.Buffer = NULL;
                McpsIndication.BufferSize = 0;
                McpsIndication.DownLinkCounter = downLinkCounter;
                McpsConfirm.Status = LORAMAC_EVENT_INFO_STATUS_OK;

                AdrAckCounter = 0;
                MacCommandsBufferToRepeatIndex = 0;

                // Update 32 bits downlink counter
                if ( multicast == 1 ) {
                    McpsIndication.McpsIndication = MCPS_MULTICAST;

                    if ( ( curMulticastParams->DownLinkCounter == downLinkCounter ) &&
                         ( curMulticastParams->DownLinkCounter != 0 ) ) {
                        McpsIndication.Status = LORAMAC_EVENT_INFO_STATUS_DOWNLINK_REPEATED;
                        McpsIndication.DownLinkCounter = downLinkCounter;
                        PrepareRxDoneAbort( );
                        return;
                    }
                    curMulticastParams->DownLinkCounter = downLinkCounter;
                } else {
                    if ( macHdr.Bits.MType == FRAME_TYPE_DATA_CONFIRMED_DOWN ) {
                        SrvAckRequested = true; 
                        McpsIndication.McpsIndication = MCPS_CONFIRMED;

                        if ( ( DownLinkCounter == downLinkCounter ) &&
                             ( DownLinkCounter != 0 ) ) {
                            // Duplicated confirmed downlink. Skip indication.
                            // In this case, the MAC layer shall accept the MAC commands
                            // which are included in the downlink retransmission.
                            // It should not provide the same frame to the application
                            // layer again.
                            LoRaMacFlags.Bits.McpsIndSkip = 1;
                        }
                    } else {
                        SrvAckRequested = false;
                        McpsIndication.McpsIndication = MCPS_UNCONFIRMED;

                        if ( ( DownLinkCounter == downLinkCounter ) &&
                             ( DownLinkCounter != 0 ) ) {
                            McpsIndication.Status = LORAMAC_EVENT_INFO_STATUS_DOWNLINK_REPEATED;
                            McpsIndication.DownLinkCounter = downLinkCounter;
                            PrepareRxDoneAbort( );
                            return;
                        }
                    }
                    DownLinkCounter = downLinkCounter;
#ifndef ESP_PLATFORM
                    if(DownLinkCounter%15000==0){
                    	saveDownCnt();
                    }
#endif
                }

                // This must be done before parsing the payload and the MAC commands.
                // We need to reset the MacCommandsBufferIndex here, since we need
                // to take retransmissions and repetitions into account. Error cases
                // will be handled in function OnMacStateCheckTimerEvent.
                if ( McpsConfirm.McpsRequest == MCPS_CONFIRMED ) {
                    if ( fCtrl.Bits.Ack == 1 ) {
                        // Reset MacCommandsBufferIndex when we have received an ACK.
                        MacCommandsBufferIndex = 0;
                        // Update acknowledgement information
                        McpsConfirm.AckReceived = fCtrl.Bits.Ack;
                        McpsIndication.AckReceived = fCtrl.Bits.Ack;
                    }
                } else {
                    // Reset the variable if we have received any valid frame.
                    MacCommandsBufferIndex = 0;
                }
                port = payload[appPayloadStartIndex];
                // Process payload and MAC commands
                if ( ( ( size - 4 ) - appPayloadStartIndex ) > 0 ) {
                    port = payload[appPayloadStartIndex++];
                    frameLen = ( size - 4 ) - appPayloadStartIndex;

                    McpsIndication.Port = port;
                    if ( port == 0 ) {
                        // Only allow frames which do not have fOpts
                            if( ( fCtrl.Bits.FOptsLen == 0 ) && ( multicast == 0 ) ) {
                            LoRaMacPayloadDecrypt( payload + appPayloadStartIndex,
                                                   frameLen,
                                                   nwkSKey,
                                                   address,
                                                   DOWN_LINK,
                                                   downLinkCounter,
                                                   LoRaMacRxPayload );
                            // Decode frame payload MAC commands
                                ProcessMacCommands( LoRaMacRxPayload, 0, frameLen, snr, McpsIndication.RxSlot );
                        } else {
                            LoRaMacFlags.Bits.McpsIndSkip = 1;
                            // This is not a valid frame. Drop it and reset the ACK bits
                            McpsConfirm.AckReceived = false;
                            McpsIndication.AckReceived = false;
                            #ifdef CONFIG_LORA_VERIFY
                            if (g_lora_debug)
                                PRINTF_RAW("CMD exist at FRMpayload and Fopts, ignore it\r\n");
                            #endif
                        }
                    } else {
                        if( ( fCtrl.Bits.FOptsLen > 0 ) && ( multicast == 0 ) ){
                            // Decode Options field MAC commands. Omit the fPort.
                                ProcessMacCommands( payload, 8, appPayloadStartIndex - 1, snr, McpsIndication.RxSlot );
                        }

                        LoRaMacPayloadDecrypt( payload + appPayloadStartIndex,
                                               frameLen,
                                               appSKey,
                                               address,
                                               DOWN_LINK,
                                               downLinkCounter,
                                               LoRaMacRxPayload );

                        McpsIndication.Buffer = LoRaMacRxPayload;
                        McpsIndication.BufferSize = frameLen;
                        McpsIndication.RxData = true;
                    }
                } else {
                    if ( fCtrl.Bits.FOptsLen > 0 ) {
                        // Decode Options field MAC commands
                        ProcessMacCommands( payload, 8, appPayloadStartIndex, snr , McpsIndication.RxSlot);
                    }
                }

                // Provide always an indication, skip the callback to the user application,
                // in case of a confirmed downlink retransmission.
                LoRaMacFlags.Bits.McpsInd = 1;
            } else {
#ifdef CONFIG_LORA_VERIFY
                if (g_lora_debug) {
                    PRINTF_RAW("MIC verify failed ignore the frame\r\n");
                }
#endif
                McpsIndication.Status = LORAMAC_EVENT_INFO_STATUS_MIC_FAIL;

                PrepareRxDoneAbort( );
                return;
            }
        }
        break;
        case FRAME_TYPE_PROPRIETARY: {
            memcpy1( LoRaMacRxPayload, &payload[pktHeaderLen], size );

            McpsIndication.McpsIndication = MCPS_PROPRIETARY;
            McpsIndication.Status = LORAMAC_EVENT_INFO_STATUS_OK;
            McpsIndication.Buffer = LoRaMacRxPayload;
            McpsIndication.BufferSize = size - pktHeaderLen;

            LoRaMacFlags.Bits.McpsInd = 1;
            break;
        }
        default:
#ifdef CONFIG_LORA_VERIFY
            if (g_lora_debug)
                PRINTF_RAW("Download frame %d being received but not process it\r\n", macHdr.Bits.MType);
#endif
            McpsIndication.Status = LORAMAC_EVENT_INFO_STATUS_ERROR;
            PrepareRxDoneAbort( );
            break;
    }
    // Verify if we need to disable the AckTimeoutTimer
    CheckToDisableAckTimeout( NodeAckRequested, LoRaMacDeviceClass, McpsConfirm.AckReceived,
                                AckTimeoutRetriesCounter, AckTimeoutRetries );
    if( AckTimeoutTimer.IsRunning == false )
    {// Procedure is completed when the AckTimeoutTimer is not running anymore
    	LoRaMacFlags.Bits.MacDone = 1;
    	// Trig OnMacCheckTimerEvent call as soon as possible
    	TimerSetValue( &MacStateCheckTimer, 1 );
    	TimerStart( &MacStateCheckTimer );
    	//OnMacStateCheckTimerEvent();
    }
}

static void OnRadioTxTimeout( void )
{
    DIO_PRINTF("Event : Tx Timeout\r\n");
    lora_txing=false;
    Radio.Init( &RadioEvents );
    // Random seed initialization
    srand1( Radio.Random( ) );

    PublicNetwork = true;
    Radio.SetPublicNetwork(true);
    Radio.Sleep( );

    if( LoRaMacDeviceClass != CLASS_C )
    {
        Radio.Sleep( );
    }
    else
    {
        OpenContinuousRx2Window( );
    }

    McpsConfirm.Status = LORAMAC_EVENT_INFO_STATUS_TX_TIMEOUT;
    LoRaMacConfirmQueueSetStatusCmn( LORAMAC_EVENT_INFO_STATUS_TX_TIMEOUT );
    LoRaMacFlags.Bits.MacDone = 1;
#ifdef CONFIG_LWAN
    lwan_dev_status_set(DEVICE_STATUS_SEND_FAIL);
#endif
#if (LoraWan_RGB==1)
			turnOffRGB();
#endif

}

static void OnRadioRxError( void )
{
	DIO_PRINTF("Event : Rx Error\r\n");
#ifdef CLASS_A_WOTA
	if(wota_CadStarted)
		wota_CadStarted = false;
#endif
    bool classBRx = false;

    if( LoRaMacDeviceClass != CLASS_C )
    {
        lora_txing=false;
        Radio.Sleep( );
    }

    if( LoRaMacClassBIsBeaconExpected( ) == true )
    {
        LoRaMacClassBSetBeaconState( BEACON_STATE_TIMEOUT );
        LoRaMacClassBBeaconTimerEvent( );
        classBRx = true;
    }
    if( LoRaMacDeviceClass == CLASS_B )
    {
        if( LoRaMacClassBIsPingExpected( ) == true )
        {
            LoRaMacClassBSetPingSlotState( PINGSLOT_STATE_SET_TIMER );
            LoRaMacClassBPingSlotTimerEvent( );
            classBRx = true;
        }
        if( LoRaMacClassBIsMulticastExpected( ) == true )
        {
            LoRaMacClassBSetMulticastSlotState( PINGSLOT_STATE_SET_TIMER );
            LoRaMacClassBMulticastSlotTimerEvent( );
            classBRx = true;
        }
    }

    if( classBRx == false )
    {
        if( RxSlot == RX_SLOT_WIN_1 )
        {
            if( NodeAckRequested == true )
            {
                McpsConfirm.Status = LORAMAC_EVENT_INFO_STATUS_RX1_ERROR;
            }
            LoRaMacConfirmQueueSetStatusCmn( LORAMAC_EVENT_INFO_STATUS_RX1_ERROR );

            if( LoRaMacDeviceClass != CLASS_C )
            {
                if( TimerGetElapsedTime( AggregatedLastTxDoneTime ) >= RxWindow2Delay )
                {
                    TimerStop( &RxWindowTimer2 );
                    LoRaMacFlags.Bits.MacDone = 1;
                }
            }
        }
        else
        {
            if( NodeAckRequested == true )
            {
                McpsConfirm.Status = LORAMAC_EVENT_INFO_STATUS_RX2_ERROR;
            }
            LoRaMacConfirmQueueSetStatusCmn( LORAMAC_EVENT_INFO_STATUS_RX2_ERROR );

            if( LoRaMacDeviceClass != CLASS_C )
            {
                LoRaMacFlags.Bits.MacDone = 1;
            }
        }
    }

    if( LoRaMacDeviceClass == CLASS_C )
    {
        OpenContinuousRx2Window( );
    }
#if (LoraWan_RGB==1)
        turnOffRGB();
#endif

}

static void OnRadioRxTimeout( void )
{
    DIO_PRINTF("Event : Rx Timeout\r\n");

#ifdef CLASS_A_WOTA
	if(wota_CadStarted)
		wota_CadStarted = false;
#endif

    bool classBRx = false;

    if( LoRaMacDeviceClass != CLASS_C )
    {
        lora_txing=false;
        Radio.Sleep( );
    }

    if( LoRaMacClassBIsBeaconExpected( ) == true )
    {
        LoRaMacClassBSetBeaconState( BEACON_STATE_TIMEOUT );
        LoRaMacClassBBeaconTimerEvent( );
        classBRx = true;
    }
    if( LoRaMacDeviceClass == CLASS_B )
    {
        if( LoRaMacClassBIsPingExpected( ) == true )
        {
            LoRaMacClassBSetPingSlotState( PINGSLOT_STATE_SET_TIMER );
            LoRaMacClassBPingSlotTimerEvent( );
            classBRx = true;
        }
        if( LoRaMacClassBIsMulticastExpected( ) == true )
        {
            LoRaMacClassBSetMulticastSlotState( PINGSLOT_STATE_SET_TIMER );
            LoRaMacClassBMulticastSlotTimerEvent( );
            classBRx = true;
        }
    }

    if( classBRx == false )
    {
        if( RxSlot == RX_SLOT_WIN_1 )
        {
            if( NodeAckRequested == true )
            {
                McpsConfirm.Status = LORAMAC_EVENT_INFO_STATUS_RX1_TIMEOUT;
            }
            LoRaMacConfirmQueueSetStatusCmn( LORAMAC_EVENT_INFO_STATUS_RX1_TIMEOUT );

            if( LoRaMacDeviceClass != CLASS_C )
            {
                if( TimerGetElapsedTime( AggregatedLastTxDoneTime ) >= RxWindow2Delay )
                {
                    TimerStop( &RxWindowTimer2 );
                    LoRaMacFlags.Bits.MacDone = 1;
                }
            }
        }
        else
        {
            if( NodeAckRequested == true )
            {
                McpsConfirm.Status = LORAMAC_EVENT_INFO_STATUS_RX2_TIMEOUT;
            }
            LoRaMacConfirmQueueSetStatusCmn( LORAMAC_EVENT_INFO_STATUS_RX2_TIMEOUT );

            if( LoRaMacDeviceClass != CLASS_C )
            {
                LoRaMacFlags.Bits.MacDone = 1;
            }
        }
    }

    if( LoRaMacDeviceClass == CLASS_C )
    {
        OpenContinuousRx2Window( );
    }
#if(LoraWan_RGB==1)
			turnOffRGB();
#endif
}

static void OnRadioCadDone( bool channelActivityDetected )
{
	LoRaMacState &= ~LORAMAC_CAD_RUNNING;
#ifdef CLASS_A_WOTA
	if(wota_CadStarted)
	{
		wota_CadDone( channelActivityDetected );
		return;
	}
#endif
#ifdef CONFIG_LORA_CAD
	beforeTxCadDone( channelActivityDetected );
#endif
}

static void OnMacStateCheckTimerEvent( void )
{
    GetPhyParams_t getPhy;
    PhyParam_t phyParam;
    bool noTx = false;

    TimerStop( &MacStateCheckTimer );

    if ( LoRaMacFlags.Bits.MacDone == 1 ) {
#ifdef CLASS_A_WOTA
		wota_CadEnable = true;
#endif
        if ( ( LoRaMacState & LORAMAC_RX_ABORT ) == LORAMAC_RX_ABORT ) {
            LoRaMacState &= ~LORAMAC_RX_ABORT;
            LoRaMacState &= ~LORAMAC_TX_RUNNING;
        }

        if ( ( LoRaMacFlags.Bits.MlmeReq == 1 ) || ( ( LoRaMacFlags.Bits.McpsReq == 1 ) ) ) {
            // Get a status of any request and check if we have a TX timeout
            MlmeConfirm.Status = LoRaMacConfirmQueueGetStatusCmn( );
            if ( ( McpsConfirm.Status == LORAMAC_EVENT_INFO_STATUS_TX_TIMEOUT ) ||
                 ( MlmeConfirm.Status == LORAMAC_EVENT_INFO_STATUS_TX_TIMEOUT ) ) {
                // Stop transmit cycle due to tx timeout.
                LoRaMacState &= ~LORAMAC_TX_RUNNING;
                MacCommandsBufferIndex = 0;
                McpsConfirm.NbRetries = AckTimeoutRetriesCounter;
                McpsConfirm.AckReceived = false;
                McpsConfirm.TxTimeOnAir = 0;
                noTx = true;
            }

            if( ( LoRaMacConfirmQueueIsCmdActive( MLME_BEACON_ACQUISITION ) == true ) &&
                ( LoRaMacFlags.Bits.McpsReq == 0 ) )
            {
                if( LoRaMacFlags.Bits.MlmeReq == 1 )
                {
                    noTx = true;
                    LoRaMacState &= ~LORAMAC_TX_RUNNING;
                }
            }
        }

        if ( ( NodeAckRequested == false ) && ( noTx == false ) ) {
            if ( ( LoRaMacFlags.Bits.MlmeReq == 1 ) || ( ( LoRaMacFlags.Bits.McpsReq == 1 ) ) ) {
                if ( ( LoRaMacFlags.Bits.MlmeReq == 1 ) && ( LoRaMacConfirmQueueIsCmdActive( MLME_JOIN ) == true ) ) {
                    // Procedure for the join request
                    MlmeConfirm.NbRetries = JoinRequestTrials;

                    if( LoRaMacConfirmQueueGetStatus( MLME_JOIN ) == LORAMAC_EVENT_INFO_STATUS_OK ) {
                        // Node joined successfully
                        UpLinkCounter = 0;
                        #ifdef CONFIG_LORA_VERIFY
                        if (g_lora_debug)
                            PRINTF_RAW("Join done, UpLinkCounter:%u\r\n", (unsigned int)UpLinkCounter);
                        #endif
                        ChannelsNbRepCounter = 0;
                        LoRaMacState &= ~LORAMAC_TX_RUNNING;
                    } else {
                        if ( JoinRequestTrials >= MaxJoinRequestTrials ) {
                            LoRaMacState &= ~LORAMAC_TX_RUNNING;
                        } else {
                            LoRaMacFlags.Bits.MacDone = 0;
                            // Sends the same frame again
                            OnTxDelayedTimerEvent( );
                        }
                    }
                } else {
                    // Procedure for all other frames
                    if ( ( ChannelsNbRepCounter >= LoRaMacParams.ChannelsNbRep ) || ( LoRaMacFlags.Bits.McpsInd == 1 ) ) {
                        if ( LoRaMacFlags.Bits.McpsInd == 0 ) {
                            // Maximum repetitions without downlink. Reset MacCommandsBufferIndex. Increase ADR Ack counter.
                            // Only process the case when the MAC did not receive a downlink.
                            MacCommandsBufferIndex = 0;
                            AdrAckCounter++;
                        }

                        ChannelsNbRepCounter = 0;

                        if ( IsUpLinkCounterFixed == false ) {
                            UpLinkCounter++;
                            //SaveUpCnt();
                            #ifdef CONFIG_LORA_VERIFY
                            if (g_lora_debug)
                                PRINTF_RAW("Unconfirmed data, UpLinkCounter:%u\r\n", (unsigned int)UpLinkCounter);
                            #endif
                        }

                        LoRaMacState &= ~LORAMAC_TX_RUNNING;
                    } else {
                        LoRaMacFlags.Bits.MacDone = 0;
                        // Sends the same frame again
                        OnTxDelayedTimerEvent( );
                    }
                }
            }
        }

        if ( LoRaMacFlags.Bits.McpsInd == 1 ) {
            // Procedure if we received a frame
            if ( ( McpsConfirm.AckReceived == true ) || ( AckTimeoutRetriesCounter > AckTimeoutRetries ) ) {
                AckTimeoutRetry = false;
                NodeAckRequested = false;
                if ( IsUpLinkCounterFixed == false ) {
                    UpLinkCounter++;
                    //SaveUpCnt();
                #ifdef CONFIG_LORA_VERIFY
                if (g_lora_debug)
                    PRINTF_RAW("Confirmed data received ACK, UpLinkCounter:%u\r\n", (unsigned int)UpLinkCounter);
                #endif
                }
                McpsConfirm.NbRetries = AckTimeoutRetriesCounter;

                LoRaMacState &= ~LORAMAC_TX_RUNNING;
            }
#ifdef CONFIG_LWAN
        } else {
            if ( !(( LoRaMacFlags.Bits.MlmeReq == 1 ) && ( LoRaMacConfirmQueueIsCmdActive( MLME_JOIN ) == true )) )
                lwan_dev_status_set(DEVICE_STATUS_SEND_PASS_WITHOUT_DL);
        }
#else
        }
#endif	
        if ( ( AckTimeoutRetry == true ) && ( ( LoRaMacState & LORAMAC_TX_DELAYED ) == 0 ) ) {
            // Retransmissions procedure for confirmed uplinks
            AckTimeoutRetry = false;
            if ( ( AckTimeoutRetriesCounter < AckTimeoutRetries ) && ( AckTimeoutRetriesCounter <= MAX_ACK_RETRIES ) ) {
                AckTimeoutRetriesCounter++;

                if ( ( AckTimeoutRetriesCounter % 2 ) == 1 ) {
                    getPhy.Attribute = PHY_NEXT_LOWER_TX_DR;
                    getPhy.UplinkDwellTime = LoRaMacParams.UplinkDwellTime;
                    getPhy.Datarate = LoRaMacParams.ChannelsDatarate;
                    phyParam = RegionGetPhyParam( LoRaMacRegion, &getPhy );
                    LoRaMacParams.ChannelsDatarate = phyParam.Value;
                }
                // Try to send the frame again
                if ( ScheduleTx( ) == LORAMAC_STATUS_OK ) {
                    LoRaMacFlags.Bits.MacDone = 0;
                } else {
                    // The DR is not applicable for the payload size
                    McpsConfirm.Status = LORAMAC_EVENT_INFO_STATUS_TX_DR_PAYLOAD_SIZE_ERROR;

                    MacCommandsBufferIndex = 0;
                    LoRaMacState &= ~LORAMAC_TX_RUNNING;
                    NodeAckRequested = false;
                    McpsConfirm.AckReceived = false;
                    McpsConfirm.NbRetries = AckTimeoutRetriesCounter;
                    McpsConfirm.Datarate = LoRaMacParams.ChannelsDatarate;
                    if ( IsUpLinkCounterFixed == false ) {
                        UpLinkCounter++;
                        //SaveUpCnt();
                        #ifdef CONFIG_LORA_VERIFY
                        if (g_lora_debug)
                            PRINTF_RAW("Confirmed data can't send after decrease DR, UpLinkCounter:%u\r\n", (unsigned int)UpLinkCounter);
                        #endif
                    }
                }
            } else {
                RegionInitDefaults( LoRaMacRegion, INIT_TYPE_RESTORE );

                LoRaMacState &= ~LORAMAC_TX_RUNNING;

                MacCommandsBufferIndex = 0;
                NodeAckRequested = false;
                McpsConfirm.AckReceived = false;
                McpsConfirm.NbRetries = AckTimeoutRetriesCounter;
                if ( IsUpLinkCounterFixed == false ) {
                    UpLinkCounter++;
                    #ifdef CONFIG_LORA_VERIFY
                    if (g_lora_debug)
                        PRINTF_RAW("Confirmed data exceed retry times, UpLinkCounter:%u\r\n", (unsigned int)UpLinkCounter);
                    #endif
                }
            }
        }
    }

    // Handle events
    if( LoRaMacState == LORAMAC_IDLE )
    {
        if( LoRaMacFlags.Bits.McpsReq == 1 )
        {
            LoRaMacFlags.Bits.McpsReq = 0;
            LoRaMacPrimitives->MacMcpsConfirm( &McpsConfirm );
        }

        if( LoRaMacFlags.Bits.MlmeReq == 1 )
        {
            LoRaMacFlags.Bits.MlmeReq = 0;
            LoRaMacConfirmQueueHandleCb( &MlmeConfirm );
            if( LoRaMacConfirmQueueGetCnt( ) > 0 )
            {
                LoRaMacFlags.Bits.MlmeReq = 1;
            }
        }

        // Handle MLME indication
        if( LoRaMacFlags.Bits.MlmeInd == 1 )
        {
            LoRaMacPrimitives->MacMlmeIndication( &MlmeIndication );
            LoRaMacFlags.Bits.MlmeInd = 0;

#ifdef CONFIG_LWAN
            if(MlmeIndication.MlmeIndication == MLME_SCHEDULE_UPLINK) {
                McpsIndication.UplinkNeeded = false;
            }
#endif    
        }
        // Verify if sticky MAC commands are pending or not
        if( IsStickyMacCommandPending( ) == true )
        {// Setup MLME indication
            SetMlmeScheduleUplinkIndication( );
            LoRaMacPrimitives->MacMlmeIndication( &MlmeIndication );
            LoRaMacFlags.Bits.MlmeInd = 0;
#ifdef CONFIG_LWAN
            if(MlmeIndication.MlmeIndication == MLME_SCHEDULE_UPLINK) {
                McpsIndication.UplinkNeeded = false;
            }
#endif            
        }

        // Procedure done. Reset variables.
        LoRaMacFlags.Bits.MacDone = 0;

        LoRaMacClassBResumeBeaconing( );
    }
    else
    {
        // Operation not finished restart timer
        TimerSetValue( &MacStateCheckTimer, MAC_STATE_CHECK_TIMEOUT );
        TimerStart( &MacStateCheckTimer );
    }

    // Handle MCPS indication
    if( LoRaMacFlags.Bits.McpsInd == 1 )
    {
        LoRaMacFlags.Bits.McpsInd = 0;
        if( LoRaMacDeviceClass == CLASS_C )
        {// Activate RX2 window for Class C
            OpenContinuousRx2Window( );
        }
        if( LoRaMacFlags.Bits.McpsIndSkip == 0 )
        {
            LoRaMacPrimitives->MacMcpsIndication( &McpsIndication );
        }
        LoRaMacFlags.Bits.McpsIndSkip = 0;
    }

}

static void OnTxDelayedTimerEvent( void )
{
    LoRaMacHeader_t macHdr;
    LoRaMacFrameCtrl_t fCtrl;
    AlternateDrParams_t altDr;

    TimerStop( &TxDelayedTimer );
    LoRaMacState &= ~LORAMAC_TX_DELAYED;

    if ( ( LoRaMacFlags.Bits.MlmeReq == 1 ) && ( LoRaMacConfirmQueueIsCmdActive( MLME_JOIN ) == true ) ) {
        ResetMacParameters( );

        altDr.NbTrials = JoinRequestTrials + 1;
#ifdef CONFIG_LINKWAN
        altDr.joinmethod = LoRaMacParams.method;
        altDr.datarate = LoRaMacParams.ChannelsDatarate;
#endif
        LoRaMacParams.ChannelsDatarate = RegionAlternateDr( LoRaMacRegion, &altDr );

        macHdr.Value = 0;
        macHdr.Bits.MType = FRAME_TYPE_JOIN_REQ;

        fCtrl.Value = 0;
        fCtrl.Bits.Adr = AdrCtrlOn;

        /* In case of join request retransmissions, the stack must prepare
         * the frame again, because the network server keeps track of the random
         * LoRaMacDevNonce values to prevent reply attacks. */
        PrepareFrame( &macHdr, &fCtrl, 0, NULL, 0 );
    }

    ScheduleTx( );
}

#ifdef CONFIG_LORA_CAD
static void OnTxImmediateTimerEvent( void )
{
    TimerStop( &TxImmediateTimer );
    SendFrameOnChannel( Channel );
}
#endif

static void OnRxWindow1TimerEvent( void )
{
    TimerStop( &RxWindowTimer1 );
    RxSlot = RX_SLOT_WIN_1;

    RxWindow1Config.Channel = Channel;
    RxWindow1Config.DrOffset = LoRaMacParams.Rx1DrOffset;
    RxWindow1Config.DownlinkDwellTime = LoRaMacParams.DownlinkDwellTime;
    RxWindow1Config.RepeaterSupport = LoRaMacParams.RepeaterSupport;
    RxWindow1Config.RxContinuous = false;
    RxWindow1Config.RxSlot = RxSlot;

    if ( LoRaMacDeviceClass == CLASS_C ) {
        Radio.Standby( );
    }
	
    RegionRxConfig( LoRaMacRegion, &RxWindow1Config, ( int8_t * )&McpsIndication.RxDatarate );
    //printf("w1 dr:%d\r\n",McpsIndication.RxDatarate);
    RxWindowSetup( RxWindow1Config.RxContinuous, LoRaMacParams.MaxRxWindow );
#if(LoraWan_RGB==1)
    turnOnRGB(COLOR_RXWINDOW1,0);
#endif
}

static void OnRxWindow2TimerEvent( void )
{
    TimerStop( &RxWindowTimer2 );

    RxWindow2Config.Channel = Channel;
    RxWindow2Config.Frequency = LoRaMacParams.Rx2Channel.Frequency;
    RxWindow2Config.DownlinkDwellTime = LoRaMacParams.DownlinkDwellTime;
    RxWindow2Config.RepeaterSupport = LoRaMacParams.RepeaterSupport;
    RxWindow2Config.RxSlot = RX_SLOT_WIN_2;

    if ( LoRaMacDeviceClass != CLASS_C ) {
        RxWindow2Config.RxContinuous = false;
    } else {
        RxWindow2Config.RxContinuous = true;
    }

    if ( RegionRxConfig( LoRaMacRegion, &RxWindow2Config, ( int8_t * )&McpsIndication.RxDatarate ) == true ) {
    	//printf("w2 dr:%d\r\n",McpsIndication.RxDatarate);
        RxWindowSetup( RxWindow2Config.RxContinuous, LoRaMacParams.MaxRxWindow );
        RxSlot = RX_SLOT_WIN_2;
    }
#if(LoraWan_RGB==1)
	turnOnRGB(COLOR_RXWINDOW2,0);
#endif

}

static void CheckToDisableAckTimeout( bool nodeAckRequested, DeviceClass_t devClass, bool ackReceived,
                                      uint8_t ackTimeoutRetriesCounter, uint8_t ackTimeoutRetries )
{
    // There are three cases where we need to stop the AckTimeoutTimer:
    if( nodeAckRequested == false )
    {
        if( devClass == CLASS_C )
        {// FIRST CASE
            // We have performed an unconfirmed uplink in class c mode
            // and have received a downlink in RX1 or RX2.
            TimerStop( &AckTimeoutTimer );
        }
    }
    else
    {
        if( ackReceived == 1 )
        {// SECOND CASE
            // We have performed a confirmed uplink and have received a
            // downlink with a valid ACK.
            TimerStop( &AckTimeoutTimer );
        }
        else
        {// THIRD CASE
            if( ackTimeoutRetriesCounter > ackTimeoutRetries )
            {
                // We have performed a confirmed uplink and have not
                // received a downlink with a valid ACK. In this case
                // we need to verify if the maximum retries have been
                // elapsed. If so, stop the timer.
                TimerStop( &AckTimeoutTimer );
            }
        }
    }
}

static void OnAckTimeoutTimerEvent( void )
{
    TimerStop( &AckTimeoutTimer );

    if ( NodeAckRequested == true ) {
        AckTimeoutRetry = true;
        LoRaMacState &= ~LORAMAC_ACK_REQ;
    }
    if ( LoRaMacDeviceClass == CLASS_C ) {
        LoRaMacFlags.Bits.MacDone = 1;
    }
}

static void RxWindowSetup( bool rxContinuous, uint32_t maxRxWindow )
{
    if ( rxContinuous == false ) {
        Radio.Rx( maxRxWindow );
    } else {
        Radio.Rx( 0 ); // Continuous mode
    }
}

static LoRaMacStatus_t SwitchClass( DeviceClass_t deviceClass )
{
    LoRaMacStatus_t status = LORAMAC_STATUS_PARAMETER_INVALID;

    switch( LoRaMacDeviceClass )
    {
        case CLASS_A:
        {
            if( deviceClass == CLASS_B )
            {
                status = LoRaMacClassBSwitchClass( deviceClass );
                if( status == LORAMAC_STATUS_OK )
                {
                    LoRaMacDeviceClass = deviceClass;
                }
            }

            if( deviceClass == CLASS_C )
            {
                LoRaMacDeviceClass = deviceClass;

                // Set the NodeAckRequested indicator to default
                NodeAckRequested = false;
                // Set the radio into sleep mode in case we are still in RX mode
                Radio.Sleep( );
                // Compute Rx2 windows parameters in case the RX2 datarate has changed
                RegionComputeRxWindowParameters( LoRaMacRegion,
                                                 LoRaMacParams.Rx2Channel.Datarate,
                                                 LoRaMacParams.MinRxSymbols,
                                                 LoRaMacParams.SystemMaxRxError,
                                                 &RxWindow2Config );
                OpenContinuousRx2Window( );

                status = LORAMAC_STATUS_OK;
            }
            break;
        }
        case CLASS_B:
        {
            status = LoRaMacClassBSwitchClass( deviceClass );
            if( status == LORAMAC_STATUS_OK )
            {
                LoRaMacDeviceClass = deviceClass;
            }
            break;
        }
        case CLASS_C:
        {
            if( deviceClass == CLASS_A )
            {
                LoRaMacDeviceClass = deviceClass;

                // Set the radio into sleep to setup a defined state
                Radio.Sleep( );

                status = LORAMAC_STATUS_OK;
            }
            break;
        }
    }

    return status;
}

static bool ValidatePayloadLength( uint8_t lenN, int8_t datarate, uint8_t fOptsLen )
{
    GetPhyParams_t getPhy;
    PhyParam_t phyParam;
    uint16_t maxN = 0;
    uint16_t payloadSize = 0;

    // Setup PHY request
    getPhy.UplinkDwellTime = LoRaMacParams.UplinkDwellTime;
    getPhy.Datarate = datarate;
    getPhy.Attribute = PHY_MAX_PAYLOAD;

    // Get the maximum payload length
    if( LoRaMacParams.RepeaterSupport == true )
    {
        getPhy.Attribute = PHY_MAX_PAYLOAD_REPEATER;
    }
    phyParam = RegionGetPhyParam( LoRaMacRegion, &getPhy );
    maxN = phyParam.Value;

    // Calculate the resulting payload size
    payloadSize = ( lenN + fOptsLen );

    // Validation of the application payload size
    //if ( ((( payloadSize > maxN ) && (fOptsLen != 0) && (fOptsLen <= maxN)) || ( payloadSize <= maxN )) && ( payloadSize <= LORAMAC_PHY_MAXPAYLOAD ) ) {
    if ( ((( payloadSize > maxN ) && (fOptsLen != 0) && (lenN <= maxN) && (fOptsLen <= maxN)) || ( payloadSize <= maxN )) && ( payloadSize <= LORAMAC_PHY_MAXPAYLOAD ) ) {
        return true;
    }
    return false;
}

static bool IsStickyMacCommandPending( void )
{
    if( MacCommandsBufferToRepeatIndex > 0 )
    {
        // Sticky MAC commands pending
        return true;
    }
    return false;
}

static void SetMlmeScheduleUplinkIndication( void )
{
    MlmeIndication.MlmeIndication = MLME_SCHEDULE_UPLINK;
    LoRaMacFlags.Bits.MlmeInd = 1;
}

static LoRaMacStatus_t AddMacCommand( uint8_t cmd, uint8_t p1, uint8_t p2 )
{
    LoRaMacStatus_t status = LORAMAC_STATUS_BUSY;
    // The maximum buffer length must take MAC commands to re-send into account.
    uint8_t bufLen = LORA_MAC_COMMAND_MAX_LENGTH - MacCommandsBufferToRepeatIndex;

    switch ( cmd ) {
        case MOTE_MAC_LINK_CHECK_REQ:
            if ( MacCommandsBufferIndex < bufLen ) {
                MacCommandsBuffer[MacCommandsBufferIndex++] = cmd;
                // No payload for this command
                status = LORAMAC_STATUS_OK;
            }
            break;
        case MOTE_MAC_LINK_ADR_ANS:
            if ( MacCommandsBufferIndex < ( bufLen - 1 ) ) {
                MacCommandsBuffer[MacCommandsBufferIndex++] = cmd;
                // Margin
                MacCommandsBuffer[MacCommandsBufferIndex++] = p1;
                status = LORAMAC_STATUS_OK;
            }
            break;
        case MOTE_MAC_DUTY_CYCLE_ANS:
            if ( MacCommandsBufferIndex < bufLen ) {
                MacCommandsBuffer[MacCommandsBufferIndex++] = cmd;
                // No payload for this answer
                status = LORAMAC_STATUS_OK;
            }
            break;
        case MOTE_MAC_RX_PARAM_SETUP_ANS:
            if ( MacCommandsBufferIndex < ( bufLen - 1 ) ) {
                MacCommandsBuffer[MacCommandsBufferIndex++] = cmd;
                // Status: Datarate ACK, Channel ACK
                MacCommandsBuffer[MacCommandsBufferIndex++] = p1;
                // This is a sticky MAC command answer. Setup indication
                SetMlmeScheduleUplinkIndication( );
                status = LORAMAC_STATUS_OK;
            }
            break;
        case MOTE_MAC_DEV_STATUS_ANS:
            if ( MacCommandsBufferIndex < ( bufLen - 2 ) ) {
                MacCommandsBuffer[MacCommandsBufferIndex++] = cmd;
                // 1st byte Battery
                // 2nd byte Margin
                MacCommandsBuffer[MacCommandsBufferIndex++] = p1;
                MacCommandsBuffer[MacCommandsBufferIndex++] = p2;
                SetMlmeScheduleUplinkIndication( );
                status = LORAMAC_STATUS_OK;
#ifdef LORAMAC_CLASSB_TESTCASE
                DBG_PRINTF("ready to send MOTE_MAC_DEV_STATUS_ANS p1=%d p2=%d\r\n",p1,p2);
#endif
            }
            break;
        case MOTE_MAC_NEW_CHANNEL_ANS:
            if ( MacCommandsBufferIndex < ( bufLen - 1 ) ) {
                MacCommandsBuffer[MacCommandsBufferIndex++] = cmd;
                // Status: Datarate range OK, Channel frequency OK
                MacCommandsBuffer[MacCommandsBufferIndex++] = p1;
                status = LORAMAC_STATUS_OK;
            }
            break;
        case MOTE_MAC_RX_TIMING_SETUP_ANS:
            if ( MacCommandsBufferIndex < bufLen ) {
                MacCommandsBuffer[MacCommandsBufferIndex++] = cmd;
                // No payload for this answer
                // This is a sticky MAC command answer. Setup indication
                SetMlmeScheduleUplinkIndication( );
                status = LORAMAC_STATUS_OK;
            }
            break;
        case MOTE_MAC_TX_PARAM_SETUP_ANS:
            if ( MacCommandsBufferIndex < bufLen ) {
                MacCommandsBuffer[MacCommandsBufferIndex++] = cmd;
                // No payload for this answer
                status = LORAMAC_STATUS_OK;
            }
            break;
        case MOTE_MAC_DL_CHANNEL_ANS:
            if ( MacCommandsBufferIndex < bufLen ) {
                MacCommandsBuffer[MacCommandsBufferIndex++] = cmd;
                // Status: Uplink frequency exists, Channel frequency OK
                MacCommandsBuffer[MacCommandsBufferIndex++] = p1;
                // This is a sticky MAC command answer. Setup indication
                SetMlmeScheduleUplinkIndication( );

                status = LORAMAC_STATUS_OK;
            }
            break;
        case MOTE_MAC_DEVICE_TIME_REQ:
            if( MacCommandsBufferIndex < LORA_MAC_COMMAND_MAX_LENGTH )
            {
                MacCommandsBuffer[MacCommandsBufferIndex++] = cmd;
                // No payload for this answer
               status = LORAMAC_STATUS_OK;
#ifdef LORAMAC_CLASSB_TESTCASE
                DBG_PRINTF("ready to send MOTE_MAC_DEVICE_TIME_REQ\r\n");
#endif
            }
            break;
        case MOTE_MAC_PING_SLOT_INFO_REQ:
            if( MacCommandsBufferIndex < ( LORA_MAC_COMMAND_MAX_LENGTH - 1 ) )
            {
                MacCommandsBuffer[MacCommandsBufferIndex++] = cmd;
                // Status: Periodicity and Datarate
                MacCommandsBuffer[MacCommandsBufferIndex++] = p1;
                status = LORAMAC_STATUS_OK;
#ifdef LORAMAC_CLASSB_TESTCASE
                DBG_PRINTF("ready to send MOTE_MAC_PING_SLOT_INFO_REQ value=%d\r\n",p1);
#endif
            }
            break;
        case MOTE_MAC_PING_SLOT_FREQ_ANS:
            if( MacCommandsBufferIndex < LORA_MAC_COMMAND_MAX_LENGTH )
            {
                MacCommandsBuffer[MacCommandsBufferIndex++] = cmd;
                // Status: Datarate range OK, Channel frequency OK
                MacCommandsBuffer[MacCommandsBufferIndex++] = p1;
                SetMlmeScheduleUplinkIndication( );
                status = LORAMAC_STATUS_OK;
            }
            break;
        case MOTE_MAC_BEACON_TIMING_REQ:
            if( MacCommandsBufferIndex < LORA_MAC_COMMAND_MAX_LENGTH )
            {
                MacCommandsBuffer[MacCommandsBufferIndex++] = cmd;
                // No payload for this answer
                status = LORAMAC_STATUS_OK;
            }
            break;
        case MOTE_MAC_BEACON_FREQ_ANS:
            if( MacCommandsBufferIndex < LORA_MAC_COMMAND_MAX_LENGTH )
            {
                MacCommandsBuffer[MacCommandsBufferIndex++] = cmd;
                // Status: Channel frequency OK
                MacCommandsBuffer[MacCommandsBufferIndex++] = p1;
                SetMlmeScheduleUplinkIndication( );
                status = LORAMAC_STATUS_OK;
            }
            break;
        default:
            return LORAMAC_STATUS_SERVICE_UNKNOWN;
    }
    if ( status == LORAMAC_STATUS_OK ) {
        MacCommandsInNextTx = true;
        if (SrvAckRequested) {
            SetMlmeScheduleUplinkIndication();
        }
    }
    return status;
}

static uint8_t ParseMacCommandsToRepeat( uint8_t *cmdBufIn, uint8_t length, uint8_t *cmdBufOut )
{
    uint8_t i = 0;
    uint8_t cmdCount = 0;

    if ( ( cmdBufIn == NULL ) || ( cmdBufOut == NULL ) ) {
        return 0;
    }

    for ( i = 0; i < length; i++ ) {
        switch ( cmdBufIn[i] ) {
            // STICKY
            case MOTE_MAC_DL_CHANNEL_ANS:
            case MOTE_MAC_RX_PARAM_SETUP_ANS: {
                // 1 byte payload
                cmdBufOut[cmdCount++] = cmdBufIn[i++];
                cmdBufOut[cmdCount++] = cmdBufIn[i];
                break;
            }
            case MOTE_MAC_RX_TIMING_SETUP_ANS: {
                // 0 byte payload
                cmdBufOut[cmdCount++] = cmdBufIn[i];
                break;
            }
            // NON-STICKY
            case MOTE_MAC_DEV_STATUS_ANS: {
                // 2 bytes payload
                i += 2;
                break;
            }
            case MOTE_MAC_LINK_ADR_ANS:
            case MOTE_MAC_NEW_CHANNEL_ANS: {
                // 1 byte payload
                i++;
                break;
            }
            case MOTE_MAC_TX_PARAM_SETUP_ANS:
            case MOTE_MAC_DUTY_CYCLE_ANS:
            case MOTE_MAC_LINK_CHECK_REQ: {
                // 0 byte payload
                break;
            }
            default:
                break;
        }
    }

    return cmdCount;
}

static void ProcessMacCommands( uint8_t *payload, uint8_t macIndex, uint8_t commandsSize, uint8_t snr, LoRaMacRxSlot_t rxSlot )
{
    uint8_t status = 0;
    while ( macIndex < commandsSize ) {
        // Decode Frame MAC commands
#ifdef CONFIG_LORA_VERIFY
        if (g_lora_debug) {
            PRINTF_RAW("MacCommands:%d being processed\r\n", payload[macIndex]);
        }
#endif
        switch ( payload[macIndex++] ) {
            case SRV_MAC_LINK_CHECK_ANS:
                if( LoRaMacConfirmQueueIsCmdActive( MLME_LINK_CHECK ) == true )
                {
                    LoRaMacConfirmQueueSetStatus( LORAMAC_EVENT_INFO_STATUS_OK, MLME_LINK_CHECK );
                	MlmeConfirm.DemodMargin = payload[macIndex++];
                	MlmeConfirm.NbGateways = payload[macIndex++];
#ifdef CONFIG_LWAN                    
                    McpsIndication.LinkCheckAnsReceived = true;
#endif    
                	DBG_PRINTF("margin %d, gateways %d\r\n", MlmeConfirm.DemodMargin, MlmeConfirm.NbGateways);
                }
                break;
            case SRV_MAC_LINK_ADR_REQ: {
                LinkAdrReqParams_t linkAdrReq;
                int8_t linkAdrDatarate = DR_0;
                int8_t linkAdrTxPower = TX_POWER_0;
                uint8_t linkAdrNbRep = 0;
                uint8_t linkAdrNbBytesParsed = 0;

                // Fill parameter structure
                linkAdrReq.Payload = &payload[macIndex - 1];
                linkAdrReq.PayloadSize = commandsSize - ( macIndex - 1 );
                linkAdrReq.AdrEnabled = AdrCtrlOn;
                linkAdrReq.UplinkDwellTime = LoRaMacParams.UplinkDwellTime;
                linkAdrReq.CurrentDatarate = LoRaMacParams.ChannelsDatarate;
                linkAdrReq.CurrentTxPower = LoRaMacParams.ChannelsTxPower;
                linkAdrReq.CurrentNbRep = LoRaMacParams.ChannelsNbRep;
                // Process the ADR requests
                status = RegionLinkAdrReq( LoRaMacRegion, &linkAdrReq, &linkAdrDatarate,
                                           &linkAdrTxPower, &linkAdrNbRep, &linkAdrNbBytesParsed );
                //printf("status:%d\r\n",status);
                if ( ( status & 0x07 ) == 0x07 ) {
                    LoRaMacParams.ChannelsDatarate = linkAdrDatarate;
                    //SaveDr();
                    LoRaMacParams.ChannelsTxPower = linkAdrTxPower;
                    LoRaMacParams.ChannelsNbRep = linkAdrNbRep;
                    //printf("ChannelsDatarate:%d ChannelsTxPower:%d,ChannelsNbRep:%d\r\n",LoRaMacParams.ChannelsDatarate,LoRaMacParams.ChannelsTxPower,LoRaMacParams.ChannelsNbRep);
                }

                // Add the answers to the buffer
                for ( uint8_t i = 0; i < ( linkAdrNbBytesParsed / 5 ); i++ ) {
                    AddMacCommand( MOTE_MAC_LINK_ADR_ANS, status, 0 );
                }
                // Update MAC index
                macIndex += linkAdrNbBytesParsed - 1;
            }
            break;
            case SRV_MAC_DUTY_CYCLE_REQ:
                MaxDCycle = payload[macIndex++];
                AggregatedDCycle = 1 << MaxDCycle;
                AddMacCommand( MOTE_MAC_DUTY_CYCLE_ANS, 0, 0 );
                break;
            case SRV_MAC_RX_PARAM_SETUP_REQ: {
                RxParamSetupReqParams_t rxParamSetupReq;
                status = 0x07;

                rxParamSetupReq.DrOffset = ( payload[macIndex] >> 4 ) & 0x07;
                rxParamSetupReq.Datarate = payload[macIndex] & 0x0F;
                macIndex++;

                rxParamSetupReq.Frequency =  ( uint32_t )payload[macIndex++];
                rxParamSetupReq.Frequency |= ( uint32_t )payload[macIndex++] << 8;
                rxParamSetupReq.Frequency |= ( uint32_t )payload[macIndex++] << 16;
                rxParamSetupReq.Frequency *= 100;

                // Perform request on region
                status = RegionRxParamSetupReq( LoRaMacRegion, &rxParamSetupReq );

                if ( ( status & 0x07 ) == 0x07 ) {
                    LoRaMacParams.Rx2Channel.Datarate = rxParamSetupReq.Datarate;
                    LoRaMacParams.Rx2Channel.Frequency = rxParamSetupReq.Frequency;
                    LoRaMacParams.Rx1DrOffset = rxParamSetupReq.DrOffset;
                }
                AddMacCommand( MOTE_MAC_RX_PARAM_SETUP_ANS, status, 0 );
            }
            break;
            case SRV_MAC_DEV_STATUS_REQ: {
                uint8_t batteryLevel = BAT_LEVEL_NO_MEASURE;
                if ( ( LoRaMacCallbacks != NULL ) && ( LoRaMacCallbacks->GetBatteryLevel != NULL ) ) {
                    batteryLevel = LoRaMacCallbacks->GetBatteryLevel( );
                }
#ifdef LORAMAC_CLASSB_TESTCASE
                DBG_PRINTF("receive SRV_MAC_DEV_STATUS_REQ\r\n");
#endif
                AddMacCommand( MOTE_MAC_DEV_STATUS_ANS, batteryLevel, snr );
                break;
            }
            case SRV_MAC_NEW_CHANNEL_REQ: {
                NewChannelReqParams_t newChannelReq;
                ChannelParams_t chParam;
                status = 0x03;

                newChannelReq.ChannelId = payload[macIndex++];
                newChannelReq.NewChannel = &chParam;

                chParam.Frequency = ( uint32_t )payload[macIndex++];
                chParam.Frequency |= ( uint32_t )payload[macIndex++] << 8;
                chParam.Frequency |= ( uint32_t )payload[macIndex++] << 16;
                chParam.Frequency *= 100;
                chParam.Rx1Frequency = 0;
                chParam.DrRange.Value = payload[macIndex++];

                status = RegionNewChannelReq( LoRaMacRegion, &newChannelReq );

                AddMacCommand( MOTE_MAC_NEW_CHANNEL_ANS, status, 0 );
            }
            break;
            case SRV_MAC_RX_TIMING_SETUP_REQ: {
                uint8_t delay = payload[macIndex++] & 0x0F;

                if ( delay == 0 ) {
                    delay++;
                }
                LoRaMacParams.ReceiveDelay1 = delay * 1000;
                LoRaMacParams.ReceiveDelay2 = LoRaMacParams.ReceiveDelay1 + 1000;
                AddMacCommand( MOTE_MAC_RX_TIMING_SETUP_ANS, 0, 0 );
            }
            break;
            case SRV_MAC_TX_PARAM_SETUP_REQ: {
                TxParamSetupReqParams_t txParamSetupReq;
                uint8_t eirpDwellTime = payload[macIndex++];

                txParamSetupReq.UplinkDwellTime = 0;
                txParamSetupReq.DownlinkDwellTime = 0;

                if ( ( eirpDwellTime & 0x20 ) == 0x20 ) {
                    txParamSetupReq.DownlinkDwellTime = 1;
                }
                if ( ( eirpDwellTime & 0x10 ) == 0x10 ) {
                    txParamSetupReq.UplinkDwellTime = 1;
                }
                txParamSetupReq.MaxEirp = eirpDwellTime & 0x0F;

                // Check the status for correctness
                if ( RegionTxParamSetupReq( LoRaMacRegion, &txParamSetupReq ) != -1 ) {
                    // Accept command
                    LoRaMacParams.UplinkDwellTime = txParamSetupReq.UplinkDwellTime;
                    LoRaMacParams.DownlinkDwellTime = txParamSetupReq.DownlinkDwellTime;
                    LoRaMacParams.MaxEirp = LoRaMacMaxEirpTable[txParamSetupReq.MaxEirp];
                    // Add command response
                    AddMacCommand( MOTE_MAC_TX_PARAM_SETUP_ANS, 0, 0 );
                }
            }
            break;
            case SRV_MAC_DL_CHANNEL_REQ: {
                DlChannelReqParams_t dlChannelReq;
                status = 0x03;

                dlChannelReq.ChannelId = payload[macIndex++];
                dlChannelReq.Rx1Frequency = ( uint32_t )payload[macIndex++];
                dlChannelReq.Rx1Frequency |= ( uint32_t )payload[macIndex++] << 8;
                dlChannelReq.Rx1Frequency |= ( uint32_t )payload[macIndex++] << 16;
                dlChannelReq.Rx1Frequency *= 100;

                status = RegionDlChannelReq( LoRaMacRegion, &dlChannelReq );

                AddMacCommand( MOTE_MAC_DL_CHANNEL_ANS, status, 0 );
            }
            break;
            case SRV_MAC_DEVICE_TIME_ANS:
                {
                    TimerTime_t currentTime = 0;
                    TimerSysTime_t sysTimeAns = { 0 };
                    TimerSysTime_t sysTime = { 0 };
                    TimerSysTime_t sysTimeCurrent = { 0 };

                    sysTimeAns.Seconds = ( uint32_t )payload[macIndex++];
                    sysTimeAns.Seconds |= ( uint32_t )payload[macIndex++] << 8;
                    sysTimeAns.Seconds |= ( uint32_t )payload[macIndex++] << 16;
                    sysTimeAns.Seconds |= ( uint32_t )payload[macIndex++] << 24;
                    sysTimeAns.SubSeconds = payload[macIndex++];

                    // Convert the fractional second received in ms
                    // round( pow( 0.5, 8.0 ) * 1000 ) = 3.90625
                    sysTimeAns.SubSeconds = sysTimeAns.SubSeconds * 3.90625;

                    // Add Unix to Gps epcoh offset. The system time is based on Unix time.
                    sysTimeAns.Seconds += UNIX_GPS_EPOCH_OFFSET;

                    // Compensate time difference between Tx Done time and now
                    sysTimeCurrent = TimerGetSysTime( );

                    sysTime = TimerAddSysTime( sysTimeCurrent, TimerSubSysTime( sysTimeAns, LastTxSysTime ) );
#ifdef LORAMAC_CLASSB_TESTCASE
                    DBG_PRINTF("receive SRV_MAC_DEVICE_TIME_ANS, set time=%u.%d\r\n", (unsigned int)sysTime.Seconds, sysTime.SubSeconds);
#endif
                    // Apply the new system time.
                    TimerSetSysTime( sysTime );
                    currentTime = TimerGetCurrentTime( );
                    
                    LoRaMacClassBDeviceTimeAns( currentTime );
#ifdef CONFIG_LWAN                    
                    McpsIndication.DevTimeAnsReceived = true;
#endif    
                }
                break;
            case SRV_MAC_PING_SLOT_INFO_ANS:
                {
                    // According to the specification, it is not allowed to process this answer in
                    // a ping or multicast slot
                    if( ( rxSlot != RX_SLOT_WIN_PING_SLOT ) && ( rxSlot != RX_SLOT_WIN_MULTICAST_SLOT ) )
                    {
#ifdef LORAMAC_CLASSB_TESTCASE
                        DBG_PRINTF("receive SRV_MAC_PING_SLOT_INFO_ANS in RX1\r\n");
#endif
                        LoRaMacClassBPingSlotInfoAns( );
                    }
                }
                break;
            case SRV_MAC_PING_SLOT_CHANNEL_REQ:
                {
                    uint8_t status = 0x03;
                    uint32_t frequency = 0;
                    uint8_t datarate;

                    frequency = ( uint32_t )payload[macIndex++];
                    frequency |= ( uint32_t )payload[macIndex++] << 8;
                    frequency |= ( uint32_t )payload[macIndex++] << 16;
                    frequency *= 100;
                    datarate = payload[macIndex++] & 0x0F;
#ifdef LORAMAC_CLASSB_TESTCASE
                    DBG_PRINTF("receive SRV_MAC_PING_SLOT_CHANNEL_REQ frequency=%u datarate=%u\r\n", (unsigned int)frequency, datarate);
#endif
                    status = LoRaMacClassBPingSlotChannelReq( datarate, frequency );
                    AddMacCommand( MOTE_MAC_PING_SLOT_FREQ_ANS, status, 0 );
                }
                break;
            case SRV_MAC_BEACON_TIMING_ANS:
                {
                    uint16_t beaconTimingDelay = 0;
                    uint8_t beaconTimingChannel = 0;

                    beaconTimingDelay = ( uint16_t )payload[macIndex++];
                    beaconTimingDelay |= ( uint16_t )payload[macIndex++] << 8;
                    beaconTimingChannel = payload[macIndex++];

                    LoRaMacClassBBeaconTimingAns( beaconTimingDelay, beaconTimingChannel );
                }
                break;
            case SRV_MAC_BEACON_FREQ_REQ:
                {
                    uint32_t frequency = 0;

                    frequency = ( uint32_t )payload[macIndex++];
                    frequency |= ( uint32_t )payload[macIndex++] << 8;
                    frequency |= ( uint32_t )payload[macIndex++] << 16;
                    frequency *= 100;
#ifdef LORAMAC_CLASSB_TESTCASE
                    DBG_PRINTF("receive SRV_MAC_BEACON_FREQ_REQ frequency=%u\r\n", (unsigned int)frequency);
#endif
                    if( LoRaMacClassBBeaconFreqReq( frequency ) == true )
                    {
                        AddMacCommand( MOTE_MAC_BEACON_FREQ_ANS, 1, 0 );
                    }
                    else
                    {
                        AddMacCommand( MOTE_MAC_BEACON_FREQ_ANS, 0, 0 );
                    }
                }
                break;
            default:
                // Unknown command. ABORT MAC commands processing
                return;
        }
    }
}



#ifdef CONFIG_LORA_CAD
static bool StartCAD( uint8_t channel )
{
    TxConfigParams_t txConfig;
    int8_t txPower = 0;
    TimerTime_t txTime = 0;

    memset(&txConfig, 0, sizeof(TxConfigParams_t));
    txConfig.Channel = channel;
    txConfig.Datarate = LoRaMacParams.ChannelsDatarate;
    txConfig.TxPower = LoRaMacParams.ChannelsTxPower;
    txConfig.MaxEirp = LoRaMacParams.MaxEirp;
    txConfig.AntennaGain = LoRaMacParams.AntennaGain;
    txConfig.PktLen = LoRaMacBufferPktLen;

    bool ret = RegionTxConfig( LoRaMacRegion, &txConfig, &txPower, &txTime );
#ifdef __asr6601__
		delay(2);
#endif

    Radio.StartCad(LORA_CAD_SYMBOLS);
    LoRaMacState |= LORAMAC_CAD_RUNNING;
    return ret;
}

static void beforeTxCadDone( bool channelActivityDetected )
{
    Radio.Sleep( );
    //printf("beforeTxCadDone\r\n");
    if(channelActivityDetected && g_lora_cad_cnt<LORA_CAD_CNT_MAX) {
        // Send later - prepare timer
        g_lora_cad_cnt ++;
        LoRaMacState |= LORAMAC_TX_DELAYED;
        TimerSetValue( &TxDelayedTimer, LORA_CAD_DELAY );
        TimerStart( &TxDelayedTimer );
    }else {
        // Try to send now
        g_lora_cad_cnt = 1;
        LoRaMacState |= LORAMAC_TX_RUNNING;
        //SendFrameOnChannel( Channel );
        TimerSetValue( &TxImmediateTimer, 1 );
        TimerStart( &TxImmediateTimer );
    }
}

#endif

#ifdef CLASS_A_WOTA
uint32_t testtime;
void stopWotaCad()
{
	wota_CadEnable = false;
	TimerStop(&wota_CadTimer);
	wota_CadTimerStarted = false;
	wota_CadStarted = false;
	Radio.Sleep();
}
void wotaCadProcess()
{
	if(LoRaMacState==LORAMAC_IDLE &&IsLoRaMacNetworkJoined)
	{
		if(wota_CadTimerStarted == false && wota_CadStarted == false && wota_CadEnable)
		{
			TimerSetValue(&wota_CadTimer,1);
			TimerStart(&wota_CadTimer);
			wota_CadTimerStarted = true;
		}
	}
	else
	{
		TimerStop(&wota_CadTimer);
		wota_CadTimerStarted = false;
		wota_CadStarted = false;
	}
}

bool Onwota_CadTimer()
{
	//printf("%d\r\n",testtime);
	//testtime=(uint32_t)TimerGetCurrentTime();
	//TimerStop(&wota_CadTimer);
	wota_CadTimerStarted = false;
	if(LoRaMacState==LORAMAC_IDLE)
	{
		wota_CadStarted = true;
		RxWindow3Config.RxContinuous = true;
		RxWindow3Config.Datarate = wota_dr;
		RxWindow3Config.Frequency = wota_freq;
		RxWindow3Config.RxSlot = RX_SLOT_WOTA;
		
		RegionRxConfig( LoRaMacRegion, &RxWindow3Config, ( int8_t * )&McpsIndication.RxDatarate );
		Radio.StartCad(CLASS_A_WOTA_CAD_SYMBOLS);
		TimerSetValue(&wota_CadTimer,wota_cycle_time-20);
		TimerStart(&wota_CadTimer);
		wota_CadTimerStarted = true;
	}
}

static void wota_CadDone( bool channelActivityDetected )
{
	if(channelActivityDetected) {
		//Radio.Sleep();
		if ( RegionRxConfig( LoRaMacRegion, &RxWindow3Config, ( int8_t * )&McpsIndication.RxDatarate ) == true ) {
			RxWindowSetup( false, wota_max_rxtime );
			RxSlot = RX_SLOT_WIN_CLASS_C;;
		}
		wota_CadTimerStarted=false;
		TimerStop(&wota_CadTimer);
#if(LoraWan_RGB==1)
		turnOnRGB(COLOR_RXWINDOW3,0);
#endif
    }
    else
    {
        Radio.Sleep( );
        wota_CadStarted = false;
    }
    //testtime=(uint32_t)TimerGetCurrentTime()-testtime;
}

#endif

LoRaMacStatus_t Send( LoRaMacHeader_t *macHdr, uint8_t fPort, void *fBuffer, uint16_t fBufferSize )
{
    LoRaMacFrameCtrl_t fCtrl;
    LoRaMacStatus_t status = LORAMAC_STATUS_PARAMETER_INVALID;
#ifdef CONFIG_LWAN
    lwan_dev_status_set(DEVICE_STATUS_IDLE);
#endif
    fCtrl.Value = 0;
    fCtrl.Bits.FOptsLen      = 0;
    if( LoRaMacDeviceClass == CLASS_B )
    {
#ifdef LORAMAC_CLASSB_TESTCASE
        DBG_PRINTF("Send class b frame\r\n");
#endif        
        fCtrl.Bits.FPending      = 1;
    }
    else
    {
        fCtrl.Bits.FPending      = 0;
    }
    fCtrl.Bits.Ack           = false;
    fCtrl.Bits.AdrAckReq     = false;
    fCtrl.Bits.Adr           = AdrCtrlOn;

    // Prepare the frame
    status = PrepareFrame( macHdr, &fCtrl, fPort, fBuffer, fBufferSize );

    // Validate status
    if ( status != LORAMAC_STATUS_OK ) {
        return status;
    }

    // Reset confirm parameters
    McpsConfirm.NbRetries = 0;
    McpsConfirm.AckReceived = false;
    McpsConfirm.UpLinkCounter = UpLinkCounter;


    status = ScheduleTx( );
#ifdef CONFIG_LORA_VERIFY
    if (g_lora_debug == true) {
        PRINTF_RAW("MacHdr major:%d rfu:%d mtype:%d\r\n", macHdr->Bits.Major, macHdr->Bits.RFU, macHdr->Bits.MType);
        PRINTF_RAW("fCtrl rfu:%d Adr:%d AdrAckReq:%d fCtrl.Bits.Ack:%d\r\n", fCtrl.Bits.FPending, fCtrl.Bits.Adr, fCtrl.Bits.AdrAckReq, fCtrl.Bits.Ack);
    }
#endif
    return status;
}

static LoRaMacStatus_t ScheduleTx( void )
{
    TimerTime_t dutyCycleTimeOff = 0;
    NextChanParams_t nextChan;

    // Check if the device is off
    if ( MaxDCycle == 255 ) {
        return LORAMAC_STATUS_DEVICE_OFF;
    }
    if ( MaxDCycle == 0 ) {
        AggregatedTimeOff = 0;
    }

    // Update Backoff
    CalculateBackOff( LastTxChannel );

    nextChan.AggrTimeOff = AggregatedTimeOff;
    nextChan.Datarate = LoRaMacParams.ChannelsDatarate;
    nextChan.DutyCycleEnabled = DutyCycleOn;
    nextChan.Joined = IsLoRaMacNetworkJoined;
    nextChan.LastAggrTx = AggregatedLastTxDoneTime;

	GetPhyParams_t getPhy;
	PhyParam_t phyParam;
	getPhy.Attribute = PHY_MIN_TX_DR;
	phyParam = RegionGetPhyParam( LoRaMacRegion, &getPhy );
	int8_t minDatarate = phyParam.Value;

	getPhy.Attribute = PHY_MAX_TX_DR;
	phyParam = RegionGetPhyParam( LoRaMacRegion, &getPhy );
	int8_t maxDatarate = phyParam.Value;

    // Select channel
    while ( RegionNextChannel( LoRaMacRegion, &nextChan, &Channel, &dutyCycleTimeOff, &AggregatedTimeOff ) == false ) {
        // Set the default datarate
        //LoRaMacParams.ChannelsDatarate = LoRaMacParamsDefaults.ChannelsDatarate;
        if(LoRaMacParams.ChannelsDatarate == minDatarate)
        {
            LoRaMacParams.ChannelsDatarate = maxDatarate;
        }
        else
        {
            LoRaMacParams.ChannelsDatarate --;
        }
        // Update datarate in the function parameters
        nextChan.Datarate = LoRaMacParams.ChannelsDatarate;
    }


    // Compute Rx1 windows parameters
    RegionComputeRxWindowParameters( LoRaMacRegion,
                                     RegionApplyDrOffset( LoRaMacRegion, LoRaMacParams.DownlinkDwellTime, LoRaMacParams.ChannelsDatarate,
                                                          LoRaMacParams.Rx1DrOffset ),
                                     LoRaMacParams.MinRxSymbols,
                                     LoRaMacParams.SystemMaxRxError,
                                     &RxWindow1Config );
    // Compute Rx2 windows parameters
    RegionComputeRxWindowParameters( LoRaMacRegion,
                                     LoRaMacParams.Rx2Channel.Datarate,
                                     LoRaMacParams.MinRxSymbols,
                                     LoRaMacParams.SystemMaxRxError,
                                     &RxWindow2Config );

    if ( IsLoRaMacNetworkJoined == false ) {
        RxWindow1Delay = LoRaMacParams.JoinAcceptDelay1 + RxWindow1Config.WindowOffset;
        RxWindow2Delay = LoRaMacParams.JoinAcceptDelay2 + RxWindow2Config.WindowOffset;
    } else {
        if ( ValidatePayloadLength( LoRaMacTxPayloadLen, LoRaMacParams.ChannelsDatarate, MacCommandsBufferIndex ) == false ) {
            return LORAMAC_STATUS_LENGTH_ERROR;
        }
        RxWindow1Delay = LoRaMacParams.ReceiveDelay1 + RxWindow1Config.WindowOffset;
        RxWindow2Delay = LoRaMacParams.ReceiveDelay2 + RxWindow2Config.WindowOffset;
    }

    // Schedule transmission of frame
    if ( dutyCycleTimeOff == 0 ) {
#ifdef  CONFIG_LORA_CAD   
        StartCAD(Channel);
        return LORAMAC_STATUS_OK;
#else        
        // Try to send now
        return SendFrameOnChannel( Channel );
#endif        
    } else {
        // Send later - prepare timer
        LoRaMacState |= LORAMAC_TX_DELAYED;
        TimerSetValue( &TxDelayedTimer, dutyCycleTimeOff );
        TimerStart( &TxDelayedTimer );

        return LORAMAC_STATUS_OK;
    }
}

static void CalculateBackOff( uint8_t channel )
{
    CalcBackOffParams_t calcBackOff;

    calcBackOff.Joined = IsLoRaMacNetworkJoined;
    calcBackOff.DutyCycleEnabled = DutyCycleOn;
    calcBackOff.Channel = channel;
    calcBackOff.ElapsedTime = TimerGetElapsedTime( LoRaMacInitializationTime );
    calcBackOff.TxTimeOnAir = TxTimeOnAir;
    calcBackOff.LastTxIsJoinRequest = LastTxIsJoinRequest;

    // Update regional back-off
    RegionCalcBackOff( LoRaMacRegion, &calcBackOff );

    // Update aggregated time-off
    AggregatedTimeOff = TxTimeOnAir * AggregatedDCycle - TxTimeOnAir;
}

static void ResetMacParameters( void )
{
    IsLoRaMacNetworkJoined = false;

    // Counters
    UpLinkCounter = 0;
    DownLinkCounter = -1;
    AdrAckCounter = 0;

    ChannelsNbRepCounter = 0;

    AckTimeoutRetries = 1;
    AckTimeoutRetriesCounter = 1;
    AckTimeoutRetry = false;

    MaxDCycle = 0;
    AggregatedDCycle = 1;

    MacCommandsBufferIndex = 0;
    MacCommandsBufferToRepeatIndex = 0;

    IsRxWindowsEnabled = true;

    LoRaMacParams.ChannelsTxPower = LoRaMacParamsDefaults.ChannelsTxPower;
    LoRaMacParams.ChannelsDatarate = LoRaMacParamsDefaults.ChannelsDatarate;
    LoRaMacParams.Rx1DrOffset = LoRaMacParamsDefaults.Rx1DrOffset;
    memcpy(&LoRaMacParams.Rx2Channel, &LoRaMacParamsDefaults.Rx2Channel, sizeof(LoRaMacParams.Rx2Channel));
    LoRaMacParams.UplinkDwellTime = LoRaMacParamsDefaults.UplinkDwellTime;
    LoRaMacParams.DownlinkDwellTime = LoRaMacParamsDefaults.DownlinkDwellTime;
    LoRaMacParams.MaxEirp = LoRaMacParamsDefaults.MaxEirp;
    LoRaMacParams.AntennaGain = LoRaMacParamsDefaults.AntennaGain;

    NodeAckRequested = false;
    SrvAckRequested = false;
    MacCommandsInNextTx = false;

    // Reset Multicast downlink counters
    MulticastParams_t *cur = MulticastChannels;
    while ( cur != NULL ) {
        cur->DownLinkCounter = 0;
        cur = cur->Next;
    }

    // Initialize channel index.
    Channel = 0;
    LastTxChannel = Channel;
}

static bool IsFPortAllowed( uint8_t fPort )
{
    if( fPort > 224 )
    {
        return false;
    }
    return true;
}

static void OpenContinuousRx2Window( void )
{
    OnRxWindow2TimerEvent( );
    RxSlot = RX_SLOT_WIN_CLASS_C;
}

LoRaMacStatus_t PrepareFrame( LoRaMacHeader_t *macHdr, LoRaMacFrameCtrl_t *fCtrl, uint8_t fPort, void *fBuffer,
                              uint16_t fBufferSize )
{
    AdrNextParams_t adrNext;
    uint16_t i;
    uint8_t pktHeaderLen = 0;
    uint32_t mic = 0;
    const void *payload = fBuffer;
    uint8_t framePort = fPort;

    LoRaMacBufferPktLen = 0;

    NodeAckRequested = false;

    if ( fBuffer == NULL ) {
        fBufferSize = 0;
    }

    LoRaMacTxPayloadLen = fBufferSize;

    LoRaMacBuffer[pktHeaderLen++] = macHdr->Value;

    switch ( macHdr->Bits.MType ) {
#ifdef CONFIG_LORA_VERIFY
        if (g_lora_debug)
            PRINTF_RAW("UpLoad frame %d being processed\r\n", macHdr->Bits.MType);
#endif
        case FRAME_TYPE_JOIN_REQ:
            LoRaMacBufferPktLen = pktHeaderLen;

            memcpyr( LoRaMacBuffer + LoRaMacBufferPktLen, LoRaMacAppEui, 8 );
            LoRaMacBufferPktLen += 8;
            memcpyr( LoRaMacBuffer + LoRaMacBufferPktLen, LoRaMacDevEui, 8 );
            LoRaMacBufferPktLen += 8;

            LoRaMacDevNonce = rand1( );
#ifdef CONFIG_LORA_VERIFY
			if (g_lora_debug) {
                PRINTF_RAW("DevNonce:%d\rn\n", LoRaMacDevNonce);
            }
#endif
            LoRaMacBuffer[LoRaMacBufferPktLen++] = LoRaMacDevNonce & 0xFF;
            LoRaMacBuffer[LoRaMacBufferPktLen++] = ( LoRaMacDevNonce >> 8 ) & 0xFF;

            LoRaMacJoinComputeMic( LoRaMacBuffer, LoRaMacBufferPktLen & 0xFF, LoRaMacAppKey, &mic );

            LoRaMacBuffer[LoRaMacBufferPktLen++] = mic & 0xFF;
            LoRaMacBuffer[LoRaMacBufferPktLen++] = ( mic >> 8 ) & 0xFF;
            LoRaMacBuffer[LoRaMacBufferPktLen++] = ( mic >> 16 ) & 0xFF;
            LoRaMacBuffer[LoRaMacBufferPktLen++] = ( mic >> 24 ) & 0xFF;

            break;
        case FRAME_TYPE_DATA_CONFIRMED_UP:
            NodeAckRequested = true;
        //Intentional fallthrough
        case FRAME_TYPE_DATA_UNCONFIRMED_UP:
            if ( IsLoRaMacNetworkJoined == false ) {
                return LORAMAC_STATUS_NO_NETWORK_JOINED; // No network has been joined yet
            }

            // Adr next request
            adrNext.UpdateChanMask = true;
            adrNext.AdrEnabled = fCtrl->Bits.Adr;
            adrNext.AdrAckCounter = AdrAckCounter;
            adrNext.Datarate = LoRaMacParams.ChannelsDatarate;
            adrNext.TxPower = LoRaMacParams.ChannelsTxPower;
            adrNext.UplinkDwellTime = LoRaMacParams.UplinkDwellTime;

            fCtrl->Bits.AdrAckReq = RegionAdrNext( LoRaMacRegion, &adrNext,
                                                   &LoRaMacParams.ChannelsDatarate, &LoRaMacParams.ChannelsTxPower, &AdrAckCounter );

            if ( SrvAckRequested == true ) {
                SrvAckRequested = false;
                fCtrl->Bits.Ack = 1;
            }

            LoRaMacBuffer[pktHeaderLen++] = ( LoRaMacDevAddr ) & 0xFF;
            LoRaMacBuffer[pktHeaderLen++] = ( LoRaMacDevAddr >> 8 ) & 0xFF;
            LoRaMacBuffer[pktHeaderLen++] = ( LoRaMacDevAddr >> 16 ) & 0xFF;
            LoRaMacBuffer[pktHeaderLen++] = ( LoRaMacDevAddr >> 24 ) & 0xFF;

            LoRaMacBuffer[pktHeaderLen++] = fCtrl->Value;

            LoRaMacBuffer[pktHeaderLen++] = UpLinkCounter & 0xFF;
            LoRaMacBuffer[pktHeaderLen++] = ( UpLinkCounter >> 8 ) & 0xFF;

            // Copy the MAC commands which must be re-send into the MAC command buffer
            memcpy1( &MacCommandsBuffer[MacCommandsBufferIndex], MacCommandsBufferToRepeat, MacCommandsBufferToRepeatIndex );
            MacCommandsBufferIndex += MacCommandsBufferToRepeatIndex;

            if ( ( payload != NULL ) && ( LoRaMacTxPayloadLen > 0 ) ) {
                if ( MacCommandsInNextTx == true ) {
                    if ( MacCommandsBufferIndex <= LORA_MAC_COMMAND_MAX_FOPTS_LENGTH ) {
                        fCtrl->Bits.FOptsLen += MacCommandsBufferIndex;

                        // Update FCtrl field with new value of OptionsLength
                        LoRaMacBuffer[0x05] = fCtrl->Value;
                        for ( i = 0; i < MacCommandsBufferIndex; i++ ) {
                            LoRaMacBuffer[pktHeaderLen++] = MacCommandsBuffer[i];
                        }
                    } else {
                        LoRaMacTxPayloadLen = MacCommandsBufferIndex;
                        payload = MacCommandsBuffer;
                        framePort = 0;
                    }
                }
            } else {
                if ( ( MacCommandsBufferIndex > 0 ) && ( MacCommandsInNextTx == true ) ) {
                    LoRaMacTxPayloadLen = MacCommandsBufferIndex;
                    payload = MacCommandsBuffer;
                    framePort = 0;
                }
            }
            MacCommandsInNextTx = false;
            // Store MAC commands which must be re-send in case the device does not receive a downlink anymore
            MacCommandsBufferToRepeatIndex = ParseMacCommandsToRepeat( MacCommandsBuffer, MacCommandsBufferIndex,
                                                                       MacCommandsBufferToRepeat );
            if ( MacCommandsBufferToRepeatIndex > 0 ) {
                MacCommandsInNextTx = true;
            }

            if ( ( payload != NULL ) && ( LoRaMacTxPayloadLen > 0 ) ) {
                LoRaMacBuffer[pktHeaderLen++] = framePort;
                if ((pktHeaderLen + LoRaMacTxPayloadLen) > (LORAMAC_PHY_MAXPAYLOAD - 4))  {
                    LoRaMacTxPayloadLen = LORAMAC_PHY_MAXPAYLOAD - 4 - pktHeaderLen;
                }
    
                if ( framePort == 0 ) {
                    // Reset buffer index as the mac commands are being sent on port 0
                    MacCommandsBufferIndex = 0;
                    LoRaMacPayloadEncrypt( (uint8_t * ) payload, LoRaMacTxPayloadLen, LoRaMacNwkSKey, LoRaMacDevAddr, UP_LINK,
                                           UpLinkCounter, &LoRaMacBuffer[pktHeaderLen] );
                } else {
                    LoRaMacPayloadEncrypt( (uint8_t * ) payload, LoRaMacTxPayloadLen, LoRaMacAppSKey, LoRaMacDevAddr, UP_LINK,
                                           UpLinkCounter, &LoRaMacBuffer[pktHeaderLen] );
                }
            }
            LoRaMacBufferPktLen = pktHeaderLen + LoRaMacTxPayloadLen;

            LoRaMacComputeMic( LoRaMacBuffer, LoRaMacBufferPktLen, LoRaMacNwkSKey, LoRaMacDevAddr, UP_LINK, UpLinkCounter, &mic );

            LoRaMacBuffer[LoRaMacBufferPktLen + 0] = mic & 0xFF;
            LoRaMacBuffer[LoRaMacBufferPktLen + 1] = ( mic >> 8 ) & 0xFF;
            LoRaMacBuffer[LoRaMacBufferPktLen + 2] = ( mic >> 16 ) & 0xFF;
            LoRaMacBuffer[LoRaMacBufferPktLen + 3] = ( mic >> 24 ) & 0xFF;

            LoRaMacBufferPktLen += LORAMAC_MFR_LEN;

            break;
        case FRAME_TYPE_PROPRIETARY:
            if ( ( fBuffer != NULL ) && ( LoRaMacTxPayloadLen > 0 ) ) {
                memcpy1( LoRaMacBuffer + pktHeaderLen, ( uint8_t * ) fBuffer, LoRaMacTxPayloadLen );
                LoRaMacBufferPktLen = pktHeaderLen + LoRaMacTxPayloadLen;
            }
            break;
        default:
            return LORAMAC_STATUS_SERVICE_UNKNOWN;
    }

    return LORAMAC_STATUS_OK;
}




LoRaMacStatus_t SendFrameOnChannel( uint8_t channel )
{

    TxConfigParams_t txConfig;
    int8_t txPower = 0;

    txConfig.Channel = channel;
    txConfig.Datarate = LoRaMacParams.ChannelsDatarate;
    txConfig.TxPower = LoRaMacParams.ChannelsTxPower;
    txConfig.MaxEirp = LoRaMacParams.MaxEirp;
    txConfig.AntennaGain = LoRaMacParams.AntennaGain;
    txConfig.PktLen = LoRaMacBufferPktLen;


    if( LoRaMacClassBIsBeaconExpected( ) == true )
    {
        return LORAMAC_STATUS_BUSY_BEACON_RESERVED_TIME;
    }

    if( LoRaMacDeviceClass == CLASS_B )
    {
        if( LoRaMacClassBIsPingExpected( ) == true )
        {
            return LORAMAC_STATUS_BUSY_PING_SLOT_WINDOW_TIME;
        }
        else if( LoRaMacClassBIsMulticastExpected( ) == true )
        {
            return LORAMAC_STATUS_BUSY_PING_SLOT_WINDOW_TIME;
        }
        else
        {
            LoRaMacClassBStopRxSlots( );
        }
    }
    RegionTxConfig( LoRaMacRegion, &txConfig, &txPower, &TxTimeOnAir );

    LoRaMacConfirmQueueSetStatusCmn( LORAMAC_EVENT_INFO_STATUS_ERROR );
    McpsConfirm.Status = LORAMAC_EVENT_INFO_STATUS_ERROR;
    McpsConfirm.Datarate = LoRaMacParams.ChannelsDatarate;
    McpsConfirm.TxPower = txPower;
    McpsConfirm.Channel = channel;

    // Store the time on air
    McpsConfirm.TxTimeOnAir = TxTimeOnAir;
    MlmeConfirm.TxTimeOnAir = TxTimeOnAir;

    if( LoRaMacClassBIsBeaconModeActive( ) == true )
    {
        // Currently, the Time-On-Air can only be computed when the radion is configured with
        // the TX configuration
        TimerTime_t collisionTime = LoRaMacClassBIsUplinkCollision( TxTimeOnAir );

        if( collisionTime > 0 )
        {
            return LORAMAC_STATUS_BUSY_UPLINK_COLLISION;
        }
    }

    LoRaMacClassBHaltBeaconing( );

    // Starts the MAC layer status check timer
    TimerSetValue( &MacStateCheckTimer, MAC_STATE_CHECK_TIMEOUT );
    TimerStart( &MacStateCheckTimer );

    if ( IsLoRaMacNetworkJoined == false ) {
        JoinRequestTrials++;
    }
    // Send now
#if (LoraWan_RGB==1)
    turnOnRGB(COLOR_SEND,0);
#endif

    Radio.Send( LoRaMacBuffer, LoRaMacBufferPktLen );

    LoRaMacState |= LORAMAC_TX_RUNNING;

    return LORAMAC_STATUS_OK;
}

LoRaMacStatus_t SetTxContinuousWave( uint16_t timeout )
{
    ContinuousWaveParams_t continuousWave;

    continuousWave.Channel = Channel;
    continuousWave.Datarate = LoRaMacParams.ChannelsDatarate;
    continuousWave.TxPower = LoRaMacParams.ChannelsTxPower;
    continuousWave.MaxEirp = LoRaMacParams.MaxEirp;
    continuousWave.AntennaGain = LoRaMacParams.AntennaGain;
    continuousWave.Timeout = timeout;

    RegionSetContinuousWave( LoRaMacRegion, &continuousWave );

    // Starts the MAC layer status check timer
    TimerSetValue( &MacStateCheckTimer, MAC_STATE_CHECK_TIMEOUT );
    TimerStart( &MacStateCheckTimer );

    LoRaMacState |= LORAMAC_TX_RUNNING;

    return LORAMAC_STATUS_OK;
}

LoRaMacStatus_t SetTxContinuousWave1( uint16_t timeout, uint32_t frequency, uint8_t power )
{
    Radio.SetTxContinuousWave( frequency, power, timeout );

    // Starts the MAC layer status check timer
    TimerSetValue( &MacStateCheckTimer, MAC_STATE_CHECK_TIMEOUT );
    TimerStart( &MacStateCheckTimer );

    LoRaMacState |= LORAMAC_TX_RUNNING;

    return LORAMAC_STATUS_OK;
}

LoRaMacStatus_t LoRaMacInitialization( LoRaMacPrimitives_t *primitives, LoRaMacCallback_t *callbacks,
                                       LoRaMacRegion_t region )
{
    GetPhyParams_t getPhy;
    PhyParam_t phyParam;
    LoRaMacClassBCallback_t classBCallbacks;
    LoRaMacClassBParams_t classBParams;

    if ( primitives == NULL ) {
        return LORAMAC_STATUS_PARAMETER_INVALID;
    }

    if( ( primitives->MacMcpsConfirm == NULL ) ||
        ( primitives->MacMcpsIndication == NULL ) ||
        ( primitives->MacMlmeConfirm == NULL ) ||
        ( primitives->MacMlmeIndication == NULL ) ) {
        return LORAMAC_STATUS_PARAMETER_INVALID;
    }
    // Verify if the region is supported
    if ( RegionIsActive( region ) == false ) {
        return LORAMAC_STATUS_REGION_NOT_SUPPORTED;
    }

    // Confirm queue reset
    LoRaMacConfirmQueueInit( primitives );

    LoRaMacPrimitives = primitives;
    LoRaMacCallbacks = callbacks;
    LoRaMacRegion = region;

    if(IsLoRaMacNetworkJoined==false){
    LoRaMacFlags.Value = 0;

    LoRaMacDeviceClass = CLASS_A;
    LoRaMacState = LORAMAC_IDLE;

    JoinRequestTrials = 0;
    MaxJoinRequestTrials = 1;

    // Reset duty cycle times
    AggregatedLastTxDoneTime = 0;
    AggregatedTimeOff = 0;

    // Reset to defaults
    getPhy.Attribute = PHY_DUTY_CYCLE;
    phyParam = RegionGetPhyParam( LoRaMacRegion, &getPhy );
    DutyCycleOn = ( bool ) phyParam.Value;

    getPhy.Attribute = PHY_DEF_TX_POWER;
    phyParam = RegionGetPhyParam( LoRaMacRegion, &getPhy );
    LoRaMacParamsDefaults.ChannelsTxPower = phyParam.Value;

    getPhy.Attribute = PHY_DEF_TX_DR;
    phyParam = RegionGetPhyParam( LoRaMacRegion, &getPhy );
    LoRaMacParamsDefaults.ChannelsDatarate = phyParam.Value;

    getPhy.Attribute = PHY_MAX_RX_WINDOW;
    phyParam = RegionGetPhyParam( LoRaMacRegion, &getPhy );
    LoRaMacParamsDefaults.MaxRxWindow = phyParam.Value;

    getPhy.Attribute = PHY_RECEIVE_DELAY1;
    phyParam = RegionGetPhyParam( LoRaMacRegion, &getPhy );
    LoRaMacParamsDefaults.ReceiveDelay1 = phyParam.Value;

    getPhy.Attribute = PHY_RECEIVE_DELAY2;
    phyParam = RegionGetPhyParam( LoRaMacRegion, &getPhy );
    LoRaMacParamsDefaults.ReceiveDelay2 = phyParam.Value;

    getPhy.Attribute = PHY_JOIN_ACCEPT_DELAY1;
    phyParam = RegionGetPhyParam( LoRaMacRegion, &getPhy );
    LoRaMacParamsDefaults.JoinAcceptDelay1 = phyParam.Value;

    getPhy.Attribute = PHY_JOIN_ACCEPT_DELAY2;
    phyParam = RegionGetPhyParam( LoRaMacRegion, &getPhy );
    LoRaMacParamsDefaults.JoinAcceptDelay2 = phyParam.Value;

    getPhy.Attribute = PHY_DEF_DR1_OFFSET;
    phyParam = RegionGetPhyParam( LoRaMacRegion, &getPhy );
    LoRaMacParamsDefaults.Rx1DrOffset = phyParam.Value;

    getPhy.Attribute = PHY_DEF_RX2_FREQUENCY;
    phyParam = RegionGetPhyParam( LoRaMacRegion, &getPhy );
    LoRaMacParamsDefaults.Rx2Channel.Frequency = phyParam.Value;

    getPhy.Attribute = PHY_DEF_RX2_DR;
    phyParam = RegionGetPhyParam( LoRaMacRegion, &getPhy );
    LoRaMacParamsDefaults.Rx2Channel.Datarate = phyParam.Value;

    getPhy.Attribute = PHY_DEF_UPLINK_DWELL_TIME;
    phyParam = RegionGetPhyParam( LoRaMacRegion, &getPhy );
    LoRaMacParamsDefaults.UplinkDwellTime = phyParam.Value;

    getPhy.Attribute = PHY_DEF_DOWNLINK_DWELL_TIME;
    phyParam = RegionGetPhyParam( LoRaMacRegion, &getPhy );
    LoRaMacParamsDefaults.DownlinkDwellTime = phyParam.Value;

    getPhy.Attribute = PHY_DEF_MAX_EIRP;
    phyParam = RegionGetPhyParam( LoRaMacRegion, &getPhy );
    LoRaMacParamsDefaults.MaxEirp = phyParam.fValue;

    getPhy.Attribute = PHY_DEF_ANTENNA_GAIN;
    phyParam = RegionGetPhyParam( LoRaMacRegion, &getPhy );
    LoRaMacParamsDefaults.AntennaGain = phyParam.fValue;

    RegionInitDefaults( LoRaMacRegion, INIT_TYPE_INIT );

    // Init parameters which are not set in function ResetMacParameters
    LoRaMacParams.RepeaterSupport = false;
    LoRaMacParamsDefaults.ChannelsNbRep = 1;
    LoRaMacParamsDefaults.SystemMaxRxError = 30;
    LoRaMacParamsDefaults.MinRxSymbols = 6;

    LoRaMacParams.SystemMaxRxError = LoRaMacParamsDefaults.SystemMaxRxError;
    LoRaMacParams.MinRxSymbols = LoRaMacParamsDefaults.MinRxSymbols;
    LoRaMacParams.MaxRxWindow = LoRaMacParamsDefaults.MaxRxWindow;
    LoRaMacParams.ReceiveDelay1 = LoRaMacParamsDefaults.ReceiveDelay1;
    LoRaMacParams.ReceiveDelay2 = LoRaMacParamsDefaults.ReceiveDelay2;
    LoRaMacParams.JoinAcceptDelay1 = LoRaMacParamsDefaults.JoinAcceptDelay1;
    LoRaMacParams.JoinAcceptDelay2 = LoRaMacParamsDefaults.JoinAcceptDelay2;
    LoRaMacParams.ChannelsNbRep = LoRaMacParamsDefaults.ChannelsNbRep;

      ResetMacParameters( );
    }

    // Initialize timers
    TimerInit( &MacStateCheckTimer, OnMacStateCheckTimerEvent );
    TimerSetValue( &MacStateCheckTimer, MAC_STATE_CHECK_TIMEOUT );

    TimerInit( &TxDelayedTimer, OnTxDelayedTimerEvent );
    TimerInit( &RxWindowTimer1, OnRxWindow1TimerEvent );
    TimerInit( &RxWindowTimer2, OnRxWindow2TimerEvent );
    TimerInit( &AckTimeoutTimer, OnAckTimeoutTimerEvent );
#ifdef CONFIG_LORA_CAD    
    TimerInit( &TxImmediateTimer, OnTxImmediateTimerEvent );
#endif

#ifdef CLASS_A_WOTA
	TimerInit( &wota_CadTimer, Onwota_CadTimer );
#endif

    // Store the current initialization time
    LoRaMacInitializationTime = TimerGetCurrentTime( );

    // Initialize Radio driver
    RadioEvents.TxDone = OnRadioTxDone;
    RadioEvents.RxDone = OnRadioRxDone;
    RadioEvents.RxError = OnRadioRxError;
    RadioEvents.TxTimeout = OnRadioTxTimeout;
    RadioEvents.RxTimeout = OnRadioRxTimeout;
#if defined(CONFIG_LORA_CAD) || defined(CLASS_A_WOTA)
    RadioEvents.CadDone = OnRadioCadDone;
#endif
    Radio.Init( &RadioEvents );

    // Random seed initialization
    srand1( Radio.Random( ) );

    PublicNetwork = true;
    Radio.SetPublicNetwork(true);
    Radio.Sleep( );

    // Initialize class b
    // Apply callback
    classBCallbacks.GetTemperatureLevel = NULL;
    if( callbacks != NULL )
    {
        classBCallbacks.GetTemperatureLevel = callbacks->GetTemperatureLevel;
    }

    // Must all be static. Don't use local references.
    classBParams.MlmeIndication = &MlmeIndication;
    classBParams.McpsIndication = &McpsIndication;
    classBParams.MlmeConfirm = &MlmeConfirm;
    classBParams.LoRaMacFlags = &LoRaMacFlags;
    classBParams.LoRaMacDevAddr = &LoRaMacDevAddr;
    classBParams.LoRaMacRegion = &LoRaMacRegion;
    classBParams.MacStateCheckTimer = &MacStateCheckTimer;
    classBParams.LoRaMacParams = &LoRaMacParams;
    classBParams.MulticastChannels = &MulticastChannels;

    LoRaMacClassBInit( &classBParams, &classBCallbacks );

    return LORAMAC_STATUS_OK;
}

extern int8_t defaultDrForNoAdr;
extern int8_t currentDrForNoAdr;

LoRaMacStatus_t LoRaMacQueryTxPossible( uint8_t size, LoRaMacTxInfo_t *txInfo )
{
    AdrNextParams_t adrNext;
    GetPhyParams_t getPhy;
    PhyParam_t phyParam;
    int8_t datarate;
    int8_t txPower = LoRaMacParamsDefaults.ChannelsTxPower;
    
    // Get the minimum possible datarate
    getPhy.Attribute = PHY_MIN_TX_DR;
    phyParam = RegionGetPhyParam( LoRaMacRegion, &getPhy );
    defaultDrForNoAdr = MAX( defaultDrForNoAdr, phyParam.Value );

    // Get the mac possible datarate
    getPhy.Attribute = PHY_MAX_TX_DR;
    phyParam = RegionGetPhyParam( LoRaMacRegion, &getPhy );
    int8_t maxDatarate = phyParam.Value;
    defaultDrForNoAdr = MIN( defaultDrForNoAdr, phyParam.Value );

    currentDrForNoAdr = defaultDrForNoAdr;
    if(AdrCtrlOn)
    {
        datarate = LoRaMacParams.ChannelsDatarate;
    }
    else
    {
        datarate=currentDrForNoAdr;
    }
    uint8_t fOptLen = MacCommandsBufferIndex + MacCommandsBufferToRepeatIndex;

    if ( txInfo == NULL ) {
        return LORAMAC_STATUS_PARAMETER_INVALID;
    }



    // Setup ADR request
    adrNext.UpdateChanMask = false;
    adrNext.AdrEnabled = AdrCtrlOn;
    adrNext.AdrAckCounter = AdrAckCounter;
    //adrNext.Datarate = LoRaMacParams.ChannelsDatarate;
    adrNext.Datarate = datarate;
    adrNext.TxPower = LoRaMacParams.ChannelsTxPower;
    adrNext.UplinkDwellTime = LoRaMacParams.UplinkDwellTime;

    // We call the function for information purposes only. We don't want to
    // apply the datarate, the tx power and the ADR ack counter.
    RegionAdrNext( LoRaMacRegion, &adrNext, &datarate, &txPower, &AdrAckCounter );

/*
    // Setup PHY request
    getPhy.UplinkDwellTime = LoRaMacParams.UplinkDwellTime;
    getPhy.Datarate = datarate;
    getPhy.Attribute = PHY_MAX_PAYLOAD;

    // Change request in case repeater is supported
    if( LoRaMacParams.RepeaterSupport == true ) {
        getPhy.Attribute = PHY_MAX_PAYLOAD_REPEATER;
    }
    phyParam = RegionGetPhyParam( LoRaMacRegion, &getPhy );
    txInfo->CurrentPayloadSize = phyParam.Value;

    // Verify if the fOpts fit into the maximum payload
    if ( txInfo->CurrentPayloadSize >= fOptLen ) {
        txInfo->MaxPossiblePayload = txInfo->CurrentPayloadSize - fOptLen;
    } else {
        txInfo->MaxPossiblePayload = txInfo->CurrentPayloadSize;
        // The fOpts don't fit into the maximum payload. Omit the MAC commands to
        // ensure that another uplink is possible.
        fOptLen = 0;
        MacCommandsBufferIndex = 0;
        MacCommandsBufferToRepeatIndex = 0;
    }
*/
    // Verify if the fOpts and the payload fit into the maximum payload
    while ( ValidatePayloadLength( size, datarate, fOptLen ) == false ) {
		getPhy.UplinkDwellTime = LoRaMacParams.UplinkDwellTime;
		getPhy.Datarate = datarate;
		getPhy.Attribute = PHY_MAX_PAYLOAD;
		
		// Change request in case repeater is supported
		if( LoRaMacParams.RepeaterSupport == true ) {
			getPhy.Attribute = PHY_MAX_PAYLOAD_REPEATER;
		}
		phyParam = RegionGetPhyParam( LoRaMacRegion, &getPhy );
		uint8_t maxN = phyParam.Value;
        if(AdrCtrlOn)
        {
            if(LoRaMacParams.ChannelsDatarate >= maxDatarate)
                return LORAMAC_STATUS_LENGTH_ERROR;
            LoRaMacParams.ChannelsDatarate ++;
            datarate=LoRaMacParams.ChannelsDatarate;
        }
        else
        {
            if(currentDrForNoAdr >= maxDatarate)
                return LORAMAC_STATUS_LENGTH_ERROR;
            currentDrForNoAdr++;
            datarate=currentDrForNoAdr;
        }
		printf("Payload length(%d) and fOptLen(%d) exceed max size(%d for current datarate DR %d), set datarate to Dr %d\r\n",size,fOptLen,maxN,datarate-1,datarate);
    }
    return LORAMAC_STATUS_OK;
}

LoRaMacStatus_t LoRaMacMibGetRequestConfirm( MibRequestConfirm_t *mibGet )
{
    LoRaMacStatus_t status = LORAMAC_STATUS_OK;
    GetPhyParams_t getPhy;
    PhyParam_t phyParam;

    if ( mibGet == NULL ) {
        return LORAMAC_STATUS_PARAMETER_INVALID;
    }

    switch ( mibGet->Type ) {
        case MIB_DEVICE_CLASS: {
            mibGet->Param.Class = LoRaMacDeviceClass;
            break;
        }
        case MIB_NETWORK_JOINED: {
            mibGet->Param.IsNetworkJoined = IsLoRaMacNetworkJoined;
            break;
        }
        case MIB_ADR: {
            mibGet->Param.AdrEnable = AdrCtrlOn;
            break;
        }
        case MIB_NET_ID: {
            mibGet->Param.NetID = LoRaMacNetID;
            break;
        }
        case MIB_DEV_ADDR: {
            mibGet->Param.DevAddr = LoRaMacDevAddr;
            break;
        }
        case MIB_NWK_SKEY: {
            mibGet->Param.NwkSKey = LoRaMacNwkSKey;
            break;
        }
        case MIB_APP_SKEY: {
            mibGet->Param.AppSKey = LoRaMacAppSKey;
            break;
        }
        case MIB_PUBLIC_NETWORK: {
            mibGet->Param.EnablePublicNetwork = PublicNetwork;
            break;
        }
        case MIB_REPEATER_SUPPORT: {
            mibGet->Param.EnableRepeaterSupport = LoRaMacParams.RepeaterSupport;
            break;
        }
        case MIB_CHANNELS: {
            getPhy.Attribute = PHY_CHANNELS;
            phyParam = RegionGetPhyParam( LoRaMacRegion, &getPhy );

            mibGet->Param.ChannelList = phyParam.Channels;
            break;
        }
        case MIB_RX2_CHANNEL: {
            mibGet->Param.Rx2Channel = LoRaMacParams.Rx2Channel;
            break;
        }
        case MIB_RX2_DEFAULT_CHANNEL: {
            mibGet->Param.Rx2Channel = LoRaMacParamsDefaults.Rx2Channel;
            break;
        }
        case MIB_CHANNELS_DEFAULT_MASK: {
            getPhy.Attribute = PHY_CHANNELS_DEFAULT_MASK;
            phyParam = RegionGetPhyParam( LoRaMacRegion, &getPhy );

            mibGet->Param.ChannelsDefaultMask = phyParam.ChannelsMask;
            break;
        }
        case MIB_CHANNELS_MASK: {
            getPhy.Attribute = PHY_CHANNELS_MASK;
            phyParam = RegionGetPhyParam( LoRaMacRegion, &getPhy );

            mibGet->Param.ChannelsMask = phyParam.ChannelsMask;
            break;
        }
        case MIB_CHANNELS_NB_REP: {
            mibGet->Param.ChannelNbRep = LoRaMacParams.ChannelsNbRep;
            break;
        }
        case MIB_MAX_RX_WINDOW_DURATION: {
            mibGet->Param.MaxRxWindow = LoRaMacParams.MaxRxWindow;
            break;
        }
        case MIB_RECEIVE_DELAY_1: {
            mibGet->Param.ReceiveDelay1 = LoRaMacParams.ReceiveDelay1;
            break;
        }
        case MIB_RECEIVE_DELAY_2: {
            mibGet->Param.ReceiveDelay2 = LoRaMacParams.ReceiveDelay2;
            break;
        }
        case MIB_JOIN_ACCEPT_DELAY_1: {
            mibGet->Param.JoinAcceptDelay1 = LoRaMacParams.JoinAcceptDelay1;
            break;
        }
        case MIB_JOIN_ACCEPT_DELAY_2: {
            mibGet->Param.JoinAcceptDelay2 = LoRaMacParams.JoinAcceptDelay2;
            break;
        }
        case MIB_CHANNELS_DEFAULT_DATARATE: {
            mibGet->Param.ChannelsDefaultDatarate = LoRaMacParamsDefaults.ChannelsDatarate;
            break;
        }
        case MIB_CHANNELS_DATARATE: {
            mibGet->Param.ChannelsDatarate = LoRaMacParams.ChannelsDatarate;
            break;
        }
        case MIB_CHANNELS_DEFAULT_TX_POWER: {
            mibGet->Param.ChannelsDefaultTxPower = LoRaMacParamsDefaults.ChannelsTxPower;
            break;
        }
        case MIB_CHANNELS_TX_POWER: {
            mibGet->Param.ChannelsTxPower = LoRaMacParams.ChannelsTxPower;
            break;
        }
        case MIB_UPLINK_COUNTER: {
            mibGet->Param.UpLinkCounter = UpLinkCounter;
            break;
        }
        case MIB_DOWNLINK_COUNTER: {
            mibGet->Param.DownLinkCounter = DownLinkCounter;
            break;
        }
        case MIB_MULTICAST_CHANNEL: {
            mibGet->Param.MulticastList = MulticastChannels;
            break;
        }
        case MIB_SYSTEM_MAX_RX_ERROR: {
            mibGet->Param.SystemMaxRxError = LoRaMacParams.SystemMaxRxError;
            break;
        }
        case MIB_MIN_RX_SYMBOLS: {
            mibGet->Param.MinRxSymbols = LoRaMacParams.MinRxSymbols;
            break;
        }
        case MIB_ANTENNA_GAIN: {
            mibGet->Param.AntennaGain = LoRaMacParams.AntennaGain;
            break;
        }
#ifdef CONFIG_LWAN
        case MIB_RX1_DATARATE_OFFSET: {
            mibGet->Param.Rx1DrOffset = LoRaMacParams.Rx1DrOffset;
            break;
        }
        case MIB_MAC_STATE: {
            mibGet->Param.LoRaMacState = LoRaMacState;
            break;
        }
#endif
#ifdef CONFIG_LINKWAN
        case MIB_FREQ_BAND: {
            mibGet->Param.freqband = LoRaMacParams.freqband;
            break;
        }
#endif
        default:
            if( LoRaMacDeviceClass == CLASS_B )
            {
                status = LoRaMacClassBMibGetRequestConfirm( mibGet );
            }
            else
            {
                status = LORAMAC_STATUS_SERVICE_UNKNOWN;
            }
            break;
    }

    return status;
}

LoRaMacStatus_t LoRaMacMibSetRequestConfirm( MibRequestConfirm_t *mibSet )
{
    LoRaMacStatus_t status = LORAMAC_STATUS_OK;
    ChanMaskSetParams_t chanMaskSet;
    VerifyParams_t verify;

    if ( mibSet == NULL ) {
        return LORAMAC_STATUS_PARAMETER_INVALID;
    }
    if ( ( LoRaMacState & LORAMAC_TX_RUNNING ) == LORAMAC_TX_RUNNING ) {
        return LORAMAC_STATUS_BUSY;
    }

    switch ( mibSet->Type ) {
        case MIB_DEVICE_CLASS:
        {
            status = SwitchClass( mibSet->Param.Class );
            break;
        }
        case MIB_NETWORK_JOINED:
        {
            IsLoRaMacNetworkJoined = mibSet->Param.IsNetworkJoined;
            break;
        }
        case MIB_ADR: {
            AdrCtrlOn = mibSet->Param.AdrEnable;
            break;
        }
        case MIB_NET_ID: {
            LoRaMacNetID = mibSet->Param.NetID;
            break;
        }
        case MIB_DEV_ADDR: {
            LoRaMacDevAddr = mibSet->Param.DevAddr;
            break;
        }
        case MIB_NWK_SKEY: {
            if ( mibSet->Param.NwkSKey != NULL ) {
                memcpy1( LoRaMacNwkSKey, mibSet->Param.NwkSKey,
                         sizeof( LoRaMacNwkSKey ) );
            } else {
                status = LORAMAC_STATUS_PARAMETER_INVALID;
            }
            break;
        }
        case MIB_APP_SKEY: {
            if ( mibSet->Param.AppSKey != NULL ) {
                memcpy1( LoRaMacAppSKey, mibSet->Param.AppSKey,
                         sizeof( LoRaMacAppSKey ) );
            } else {
                status = LORAMAC_STATUS_PARAMETER_INVALID;
            }
            break;
        }
        case MIB_PUBLIC_NETWORK: {
            PublicNetwork = mibSet->Param.EnablePublicNetwork;
            Radio.SetPublicNetwork(mibSet->Param.EnablePublicNetwork);
            break;
        }
        case MIB_REPEATER_SUPPORT: {
             LoRaMacParams.RepeaterSupport = mibSet->Param.EnableRepeaterSupport;
            break;
        }
        case MIB_RX2_CHANNEL: {
            verify.DatarateParams.Datarate = mibSet->Param.Rx2Channel.Datarate;
            verify.DatarateParams.DownlinkDwellTime = LoRaMacParams.DownlinkDwellTime;

            if ( RegionVerify( LoRaMacRegion, &verify, PHY_RX_DR ) == true ) {
                memcpy(&LoRaMacParams.Rx2Channel, &mibSet->Param.Rx2Channel, sizeof(LoRaMacParams.Rx2Channel));
                if ( ( LoRaMacDeviceClass == CLASS_C ) && ( IsLoRaMacNetworkJoined == true ) ) {
                    // Compute Rx2 windows parameters
                    RegionComputeRxWindowParameters( LoRaMacRegion,
                                                     LoRaMacParams.Rx2Channel.Datarate,
                                                     LoRaMacParams.MinRxSymbols,
                                                     LoRaMacParams.SystemMaxRxError,
                                                     &RxWindow2Config );

                    RxWindow2Config.Channel = Channel;
                    RxWindow2Config.Frequency = LoRaMacParams.Rx2Channel.Frequency;
                    RxWindow2Config.DownlinkDwellTime = LoRaMacParams.DownlinkDwellTime;
                    RxWindow2Config.RepeaterSupport = LoRaMacParams.RepeaterSupport;
                    RxWindow2Config.RxSlot = RX_SLOT_WIN_2;
                    RxWindow2Config.RxContinuous = true;

                    Radio.Sleep();
                    if ( RegionRxConfig( LoRaMacRegion, &RxWindow2Config, ( int8_t * )&McpsIndication.RxDatarate ) == true ) {
                        RxWindowSetup( RxWindow2Config.RxContinuous, LoRaMacParams.MaxRxWindow );
                        RxSlot = RxWindow2Config.RxSlot;
                    } else {
                        status = LORAMAC_STATUS_PARAMETER_INVALID;
                    }
                }
            } else {
                status = LORAMAC_STATUS_PARAMETER_INVALID;
            }
            break;
        }
        case MIB_RX2_DEFAULT_CHANNEL: {
            verify.DatarateParams.Datarate = mibSet->Param.Rx2Channel.Datarate;
            verify.DatarateParams.DownlinkDwellTime = LoRaMacParams.DownlinkDwellTime;

            if ( RegionVerify( LoRaMacRegion, &verify, PHY_RX_DR ) == true ) {
                LoRaMacParamsDefaults.Rx2Channel = mibSet->Param.Rx2DefaultChannel;
            } else {
                status = LORAMAC_STATUS_PARAMETER_INVALID;
            }
            break;
        }
        case MIB_CHANNELS_DEFAULT_MASK: {
            chanMaskSet.ChannelsMaskIn = mibSet->Param.ChannelsMask;
            chanMaskSet.ChannelsMaskType = CHANNELS_DEFAULT_MASK;

            if ( RegionChanMaskSet( LoRaMacRegion, &chanMaskSet ) == false ) {
                status = LORAMAC_STATUS_PARAMETER_INVALID;
            }
            break;
        }
        case MIB_CHANNELS_MASK: {
            chanMaskSet.ChannelsMaskIn = mibSet->Param.ChannelsMask;
            chanMaskSet.ChannelsMaskType = CHANNELS_MASK;

            if ( RegionChanMaskSet( LoRaMacRegion, &chanMaskSet ) == false ) {
                status = LORAMAC_STATUS_PARAMETER_INVALID;
            }
            break;
        }
        case MIB_CHANNELS_NB_REP: {
            if ( ( mibSet->Param.ChannelNbRep >= 1 ) &&
                 ( mibSet->Param.ChannelNbRep <= 15 ) ) {
                LoRaMacParams.ChannelsNbRep = mibSet->Param.ChannelNbRep;
            } else {
                status = LORAMAC_STATUS_PARAMETER_INVALID;
            }
            break;
        }
        case MIB_MAX_RX_WINDOW_DURATION: {
            LoRaMacParams.MaxRxWindow = mibSet->Param.MaxRxWindow;
            break;
        }
        case MIB_RECEIVE_DELAY_1: {
            LoRaMacParams.ReceiveDelay1 = mibSet->Param.ReceiveDelay1;
            break;
        }
        case MIB_RECEIVE_DELAY_2: {
            LoRaMacParams.ReceiveDelay2 = mibSet->Param.ReceiveDelay2;
            break;
        }
        case MIB_JOIN_ACCEPT_DELAY_1: {
            LoRaMacParams.JoinAcceptDelay1 = mibSet->Param.JoinAcceptDelay1;
            break;
        }
        case MIB_JOIN_ACCEPT_DELAY_2: {
            LoRaMacParams.JoinAcceptDelay2 = mibSet->Param.JoinAcceptDelay2;
            break;
        }
        case MIB_CHANNELS_DEFAULT_DATARATE: {
            verify.DatarateParams.Datarate = mibSet->Param.ChannelsDefaultDatarate;

            if ( RegionVerify( LoRaMacRegion, &verify, PHY_DEF_TX_DR ) == true ) {
                LoRaMacParamsDefaults.ChannelsDatarate = verify.DatarateParams.Datarate;
            } else {
                status = LORAMAC_STATUS_PARAMETER_INVALID;
            }
            break;
        }
        case MIB_CHANNELS_DATARATE: {
            verify.DatarateParams.Datarate = mibSet->Param.ChannelsDatarate;

            if ( RegionVerify( LoRaMacRegion, &verify, PHY_TX_DR ) == true ) {
                LoRaMacParams.ChannelsDatarate = verify.DatarateParams.Datarate;
            } else {
                status = LORAMAC_STATUS_PARAMETER_INVALID;
            }
            break;
        }
        case MIB_CHANNELS_DEFAULT_TX_POWER: {
            verify.TxPower = mibSet->Param.ChannelsDefaultTxPower;

            if ( RegionVerify( LoRaMacRegion, &verify, PHY_DEF_TX_POWER ) == true ) {
                LoRaMacParamsDefaults.ChannelsTxPower = verify.TxPower;
            } else {
                status = LORAMAC_STATUS_PARAMETER_INVALID;
            }
            break;
        }
        case MIB_CHANNELS_TX_POWER: {
            verify.TxPower = mibSet->Param.ChannelsTxPower;

            if ( RegionVerify( LoRaMacRegion, &verify, PHY_TX_POWER ) == true ) {
                LoRaMacParams.ChannelsTxPower = verify.TxPower;
            } else {
                status = LORAMAC_STATUS_PARAMETER_INVALID;
            }
            break;
        }
        case MIB_UPLINK_COUNTER: {
            UpLinkCounter = mibSet->Param.UpLinkCounter;
            break;
        }
        case MIB_DOWNLINK_COUNTER: {
            DownLinkCounter = mibSet->Param.DownLinkCounter;
            break;
        }
        case MIB_SYSTEM_MAX_RX_ERROR: {
            LoRaMacParams.SystemMaxRxError = LoRaMacParamsDefaults.SystemMaxRxError = mibSet->Param.SystemMaxRxError;
            break;
        }
        case MIB_MIN_RX_SYMBOLS: {
            LoRaMacParams.MinRxSymbols = LoRaMacParamsDefaults.MinRxSymbols = mibSet->Param.MinRxSymbols;
            break;
        }
        case MIB_ANTENNA_GAIN: {
            LoRaMacParams.AntennaGain = mibSet->Param.AntennaGain;
            break;
        }
        case MIB_MULTICAST_CHANNEL: {
            status = LoRaMacMulticastChannelLink(mibSet->Param.MulticastList);
            break;
        }
        case MIB_MULTICAST_CHANNEL_DEL: {
            status = LoRaMacMulticastChannelUnlink(mibSet->Param.MulticastList);
            break;
        }
#ifdef CONFIG_LWAN
        case MIB_RX1_DATARATE_OFFSET: {
            LoRaMacParams.Rx1DrOffset = mibSet->Param.Rx1DrOffset;
            break;
        }
#endif

#ifdef CONFIG_LINKWAN
        case MIB_FREQ_BAND: {
            LoRaMacParams.freqband = mibSet->Param.freqband;
            break;
        }
#endif        
        default:
        {
            if( LoRaMacDeviceClass == CLASS_B )
            {
                status = LoRaMacMibClassBSetRequestConfirm( mibSet );
            }
            else
            {
                status = LORAMAC_STATUS_SERVICE_UNKNOWN;
            }
            break;
        }
    }

    return status;
}

LoRaMacStatus_t LoRaMacChannelAdd( uint8_t id, ChannelParams_t params )
{
    ChannelAddParams_t channelAdd;

    // Validate if the MAC is in a correct state
    if ( ( LoRaMacState & LORAMAC_TX_RUNNING ) == LORAMAC_TX_RUNNING ) {
        if ( ( LoRaMacState & LORAMAC_TX_CONFIG ) != LORAMAC_TX_CONFIG ) {
            return LORAMAC_STATUS_BUSY;
        }
    }

    channelAdd.NewChannel = &params;
    channelAdd.ChannelId = id;

    return RegionChannelAdd( LoRaMacRegion, &channelAdd );
}

LoRaMacStatus_t LoRaMacChannelRemove( uint8_t id )
{
    ChannelRemoveParams_t channelRemove;

    if ( ( LoRaMacState & LORAMAC_TX_RUNNING ) == LORAMAC_TX_RUNNING ) {
        if ( ( LoRaMacState & LORAMAC_TX_CONFIG ) != LORAMAC_TX_CONFIG ) {
            return LORAMAC_STATUS_BUSY;
        }
    }

    channelRemove.ChannelId = id;

    if ( RegionChannelsRemove( LoRaMacRegion, &channelRemove ) == false ) {
        return LORAMAC_STATUS_PARAMETER_INVALID;
    }
    return LORAMAC_STATUS_OK;
}

LoRaMacStatus_t LoRaMacMulticastChannelLink( MulticastParams_t *channelParam )
{
    if ( channelParam == NULL ) {
        return LORAMAC_STATUS_PARAMETER_INVALID;
    }
    if ( ( LoRaMacState & LORAMAC_TX_RUNNING ) == LORAMAC_TX_RUNNING ) {
        return LORAMAC_STATUS_BUSY;
    }

    // Calculate class b parameters
    LoRaMacClassBSetMulticastPeriodicity( channelParam );

    // Reset downlink counter
    channelParam->DownLinkCounter = 0;
    channelParam->Next = NULL;

    if ( MulticastChannels == NULL ) {
        // New node is the fist element
        MulticastChannels = channelParam;
    } else {
        MulticastParams_t *cur = MulticastChannels;

        // Search the last node in the list
        while ( cur->Next != NULL ) {
            cur = cur->Next;
        }
        // This function always finds the last node
        cur->Next = channelParam;
    }

    return LORAMAC_STATUS_OK;
}

LoRaMacStatus_t LoRaMacMulticastChannelUnlink( MulticastParams_t *channelParam )
{
    if ( channelParam == NULL ) {
        return LORAMAC_STATUS_PARAMETER_INVALID;
    }
    if ( ( LoRaMacState & LORAMAC_TX_RUNNING ) == LORAMAC_TX_RUNNING ) {
        return LORAMAC_STATUS_BUSY;
    }

    if ( MulticastChannels != NULL ) {
        if ( MulticastChannels == channelParam ) {
            // First element
            MulticastChannels = channelParam->Next;
        } else {
            MulticastParams_t *cur = MulticastChannels;

            // Search the node in the list
            while ( cur->Next && cur->Next != channelParam ) {
                cur = cur->Next;
            }
            // If we found the node, remove it
            if ( cur->Next ) {
                cur->Next = channelParam->Next;
            }
        }
        channelParam->Next = NULL;
    }

    return LORAMAC_STATUS_OK;
}

LoRaMacStatus_t LoRaMacMlmeRequest( MlmeReq_t *mlmeRequest )
{
    LoRaMacStatus_t status = LORAMAC_STATUS_SERVICE_UNKNOWN;
    LoRaMacHeader_t macHdr;
    MlmeConfirmQueue_t queueElement;
    AlternateDrParams_t altDr;
    VerifyParams_t verify;
    GetPhyParams_t getPhy;
    PhyParam_t phyParam;

    if ( mlmeRequest == NULL ) {
        return LORAMAC_STATUS_PARAMETER_INVALID;
    }
    if( LoRaMacState != LORAMAC_IDLE )
    {
        return LORAMAC_STATUS_BUSY;
    }
    if( LoRaMacConfirmQueueIsFull( ) == true )
    {
        return LORAMAC_STATUS_BUSY;
    }


    switch ( mlmeRequest->Type ) {
        case MLME_JOIN: {
            if ( ( mlmeRequest->Req.Join.DevEui == NULL ) ||
                 ( mlmeRequest->Req.Join.AppEui == NULL ) ||
                 ( mlmeRequest->Req.Join.AppKey == NULL )) {
                return LORAMAC_STATUS_PARAMETER_INVALID;
            }

            // Verify the parameter NbTrials for the join procedure
            verify.NbJoinTrials = mlmeRequest->Req.Join.NbTrials;

            if ( RegionVerify( LoRaMacRegion, &verify, PHY_NB_JOIN_TRIALS ) == false ) {
                // Value not supported, get default
                getPhy.Attribute = PHY_DEF_NB_JOIN_TRIALS;
                phyParam = RegionGetPhyParam( LoRaMacRegion, &getPhy );
                mlmeRequest->Req.Join.NbTrials = ( uint8_t ) phyParam.Value;
            }

            LoRaMacFlags.Bits.MlmeReq = 1;
            queueElement.Request = mlmeRequest->Type;

            LoRaMacDevEui = mlmeRequest->Req.Join.DevEui;
            LoRaMacAppEui = mlmeRequest->Req.Join.AppEui;
            LoRaMacAppKey = mlmeRequest->Req.Join.AppKey;
            queueElement.Status = LORAMAC_EVENT_INFO_STATUS_JOIN_FAIL;
            queueElement.RestrictCommonReadyToHandle = false;
            LoRaMacConfirmQueueAdd( &queueElement );
            MaxJoinRequestTrials = mlmeRequest->Req.Join.NbTrials;

            // Reset variable JoinRequestTrials
            JoinRequestTrials = 0;

            // Setup header information
            macHdr.Value = 0;
            macHdr.Bits.MType  = FRAME_TYPE_JOIN_REQ;

            ResetMacParameters( );

            altDr.NbTrials = JoinRequestTrials + 1;
#ifdef CONFIG_LINKWAN
            altDr.joinmethod = mlmeRequest->Req.Join.method;
            altDr.datarate = mlmeRequest->Req.Join.datarate;
            LoRaMacParams.method = altDr.joinmethod;
            LoRaMacParams.freqband = mlmeRequest->Req.Join.freqband;
            LoRaMacParams.update_freqband = true;
#endif
            LoRaMacParams.ChannelsDatarate = RegionAlternateDr( LoRaMacRegion, &altDr );
#ifdef CONFIG_LORA_VERIFY
            if (g_lora_debug == true)
                PRINTF_RAW("MacHdr major:%d rfu:%d mtype:%d\r\n", macHdr.Bits.Major, macHdr.Bits.RFU, macHdr.Bits.MType);
#endif
            status = Send( &macHdr, 0, NULL, 0 );
            break;
        }
        case MLME_LINK_CHECK: {
            // Apply the request
            LoRaMacFlags.Bits.MlmeReq = 1;
            queueElement.Request = mlmeRequest->Type;
            queueElement.Status = LORAMAC_EVENT_INFO_STATUS_ERROR;
            queueElement.RestrictCommonReadyToHandle = false;
            LoRaMacConfirmQueueAdd( &queueElement );

            // LoRaMac will send this command piggy-pack
            status = AddMacCommand( MOTE_MAC_LINK_CHECK_REQ, 0, 0 );
            break;
        }
        case MLME_TXCW: {
            // Apply the request
            LoRaMacFlags.Bits.MlmeReq = 1;
            queueElement.Request = mlmeRequest->Type;
            queueElement.Status = LORAMAC_EVENT_INFO_STATUS_ERROR;
            queueElement.RestrictCommonReadyToHandle = false;
            LoRaMacConfirmQueueAdd( &queueElement );

            status = SetTxContinuousWave( mlmeRequest->Req.TxCw.Timeout );
            break;
        }
        case MLME_TXCW_1:
        {
            // Apply the request
            LoRaMacFlags.Bits.MlmeReq = 1;
            queueElement.Request = mlmeRequest->Type;
            queueElement.Status = LORAMAC_EVENT_INFO_STATUS_ERROR;
            queueElement.RestrictCommonReadyToHandle = false;
            LoRaMacConfirmQueueAdd( &queueElement );

            status = SetTxContinuousWave1( mlmeRequest->Req.TxCw.Timeout, mlmeRequest->Req.TxCw.Frequency, mlmeRequest->Req.TxCw.Power );
            break;
        }
        case MLME_DEVICE_TIME:
        {
            // Apply the request
            LoRaMacFlags.Bits.MlmeReq = 1;
            queueElement.Request = mlmeRequest->Type;
            queueElement.Status = LORAMAC_EVENT_INFO_STATUS_ERROR;
            queueElement.RestrictCommonReadyToHandle = false;
            LoRaMacConfirmQueueAdd( &queueElement );

            // LoRaMac will send this command piggy-pack
            status = AddMacCommand( MOTE_MAC_DEVICE_TIME_REQ, 0, 0 );
            break;
        }
        case MLME_PING_SLOT_INFO:
        {
            uint8_t value = mlmeRequest->Req.PingSlotInfo.PingSlot.Value;

            // Apply the request
            LoRaMacFlags.Bits.MlmeReq = 1;
            queueElement.Request = mlmeRequest->Type;
            queueElement.Status = LORAMAC_EVENT_INFO_STATUS_ERROR;
            queueElement.RestrictCommonReadyToHandle = false;
            LoRaMacConfirmQueueAdd( &queueElement );

            // LoRaMac will send this command piggy-pack
            LoRaMacClassBSetPingSlotInfo( mlmeRequest->Req.PingSlotInfo.PingSlot.Fields.Periodicity );

            status = AddMacCommand( MOTE_MAC_PING_SLOT_INFO_REQ, value, 0 );
            break;
        }
        case MLME_BEACON_TIMING:
        {
            // Apply the request
            LoRaMacFlags.Bits.MlmeReq = 1;
            queueElement.Request = mlmeRequest->Type;
            queueElement.Status = LORAMAC_EVENT_INFO_STATUS_ERROR;
            queueElement.RestrictCommonReadyToHandle = false;
            LoRaMacConfirmQueueAdd( &queueElement );

            // LoRaMac will send this command piggy-pack
            status = AddMacCommand( MOTE_MAC_BEACON_TIMING_REQ, 0, 0 );
            break;
        }
        case MLME_BEACON_ACQUISITION:
        {
            // Apply the request
            LoRaMacFlags.Bits.MlmeReq = 1;
            queueElement.Request = mlmeRequest->Type;
            queueElement.Status = LORAMAC_EVENT_INFO_STATUS_ERROR;
            queueElement.RestrictCommonReadyToHandle = true;
            LoRaMacConfirmQueueAdd( &queueElement );

            if( LoRaMacClassBIsAcquisitionInProgress( ) == false )
            {
                // Start class B algorithm
                LoRaMacClassBSetBeaconState( BEACON_STATE_ACQUISITION );
                LoRaMacClassBBeaconTimerEvent( );

                status = LORAMAC_STATUS_OK;
            }
            else
            {
                status = LORAMAC_STATUS_BUSY;
            }
            break;
        }
        default:
            break;
    }

    if ( status != LORAMAC_STATUS_OK ) {
        NodeAckRequested = false;
        LoRaMacConfirmQueueRemoveLast( );
        if( LoRaMacConfirmQueueGetCnt( ) == 0 )
        {
            LoRaMacFlags.Bits.MlmeReq = 0;
        }
    }

    return status;
}

LoRaMacStatus_t LoRaMacMcpsRequest( McpsReq_t *mcpsRequest )
{


    GetPhyParams_t getPhy;
    PhyParam_t phyParam;
    LoRaMacStatus_t status = LORAMAC_STATUS_SERVICE_UNKNOWN;
    LoRaMacHeader_t macHdr;
    VerifyParams_t verify;
    uint8_t fPort = 0;
    void *fBuffer;
    uint16_t fBufferSize;
    int8_t datarate;
    bool readyToSend = false;

#ifdef CONFIG_LORA_VERIFY
	if (g_lora_debug) {
		mcps_start_time = TimerGetCurrentTime( );
	}
#endif
    if ( mcpsRequest == NULL ) {
        return LORAMAC_STATUS_PARAMETER_INVALID;
    }
    if ( ( ( LoRaMacState & LORAMAC_TX_RUNNING ) == LORAMAC_TX_RUNNING ) ||
         ( ( LoRaMacState & LORAMAC_TX_DELAYED ) == LORAMAC_TX_DELAYED ) ) {
        return LORAMAC_STATUS_BUSY;
    }

    macHdr.Value = 0;
    memset1 ( ( uint8_t * ) &McpsConfirm, 0, sizeof( McpsConfirm ) );
    McpsConfirm.Status = LORAMAC_EVENT_INFO_STATUS_ERROR;

    // AckTimeoutRetriesCounter must be reset every time a new request (unconfirmed or confirmed) is performed.
    AckTimeoutRetriesCounter = 1;

    switch ( mcpsRequest->Type ) {
        case MCPS_UNCONFIRMED: {
            readyToSend = true;
            AckTimeoutRetries = 1;

            macHdr.Bits.MType = FRAME_TYPE_DATA_UNCONFIRMED_UP;
            fPort = mcpsRequest->Req.Unconfirmed.fPort;
            fBuffer = mcpsRequest->Req.Unconfirmed.fBuffer;
            fBufferSize = mcpsRequest->Req.Unconfirmed.fBufferSize;
            datarate = mcpsRequest->Req.Unconfirmed.Datarate;
            break;
        }
        case MCPS_CONFIRMED: {
            readyToSend = true;
            AckTimeoutRetries = mcpsRequest->Req.Confirmed.NbTrials;

            macHdr.Bits.MType = FRAME_TYPE_DATA_CONFIRMED_UP;
            fPort = mcpsRequest->Req.Confirmed.fPort;
            fBuffer = mcpsRequest->Req.Confirmed.fBuffer;
            fBufferSize = mcpsRequest->Req.Confirmed.fBufferSize;
            datarate = mcpsRequest->Req.Confirmed.Datarate;
            break;
        }
        case MCPS_PROPRIETARY: {
            readyToSend = true;
            AckTimeoutRetries = 1;

            macHdr.Bits.MType = FRAME_TYPE_PROPRIETARY;
            fBuffer = mcpsRequest->Req.Proprietary.fBuffer;
            fBufferSize = mcpsRequest->Req.Proprietary.fBufferSize;
            datarate = mcpsRequest->Req.Proprietary.Datarate;
            break;
        }
        default:
            break;
    }

    // Filter fPorts
    if( IsFPortAllowed( fPort ) == false )
    {
        return LORAMAC_STATUS_PARAMETER_INVALID;
    }

    // Get the minimum possible datarate
    getPhy.Attribute = PHY_MIN_TX_DR;
    getPhy.UplinkDwellTime = LoRaMacParams.UplinkDwellTime;
    phyParam = RegionGetPhyParam( LoRaMacRegion, &getPhy );
    // Apply the minimum possible datarate.
    // Some regions have limitations for the minimum datarate.
    datarate = MAX( datarate, phyParam.Value );

    // Get the mac possible datarate
    getPhy.Attribute = PHY_MAX_TX_DR;
    getPhy.UplinkDwellTime = LoRaMacParams.UplinkDwellTime;
    phyParam = RegionGetPhyParam( LoRaMacRegion, &getPhy );
    // Apply the max possible datarate.
    // Some regions have limitations for the max datarate.
    datarate = MIN( datarate, phyParam.Value );

    if ( readyToSend == true ) {
        if ( AdrCtrlOn == false ) {
            verify.DatarateParams.Datarate = datarate;
            verify.DatarateParams.UplinkDwellTime = LoRaMacParams.UplinkDwellTime;

            if ( RegionVerify( LoRaMacRegion, &verify, PHY_TX_DR ) == true ) {
                LoRaMacParams.ChannelsDatarate = verify.DatarateParams.Datarate;
            } else {
                return LORAMAC_STATUS_PARAMETER_INVALID;
            }
        }

        status = Send( &macHdr, fPort, fBuffer, fBufferSize );
        if ( status == LORAMAC_STATUS_OK ) {
            McpsConfirm.McpsRequest = mcpsRequest->Type;
            LoRaMacFlags.Bits.McpsReq = 1;
        } else {
            NodeAckRequested = false;
        }
    }
    return status;
}

void LoRaMacTestRxWindowsOn( bool enable )
{
    IsRxWindowsEnabled = enable;
}

void LoRaMacTestSetMic( uint16_t txPacketCounter )
{
    UpLinkCounter = txPacketCounter;
    IsUpLinkCounterFixed = true;
}

void LoRaMacTestSetDutyCycleOn( bool enable )
{
    VerifyParams_t verify;

    verify.DutyCycle = enable;

    if ( RegionVerify( LoRaMacRegion, &verify, PHY_DUTY_CYCLE ) == true ) {
        DutyCycleOn = enable;
    }
}

void LoRaMacTestSetChannel( uint8_t channel )
{
    Channel = channel;
}



#if 0
static void SetPublicNetwork( bool enable )
{
    PublicNetwork = enable;
    Radio.SetModem( MODEM_LORA );
    if ( PublicNetwork == true ) {
        // Change LoRa modem SyncWord
        Radio.SetSyncWord( LORA_MAC_PUBLIC_SYNCWORD );
    } else {
        // Change LoRa modem SyncWord
        Radio.SetSyncWord( LORA_MAC_PRIVATE_SYNCWORD );
    }
}
#endif
