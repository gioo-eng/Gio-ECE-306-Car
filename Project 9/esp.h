#ifndef ESP_H
#define ESP_H

#include <stdint.h>

#define ESP_MAC_ADDRESS   "84:1f:e8:02:26:10"
#define ESP_IP_ADDRESS    "172.20.10.2"

#define ESP_TCP_PORT      (1985U)   /* TCP server port  (choose 1024–65535) */
#define ESP_TCP_PORT_STR  "1985"    /* String form used in AT+CIPSERVER cmd  */
#define ESP_HOSTNAME      "Gio Car"
#define ESP_WIFI_SSID     "ncsu"
#define ESP_WIFI_PASSWORD "1234"

/*===========================================================================
 *  Incoming command protocol
 *  -----------------------------------------------------------------------
 *  Commands arrive from the Java TCP Client wrapped in an +IPD frame:
 *
 *      +IPD,<conn>,<len>:^<PIN><DIR><UNITS>
 *
 *  Field breakdown:
 *    ^       Start-of-command sentinel character (ESP_CMD_START_CHAR)
 *    PIN     4-character secret PIN  (must match ESP_COMMAND_PIN)
 *    DIR     Direction:  F=Forward  B=Backward  R=Right  L=Left
 *    UNITS   Integer value:
 *              F / B  → time in seconds
 *              R / L  → angle in degrees
 *
 *  Examples:
 *    ^1234F2   Go forward 2 seconds
 *    ^1234B1   Go backward 1 second
 *    ^1234R90  Turn right 90 degrees
 *    ^1234L45  Turn left  45 degrees
 *===========================================================================*/
#define ESP_CMD_START_CHAR  '^'     /* Command sentinel — can be any character */
#define ESP_COMMAND_PIN     "1234"  /* 4-char secret PIN — change before demo  */

/*===========================================================================
 *  AT Command Identifiers
 *  One entry per command in the esp_commands[] table (esp.c).
 *  Keep this enum in sync with that table.
 *===========================================================================*/
typedef enum
{
    ESP_CMD_CHECK_COMM       = 0,   /* AT                                    */
    ESP_CMD_HARD_RESET,             /* AT+RESTORE  (factory defaults)        */
    ESP_CMD_SOFT_RESET,             /* AT+RST      (soft reboot)             */
    ESP_CMD_GET_VERSION,            /* AT+GMR      (firmware version)        */
    ESP_CMD_CHECK_STATUS,           /* AT+CIPSTATUS                          */
    ESP_CMD_CHECK_UART,             /* AT+UART_CUR?                          */
    ESP_CMD_SYSSTORE_ON,            /* AT+SYSSTORE=1  (begin saving to flash)*/
    ESP_CMD_SET_STATION_MODE,       /* AT+CWMODE=1    (station / client mode)*/
    ESP_CMD_GET_IP_MAC,             /* AT+CIFSR       (IP + MAC)             */
    ESP_CMD_GET_MAC,                /* AT+CIPSTAMAC?  (MAC only)             */
    ESP_CMD_LIST_NETWORKS,          /* AT+CWLAP       (scan for APs)         */
    ESP_CMD_CONNECT_WIFI,           /* AT+CWJAP="<ssid>","<pass>"            */
    ESP_CMD_SET_HOSTNAME,           /* AT+CWHOSTNAME="<name>"                */
    ESP_CMD_AUTO_CONNECT,           /* AT+CWAUTOCONN=1                       */
    ESP_CMD_RECONN_CFG,             /* AT+CWRECONNCFG=5,100                  */
    ESP_CMD_SYSSTORE_OFF,           /* AT+SYSSTORE=0  (stop saving to flash) */
    ESP_CMD_MULTI_CONN,             /* AT+CIPMUX=1    (allow multi clients)  */
    ESP_CMD_CREATE_SERVER,          /* AT+CIPSERVER=1,<port>                 */
    ESP_CMD_CHECK_LINK_STATUS,      /* AT+STATUS?                            */
    ESP_CMD_CHECK_CONN_STATE,       /* AT+CWSTATE?                           */
    ESP_CMD_PING,                   /* AT+PING="8.8.8.8"                     */
    ESP_CMD_COUNT                   /* Sentinel — do not use as a command ID */
} ESPCommandID;

/*===========================================================================
 *  AT Command Table Entry
 *===========================================================================*/
typedef struct
{
    ESPCommandID  id;
    const char   *cmd;         /* AT command string — NO trailing CR/LF;
                                  uart_send_buf() appends \r\n for you.   */
    const char   *description; /* Human-readable label for logging/display */
} ESPCommand;

/* The table is defined in esp.c */
extern const ESPCommand esp_commands[ESP_CMD_COUNT];

/*===========================================================================
 *  Incoming WiFi Command Event
 *  Populated by ESP_ParseIPDFrame() when a valid command frame arrives.
 *===========================================================================*/
typedef enum
{
    ESP_DIR_NONE    = 0,
    ESP_DIR_FORWARD = 'F',
    ESP_DIR_REVERSE = 'B',
    ESP_DIR_RIGHT   = 'R',
    ESP_DIR_LEFT    = 'L'
} ESPDirection;

typedef struct
{
    char          pin[5];      /* Received 4-char PIN + null terminator    */
    ESPDirection  direction;   /* Parsed direction (F / B / R / L)         */
    unsigned int  time_units;  /* Seconds for F/B;  degrees for R/L        */
    unsigned char valid;       /* 1 = parse succeeded and PIN matched      */
} ESPCommandEvent;

/*===========================================================================
 *  Public API
 *===========================================================================*/

/* Initialise internal state and perform the IOT_EN hardware reset — call once */
void ESP_Init(void);

/* Startup state — exposed here so the menu can display progress         */
typedef enum
{
    ESP_STARTUP_WAIT_READY = 0,   /* Waiting for ESP32 to finish booting   */
    ESP_STARTUP_WAIT_WIFI,        /* Waiting for WiFi / CWSTATE=2          */
    ESP_STARTUP_WAIT_CIPMUX_OK,   /* Sent AT+CIPMUX=1, waiting for OK      */
    ESP_STARTUP_WAIT_SERVER_OK,   /* Sent AT+CIPSERVER=1,port, waiting OK  */
    ESP_STARTUP_DONE              /* Server is up — normal operation       */
} ESPStartupState;

/* Call from the main loop for every frame received from the ESP32.
   Watches for boot / WiFi-connected messages and automatically sends
   AT+CIPMUX=1 and AT+CIPSERVER=1,<port> when the IP is assigned.
   Returns immediately (no overhead) once the server is running.         */
void ESP_ProcessStartup(const char *frame);

/* Returns the current startup state — used by the menu for display.     */
ESPStartupState ESP_GetStartupState(void);

/* Transmit an AT command from the table over UCA0 (IOT UART) */
void ESP_SendCommand(ESPCommandID id);

/* Parse a raw UCA0 frame into an ESPCommandEvent.
   Returns 1 on success (valid command + PIN matched), 0 otherwise. */
uint8_t ESP_ParseIPDFrame(const char *frame, ESPCommandEvent *out);

/* Translate a validated event into robot chain commands and schedule them */
void ESP_ScheduleEvent(const ESPCommandEvent *evt);

/* Single-slot pending-event buffer (used by main loop and menu display) */
void                   ESP_SetPendingEvent(const ESPCommandEvent *evt);
unsigned char          ESP_HasPendingEvent(void);
const ESPCommandEvent *ESP_GetPendingEvent(void);
void                   ESP_ConsumePendingEvent(void);

/*===========================================================================
 *  Command queue
 *  -----------------------------------------------------------------------
 *  ESP_EnqueueFromFrame  scans a raw UCA0 frame for every '^' sentinel and
 *  pushes each valid PIN-matched command onto a FIFO queue.  Multiple
 *  commands in a single TCP message (e.g. "^1234F2^1234B2") are all captured
 *  in one call.
 *
 *  ESP_DequeueCommand    pops the oldest command — call only when
 *                        isRobotBusy() is false so commands execute in order
 *                        without interrupting each other.
 *===========================================================================*/
#define ESP_CMD_QUEUE_SIZE  8U          /* max commands buffered at once    */

uint8_t ESP_EnqueueFromFrame(const char *frame);   /* returns # enqueued   */
uint8_t ESP_HasQueuedCommand(void);
uint8_t ESP_DequeueCommand(ESPCommandEvent *out);  /* 1 = got one, 0 = empty */

#endif // ESP_h