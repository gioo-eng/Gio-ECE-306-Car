#include "msp430.h"
#include "motors.h"
#include "functions.h"
#include "macros.h"
#include "timers.h"

extern volatile int left_ir;
extern volatile int right_ir;
volatile unsigned int drive_timer = 0; 
static int in_recovery = 0; 
static int spin_direction = 0; // 0 = spin left, 1 = spin right
    
#define TARGET 195.0

//original 700 and 250
// --- PID Constants ---
float Kp = 4800.0, Kd = 1200.0;

// --- PID Variables ---
float error = 0, previousError = 0, proportional = 0, derivative = 0;

#define BASE_SPEED 20000
#define MAX_SPEED 40000
#define MIN_SPEED 12000



void Init_PID(void) {
    error = 0; previousError = 0; proportional = 0; derivative = 0;
}


void Run_PID(void) {


    
    float physical_left = (float)left_ir;
    float physical_right = (float)right_ir;




    
    // =========================================================
    // 1. ARE WE CURRENTLY IN RECOVERY MODE?
    // =========================================================
    if (in_recovery == 1) {
        // The condition to EXIT recovery: BOTH sensors must be > 185
        if (physical_left > 185.0 && physical_right > 185.0) {
            in_recovery = 0; // Line found! Exit recovery mode.
        } else {
            // Still searching for the line. Keep one motor OFF and one at MAX.
            if (spin_direction == 0) {
                LEFT_FORWARD_SPEED = 0;
                RIGHT_FORWARD_SPEED = MAX_SPEED;
            } else {
                LEFT_FORWARD_SPEED = MAX_SPEED;
                RIGHT_FORWARD_SPEED = 0;
            }
            return; // Exit immediately so normal PID math doesn't run
        }
    }

    // =========================================================
    // 2. DO WE NEED TO ENTER RECOVERY MODE?
    // =========================================================
    // The condition to ENTER recovery: EITHER sensor drops below 130
    if (physical_left < 130.0 || physical_right < 130.0) {
        in_recovery = 1; // Lock the robot into recovery state
        
        // Decide which way to spin based on the last known position (previousError)
        if (previousError >= 0) {
            spin_direction = 0; // Spin left
            LEFT_FORWARD_SPEED = 0;
            RIGHT_FORWARD_SPEED = MAX_SPEED;
        } else {
            spin_direction = 1; // Spin right
            LEFT_FORWARD_SPEED = MAX_SPEED;
            RIGHT_FORWARD_SPEED = 0;
        }
        
        return; // Exit immediately so normal PID math doesn't run
    }
        error = physical_left - physical_right;
    

    proportional = error;
    derivative = error - previousError;

    // 3. Calculate ONE steering correction
    float turn_correction = (Kp * proportional) + (Kd * derivative);

    // 4. Apply Differential Steering
    long next_left_speed = (long)BASE_SPEED - (long)turn_correction;
    long next_right_speed = (long)BASE_SPEED + (long)turn_correction;

    // Safety caps
    if (next_left_speed > MAX_SPEED) next_left_speed = MAX_SPEED;
    if (next_left_speed < MIN_SPEED) next_left_speed = MIN_SPEED;
    if (next_right_speed > MAX_SPEED) next_right_speed = MAX_SPEED;
    if (next_right_speed < MIN_SPEED) next_right_speed = MIN_SPEED;

    LEFT_FORWARD_SPEED = (unsigned int)next_left_speed;
    RIGHT_FORWARD_SPEED = (unsigned int)next_right_speed;

    // 5. Update History
    previousError = error;
}


/* 
void Run_PID(void) {
    float physical_left = (float)left_ir;
    float physical_right = (float)right_ir;


    if (physical_left < 100.0 && physical_right < 100.0) {

error = previousError * 4.0;
    } else {

        error = physical_left - physical_right;
    }

    proportional = error;
    derivative = error - previousError;

    // 3. Calculate ONE steering correction
    float turn_correction = (Kp * proportional) + (Kd * derivative);

    // 4. Apply Differential Steering
    // We subtract from the left and add to the right to create massive turning torque.
    // If it spins the wrong way, just swap the + and - signs below!
    long next_left_speed = (long)BASE_SPEED - (long)turn_correction;
    long next_right_speed = (long)BASE_SPEED + (long)turn_correction;

    // Safety caps
    if (next_left_speed > MAX_SPEED) next_left_speed = MAX_SPEED;
    if (next_left_speed < MIN_SPEED) next_left_speed = MIN_SPEED;
    if (next_right_speed > MAX_SPEED) next_right_speed = MAX_SPEED;
    if (next_right_speed < MIN_SPEED) next_right_speed = MIN_SPEED;

    LEFT_FORWARD_SPEED = (unsigned int)next_left_speed;
    RIGHT_FORWARD_SPEED = (unsigned int)next_right_speed;

    // 5. Update History
    previousError = error;
}
*/







