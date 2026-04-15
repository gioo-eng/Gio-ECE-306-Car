/*
 * macros.h
 *
 *  Created on: Mar 21, 2026
 *      Author: everettbrostedt
 */

#ifndef MACROS_H_
#define MACROS_H_

//General Use defines
#define ALWAYS                  (1)
#define FALSE                  (0x00) //
#define TRUE                   (0x01) //
#define MOTOR                  (0x00) //
#define SMCLK_OFF              (0x00) //
#define SMCLK_ON               (0x01) //
#define PORTS                  (0x00) // RED LED 0
#define PWM_MODE               (0x01) // GREEN LED 1
#define STEP                   (2000)
#define FORWARD                (0x00) // FORWARD
#define REVERSE                (0x01) // REVERSE
#define ON                     (0x01)
#define OFF                    (0x00)

//Channel definitions for ADC
#define CHANNEL_2 (0x02)
#define CHANNEL_3 (0x03)
#define CHANNEL_5 (0x05)

//DAC definitions for lowering voltage
#define DAC_Begin (2725) // 2v
#define DAC_Limit (1500) // 4.02v
#define DAC_Adjust (1505) // 4.00v

//PWM definitions for wheel speed
#define WHEEL_OFF               (0)         // 0% duty cycle
#define SLOW                    (7500)      // 15% duty cycle
#define MEDIUM                  (15000)     // 30% duty cycle
#define FAST                    (40000)     // 80% duty cycle
#define LUCID                   (50000)     // 100% duty cycle

//PWM definitions for the LCD lighting
#define PERCENT_100             (50000)
#define PERCENT_20              (10000)
#define WHEEL_PERIOD            (50005)

//Control registers defines
#define PWM_PERIOD              (TB3CCR0)
#define LCD_BACKLITE_DIMING     (TB3CCR1)
#define RIGHT_FORWARD_SPEED     (TB3CCR2)
#define LEFT_FORWARD_SPEED      (TB3CCR3)
#define RIGHT_REVERSE_SPEED     (TB3CCR4)
#define LEFT_REVERSE_SPEED      (TB3CCR5)

//Timerb0 defines
#define TB0CCR0_INTERVAL (1250)     // clock ticks every 8µs and 1250 of them are 10ms
                                    // for time_sequence update
#define TIMER_B0_CCR0_VECTOR TIMER0_B0_VECTOR
#define TIMER_B0_CCR1_2_OV_VECTOR TIMER0_B1_VECTOR

//Switch defines
#define TB0CCR1_INTERVAL (25000)    // debounce time for SW1
#define TB0CCR2_INTERVAL (25000)    // debounce time for SW2

#define OKAY 1
#define NOT_OKAY 0
#define PRESSED 1
#define RELEASED 0

//old switch defines before interrupt
#define DEBOUNCE_TIME 50
#define DEBOUNCE_RESTART 0

//Defines for detect state machine
#define FIND_LINE           (0x00)
#define TURN_TO_LINE        (0x01)
#define NAV_LINE            (0x02)
#define FIND_CENTER         (0x03)
#define WAIT_CASE           (0x04)
#define OVERSHOOT           (0x05)
#define DIVERT              (0x06)
#define TURN_CENTER         (0x07)

//---------------------------------------------------------------------------------------------
//------------------------------------Shape state definitions----------------------------------
//---------------------------------------------------------------------------------------------
//defines for the two CIRCLE
#define CIRCLE_WHEEL_COUNT_TIME (20)
#define CIRCLE_RIGHT_COUNT_TIME (1)
#define CIRCLE_LEFT_COUNT_TIME (20)
#define CIRCLE_TRAVEL_DISTANCE (60)

//defines for a circle in one direction
#define FIRST_WHEEL_COUNT_TIME (20)
#define FIRST_RIGHT_COUNT_TIME (1)
#define FIRST_LEFT_COUNT_TIME (20)
#define FIRST_TRAVEL_DISTANCE (25)

//defines for a circle in second direction
#define SECOND_WHEEL_COUNT_TIME (20)
#define SECOND_RIGHT_COUNT_TIME (20)
#define SECOND_LEFT_COUNT_TIME (1)
#define SECOND_TRAVEL_DISTANCE (30)
#define THREE ('3')

//defines for the straight leg of the triangle
#define STRAIGHT_WHEEL_COUNT_TIME (20)
#define STRAIGHT_RIGHT_COUNT_TIME (16)
#define STRAIGHT_LEFT_COUNT_TIME (8)
#define STRAIGHT_TRAVEL_DISTANCE (5)

//defines for the 60 degree turn in the triangle
#define TURN_WHEEL_COUNT_TIME (20)
#define TURN_RIGHT_COUNT_TIME (0)
#define TURN_LEFT_COUNT_TIME (20)
#define TURN_TRAVEL_DISTANCE (8)
#define FOUR ('4')

// STATES =======================
#define NONE ('N')
#define FIGURE_8 ('F')
#define CIRCLE ('C')
#define TRIANGLE ('T')

#define STOP_I ('5')
#define FORWARD_I ('6')
#define REVERSE_I ('7')
#define TURN_I ('8')

#define WAIT ('W')
#define START ('S')
#define RUN ('R')
#define END ('E')

#define WAITING2START (300)

//Serial Defines =================
#define ONE_FIFTEEN   (0x5551)
#define FOUR_HUNDRED  (0x4A00)
#define BEGINNING     (0)

#define STARTUP       (0x01)
#define IOT_TRANSMIT  (0x02)
#define USB_TRANSMIT  (0x03)
#define RECEIVE       (0x04)
#define NEXT_RECEIVE  (0x05)
#define NEXT_TRANSMIT (0x06)
#define IDLE          (0x07)
#define IOT_STARTUP   (0x08)
#define IOT_RUN       (0x09)
#define MUX_RUN       (0x0A)
#define SERVER_RUN    (0x0B)
#define COMMAND_START (0x0C)
#define IOT_READY     (0x0D)
#define IOT_DISPLAY   (0x0E)
#define SSID          (0x0F)
#define IP_ADDY       (0x10)
#define PARSE_TIME    (0x11)


#endif /* MACROS_H_ */
