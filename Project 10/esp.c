#include <msp430.h>
#include "esp.h"
#include "serial.h"
#include "ports.h"
#include <string.h>
#include "motors.h"
#include "macros.h"
#include "functions.h"

extern volatile int system_running;
extern volatile int timer_seconds;
extern volatile int menu_mode;


const ESPCommand esp_commands[ESP_CMD_COUNT] =
{
    { ESP_CMD_CHECK_COMM,        "AT",                                              "Check comm"        },
    { ESP_CMD_HARD_RESET,        "AT+RESTORE",                                      "Hard reset"        },
    { ESP_CMD_SOFT_RESET,        "AT+RST",                                          "Soft reset"        },
    { ESP_CMD_GET_VERSION,       "AT+GMR",                                          "Get version"       },
    { ESP_CMD_CHECK_STATUS,      "AT+CIPSTATUS",                                    "CIP status"        },
    { ESP_CMD_CHECK_UART,        "AT+UART_CUR?",                                    "UART status"       },
    { ESP_CMD_SYSSTORE_ON,       "AT+SYSSTORE=1",                                   "Save settings ON"  },
    { ESP_CMD_SET_STATION_MODE,  "AT+CWMODE=1",                                     "Station mode"      },
    { ESP_CMD_GET_IP_MAC,        "AT+CIFSR",                                        "Get IP/MAC"        },
    { ESP_CMD_GET_MAC,           "AT+CIPSTAMAC?",                                   "Get MAC"           },
    { ESP_CMD_LIST_NETWORKS,     "AT+CWLAP",                                        "List networks"     },
    { ESP_CMD_CONNECT_WIFI,      "AT+CWJAP=\"" ESP_WIFI_SSID "\",\"" ESP_WIFI_PASSWORD "\"", "Connect WiFi" },
    { ESP_CMD_SET_HOSTNAME,      "AT+CWHOSTNAME=\"" ESP_HOSTNAME "\"",              "Set hostname"      },
    { ESP_CMD_AUTO_CONNECT,      "AT+CWAUTOCONN=1",                                 "Auto-connect ON"   },
    { ESP_CMD_RECONN_CFG,        "AT+CWRECONNCFG=5,100",                            "Reconn cfg 5,100"  },
    { ESP_CMD_SYSSTORE_OFF,      "AT+SYSSTORE=0",                                   "Save settings OFF" },
    { ESP_CMD_MULTI_CONN,        "AT+CIPMUX=1",                                     "Multi-conn ON"     },
    { ESP_CMD_CREATE_SERVER,     "AT+CIPSERVER=1," ESP_TCP_PORT_STR,                "Create TCP server" },
    { ESP_CMD_CHECK_LINK_STATUS, "AT+STATUS?",                                      "Link status"       },
    { ESP_CMD_CHECK_CONN_STATE,  "AT+CWSTATE?",                                     "Conn state"        },
    { ESP_CMD_PING,              "AT+PING=\"8.8.8.8\"",                             "Ping 8.8.8.8"      }
};

static ESPStartupState s_startup_state = ESP_STARTUP_WAIT_READY;

static ESPCommandEvent s_pending_event;
static unsigned char   s_has_pending = 0U;

#define ESP_CMD_Q_MASK  (ESP_CMD_QUEUE_SIZE - 1U)

static ESPCommandEvent s_cmd_queue[ESP_CMD_QUEUE_SIZE];
static uint8_t         s_q_head = 0U;
static uint8_t         s_q_tail = 0U;

void ESP_Init(void)
{
    s_has_pending         = 0U;
    s_pending_event.valid = 0U;
    s_q_head              = 0U;
    s_q_tail              = 0U;

    // Hard reset the ESP module
    P3OUT &= ~IOT_EN;
    __delay_cycles(800000UL);
    P3OUT |= IOT_EN;

    __delay_cycles(8000000UL);
    __delay_cycles(8000000UL);

    // --- CHANGED: Wait for the ESP to finish booting ---
    s_startup_state = ESP_STARTUP_WAIT_READY; 
}

void ESP_ProcessStartup(const char *frame)
{
    if (!frame) { return; }
    if (s_startup_state == ESP_STARTUP_DONE) { return; }

    if (s_startup_state == ESP_STARTUP_WAIT_READY)
    {
        if (strstr(frame, "ready") != 0)
        {
            s_startup_state = ESP_STARTUP_WAIT_WIFI;
            ESP_SendCommand(ESP_CMD_CONNECT_WIFI); 
        }
    }
    else if (s_startup_state == ESP_STARTUP_WAIT_WIFI)
    {
        // Wait until the WiFi connection command is COMPLETELY finished.
        // It will say "WIFI CONNECTED", then "WIFI GOT IP", and finally "OK".
        // We only trigger when we see the "OK".
        if (strstr(frame, "OK") != 0)
        {
            // Now the ESP is idle and ready to listen again. Safe to send CIPMUX!
            ESP_SendCommand(ESP_CMD_MULTI_CONN); 
            s_startup_state = ESP_STARTUP_WAIT_CIPMUX_OK;
        }
    }
    else if (s_startup_state == ESP_STARTUP_WAIT_CIPMUX_OK)
    {
        // 2. The millisecond the ESP says "OK" to CIPMUX...
        if (   (strstr(frame, "OK")        != 0)
            || (strstr(frame, "no change") != 0) )
        {
            // ...we fire off the SERVER command!
            ESP_SendCommand(ESP_CMD_CREATE_SERVER); 
            s_startup_state = ESP_STARTUP_WAIT_SERVER_OK;
        }
    }
    else if (s_startup_state == ESP_STARTUP_WAIT_SERVER_OK)
    {
        if (   (strstr(frame, "OK")        != 0)
            || (strstr(frame, "no change") != 0) )
        {
            s_startup_state = ESP_STARTUP_DONE; // We are fully online!
        }
    }
}
ESPStartupState ESP_GetStartupState(void)
{
    return s_startup_state;
}

void ESP_SendCommand(ESPCommandID id)
{
    if (id >= ESP_CMD_COUNT) { return; }
    uart_send_buf(esp_commands[id].cmd);
}

uint8_t ESP_ParseIPDFrame(const char *frame, ESPCommandEvent *out)
{
    const char   *p;
    unsigned int  i;
    unsigned int  val;

    if (!frame || !out) { return 0U; }

    out->valid      = 0U;
    out->direction  = ESP_DIR_NONE;
    out->time_units = 0U;
    out->pin[0]     = '\0';

    p = frame;
    while ((*p != '\0') && (*p != (char)ESP_CMD_START_CHAR)) { p++; }
    if (*p != (char)ESP_CMD_START_CHAR) { return 0U; }
    p++;

    for (i = 0U; i < 4U; i++)
    {
        if (p[i] == '\0') { return 0U; }
        out->pin[i] = p[i];
    }
    out->pin[4U] = '\0';
    p += 4U;

    if (strncmp(out->pin, ESP_COMMAND_PIN, 4U) != 0) { return 0U; }

    switch (*p)
    {
        case 'F':  out->direction = ESP_DIR_FORWARD;  break;
        case 'B':  out->direction = ESP_DIR_REVERSE;  break;
        case 'R':  out->direction = ESP_DIR_RIGHT;    break;
        case 'L':  out->direction = ESP_DIR_LEFT;     break;
        case 'T':  out->direction = ESP_DIR_TURN;     break;
        case 'A':  out->direction = ESP_DIR_ALIGN;    break;
        case 'P':  out->direction = ESP_DIR_PID;   break;
        default:   return 0U;
    }
    p++;

    if ((*p < '0') || (*p > '9')) { return 0U; }

    val = 0U;
    while ((*p >= '0') && (*p <= '9'))
    {
        val = (val * 10U) + (unsigned int)(*p - '0');
        p++;
    }
    out->time_units = val;
    out->valid      = 1U;

    return 1U;
}

void execute_iot_command(const ESPCommandEvent *evt) {
    if (!evt || !evt->valid) { return; }

    // Always ensure motors are off before changing directions
    turn_off_all(); 

    switch (evt->direction) {
        case ESP_DIR_FORWARD:
            LEFT_FORWARD_SPEED  = SLOW_L;
            RIGHT_FORWARD_SPEED = SLOW_R;
            ms_delay(evt->time_units * 1000);
            break;

        case ESP_DIR_REVERSE:
            LEFT_REVERSE_SPEED  = SLOW_L;
            RIGHT_REVERSE_SPEED = SLOW_R;
            ms_delay(evt->time_units * 1000);
            break;

        case ESP_DIR_RIGHT:
           RIGHT_FORWARD_SPEED = t_R;
           LEFT_REVERSE_SPEED  = t_L;
            ms_delay(evt->time_units * 8); 
            break;

        case ESP_DIR_LEFT:
            LEFT_FORWARD_SPEED  = t_L;
            RIGHT_REVERSE_SPEED = t_R;
            ms_delay(evt->time_units * 8);
            break;

        case ESP_DIR_TURN:
            LEFT_FORWARD_SPEED  = TURN_L;
            RIGHT_FORWARD_SPEED = TURN_R;
            ms_delay(evt->time_units * 1000);
            break;

         case ESP_DIR_ALIGN:
            LEFT_FORWARD_SPEED  = ALIGN_L;
            RIGHT_FORWARD_SPEED = ALIGN_R;
            ms_delay(evt->time_units * 1000);
            break;
        
        case ESP_DIR_PID:
             system_running = 1; 
             robot_state = STATE_COUNTDOWN;
             timer_seconds = 0;
             menu_mode = 1;
            break;

        default:
            break;
    }

    // Stop the car after the movement is complete
    turn_off_all(); 
}


uint8_t ESP_HasQueuedCommand(void)
{
    return (s_q_head != s_q_tail) ? 1U : 0U;
}

uint8_t ESP_DequeueCommand(ESPCommandEvent *out)
{
    if (s_q_head == s_q_tail) { return 0U; }
    *out     = s_cmd_queue[s_q_tail];
    s_q_tail = (s_q_tail + 1U) & ESP_CMD_Q_MASK;
    return 1U;
}

uint8_t ESP_EnqueueFromFrame(const char *frame)
{
    const char     *p     = frame;
    uint8_t         count = 0U;
    uint8_t         next_head;
    ESPCommandEvent evt;

    if (!frame) { return 0U; }

    while (*p != '\0')
    {
        while ((*p != '\0') && (*p != (char)ESP_CMD_START_CHAR)) { p++; }
        if (*p == '\0') { break; }

        if (ESP_ParseIPDFrame(p, &evt))
        {
            next_head = (s_q_head + 1U) & ESP_CMD_Q_MASK;
            if (next_head != s_q_tail)
            {
                s_cmd_queue[s_q_head] = evt;
                s_q_head              = next_head;
                count++;
            }
        }

        p++;
    }

    return count;
}

void ESP_SetPendingEvent(const ESPCommandEvent *evt)
{
    if (!evt) { return; }
    s_pending_event = *evt;
    s_has_pending   = 1U;
}

unsigned char ESP_HasPendingEvent(void)
{
    return s_has_pending;
}

const ESPCommandEvent *ESP_GetPendingEvent(void)
{
    return s_has_pending ? &s_pending_event : 0;
}

void ESP_ConsumePendingEvent(void)
{
    s_has_pending = 0U;
}