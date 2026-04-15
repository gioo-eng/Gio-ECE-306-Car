#include "msp430.h"
#include "motors.h"
#include "functions.h"
#include "macros.h"
#include "timers.h"

extern volatile int left_ir;
extern volatile int right_ir;


#define TARGET 195.0

//original 700 and 250
// --- PID Constants ---
float Kp = 1400.0, Kd = 2000.0;

// --- PID Variables ---
float error = 0, previousError = 0, proportional = 0, derivative = 0;

#define BASE_SPEED 22000
#define MAX_SPEED 34000
#define MIN_SPEED 12000



void Init_PID(void) {
    error = 0; previousError = 0; proportional = 0; derivative = 0;
}


void Run_PID(void) {
    float physical_left = (float)left_ir;
    float physical_right = (float)right_ir;

    if (physical_left < 100.0 && physical_right < 100.0) {
        // --- THE FIX ---
        // Force a massive, constant error depending on the last known direction.
        // This instantly pins one wheel to MAX_SPEED and the other to 0.
        if (previousError > 0) {
            error = 500.0;  // Lost it to the right
        } else {
            error = -500.0; // Lost it to the left
        }
    } else {
        error = physical_left - physical_right;
    }

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







