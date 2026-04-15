/*
 * motors.h
 *
 *  Created on: Mar 7, 2026
 *      Author: yanny
 */

#ifndef MOTORS_H_
#define MOTORS_H_

// Defines for PWM
#define WHEEL_OFF (0)
#define SLOW_R (23000)
#define SLOW_L (23000)
#define t_R (29000)
#define t_L (26000)
#define TURN_R (35000)
#define TURN_L (28000)
#define ALIGN_R (40000)
#define ALIGN_L (18000)
#define FAST (50000)
#define PERCENT_100 (50000)
#define PERCENT_80 (45000)
#define PWM_PERIOD           (TB3CCR0)
#define LCD_BACKLITE_DIMING  (TB3CCR1)
#define RIGHT_FORWARD_SPEED  (TB3CCR2)
#define LEFT_FORWARD_SPEED   (TB3CCR3)
#define RIGHT_REVERSE_SPEED  (TB3CCR4)
#define LEFT_REVERSE_SPEED   (TB3CCR5)
#define WHEEL_PERIOD          (50005)

// project 5 shapes
void turn_off_all(void);
void both_forward(void);
void both_reverse(void);
void spin_cw(void);
void spin_ccw(void);
void check_motor_safety(void);



#endif /* MOTORS_H_ */
