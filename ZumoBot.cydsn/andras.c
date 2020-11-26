/* =========================================
 *
 * Assignment solutions by András Ádám, 2020
 *
 * =========================================
*/

#include <andras.h>

// Function for assignment 1-1
void assignment_1_1(void) {

    // Start up robot (launch_button, motors)
    startup(true, true, false, false, false);
    
    // Navigate track
    motor_forward(255, 1310);
    motor_turn(200, 0, 262);
    motor_forward(255, 1020);
    motor_turn(200, 0, 262);
    motor_forward(255, 1040);
    motor_turn(255, 0, 250);
    motor_forward(255, 250);
    motor_turn(255, 200, 600);
    motor_turn(255, 0, 31);
    motor_forward(255, 390);
    
    // Shut down robot
    shutdown();

}

// Function for assignment 1-2
void assignment_1_2(void) {

    // Start up robot (launch_button, motors, ultrasound)
    startup(true, true, false, false, true);
    
    // Navigate track
    printf("\nPress the button to stop.\n");
    while (SW1_Read() == 1) {
        int d = Ultra_GetDistance();
        if (d < 10) {
            motor_backward(255, 100);
            motor_turn(0, 200, 350);
        }
        motor_forward(255, 10);
    }
    
    // Shut down robot
    shutdown();

}

// Function for assignment 1-3
void assignment_1_3(void) {

    // Start up robot (launch_button, motors, ultrasound)
    startup(true, true, false, false, true);
    
    // Navigate track
    printf("\nPress the button to stop.\n");
    while (SW1_Read() == 1) {
        if (Ultra_GetDistance() < 10) {
            motor_backward(255, 100);
            do {
                int angle = rand() % 180 + 90;
                tank_turn(angle);
            } while (Ultra_GetDistance() < 10);
        }
        motor_forward(255, 100);
    }
    
    // Shut down robot
    shutdown();

}

// Function for assignment 2-1
void assignment_2_1(void) {

    // Define variables
    struct sensors_ sensors;
    int count = 0;
    
    // Start up robot (launch_button, motors, IR, reflectance)
    startup(true, true, true, true, false);
    
    // Navigate track
    while (count < 5) {
        follow_line(&sensors, 255, 10);
        count++;
        if (count == 1) wait_for_IR();
    }
    
    // Shut down robot
    shutdown();

}

// Function for assignment 2-2
void assignment_2_2(void) {

    // Define variables
    struct sensors_ sensors;
    int count = 0;
    
    // Start up robot (launch_button, motors, IR, reflectance)
    startup(true, true, true, true, false);
    
    // Navigate track
    while (count < 2) {
        follow_line(&sensors, 255, 10);
        count++;
        if (count == 1) wait_for_IR();
    }
    
    // Shut down robot
    shutdown();

}

// Function for assignment 2-3
void assignment_2_3(void) {

    // Define variables
    struct sensors_ sensors;
    int count = 0;
    
    // Start up robot (launch_button, motors, IR, reflectance)
    startup(true, true, true, true, false);
    
    // Navigate track
    while (count < 5) {
        follow_line(&sensors, 150, 10);
        count++;
        if (count == 1) {
            wait_for_IR();
        } else if (count == 2) {
            while (!sensor_AND(&sensors, 0, 0, 1, 1, 0, 0)) {
                motor_turn(0, 150, 10);
                reflectance_digital(&sensors);
            }
        } else if (count == 3 || count == 4) {
            while (!sensor_AND(&sensors, 0, 0, 1, 1, 0, 0)) {
                motor_turn(150, 0, 10);
                reflectance_digital(&sensors);
            }
        }
    }
    
    // Shut down robot
    shutdown();

}

// Function for assignment 3-1
void assignment_3_1(void) {

    // Define variables
    TickType_t current = 0;
    TickType_t previous = 0;
    
    // Start up robot (launch_button)
    startup(true, false, false, false, false);
    
    // Perform task
    previous = xTaskGetTickCount();
    while (true) {
        while (SW1_Read() == 1) vTaskDelay(1);
        current = xTaskGetTickCount();
        int diff = (int) (current) - (int) (previous);
        printf("\nMilliseconds since last button push: %d.\n", diff);
        print_mqtt("Zumo01/button", "%d", diff);
        while (SW1_Read() == 0) vTaskDelay(1);
        previous = current;
    }
    
    // Shut down robot
    shutdown();

}

// Function for assignment 3-2
void assignment_3_2(void) {

    // Start up robot (launch_button, motors, ultrasound)
    startup(true, true, false, false, true);
    
    // Navigate track
    printf("\nPress the button to stop.\n");
    while (SW1_Read() == 1) {
        if (Ultra_GetDistance() < 10) {
            do {
                motor_backward(255, 100);
                int dir = rand() % 2;
                tank_turn(dir ? 90 : -90);
                printf("\nTurning 90 degrees %s.\n", dir ? "left" : "right");
                print_mqtt("Zumo01/turn", "%s", dir ? "left" : "right");
            } while (Ultra_GetDistance() < 10);
        }
        motor_forward(255, 100);
    }
    
    // Shut down robot
    shutdown();

}

// Function for assignment 3-3
void assignment_3_3(void) {

    // Define variables
    struct sensors_ sensors;
    TickType_t previous = 0;
    TickType_t current = 0;
    
    // Start up robot (launch_button, motors, IR, reflectance)
    startup(true, true, true, true, false);
    
    // Navigate track
    while (SW1_Read() == 1) {
        follow_line(&sensors, 255, 10);
        if (previous) {
            current = xTaskGetTickCount();
            int diff = (int) (current) - (int) (previous);
            printf("\nMilliseconds since last line: %d.\n", diff);
            print_mqtt("Zumo01/lap", "%d", diff);
        }
        wait_for_IR();
        previous = xTaskGetTickCount();
    }
    
    // Shut down robot
    shutdown();

}

void assignment_line_following(void) {

    // Define variables
    struct sensors_ sensors;
    int count = 0;
    TickType_t start = 0;
    TickType_t stop = 0;
    
    // Start up robot (launch_button, motors, IR, reflectance)
    startup(true, true, true, true, false);
    
    // Navigate track
    while (count < 3) {
        follow_line(&sensors, 255, 10);
        count++;
        if (count == 1) {
            send_mqtt("Zumo17/ready", "line");
            wait_for_IR();
            start = xTaskGetTickCount();
            print_mqtt("Zumo17/start", "%d", start);
        } else if (count == 3) {
            stop = xTaskGetTickCount();
            print_mqtt("Zumo17/stop", "%d", stop);
            int time = (int) stop - (int) start;
            print_mqtt("Zumo17/time", "%d", time);
        }
    }
    
    // Shut down robot
    shutdown();

}

// Start up the robot
void startup(bool launch_button, bool motor, bool IR, bool reflectance, bool ultrasound) {
    
    // Print startup message
    printf("\nStarting up.\n");

    // Start up motors
    if (motor) {
        motor_start();
        motor_forward(0, 0);
    }
    
    // Start up IR sensors
    if (IR) {
        IR_Start();
        IR_flush();
    }
    
    // Start reflectance
    if (reflectance) {
        reflectance_start();
        reflectance_set_threshold(15000, 15000, 18000, 18000, 15000, 15000);
    }
    
    // Start ultrasound
    if (ultrasound) {
        Ultra_Start();
    }
    
    // Wait for button press to start
    if (launch_button) {
        printf("\nPress start.\n");
        BatteryLed_Write(1);
        while(SW1_Read() == 1);
        BatteryLed_Write(0);
        vTaskDelay(1000);
    }

}

// Shut down the robot
void shutdown(void) {
    printf("\nShutting down.\n");
    motor_stop();
}

// Follow a line until the next intersection
void follow_line(struct sensors_ *sensors, uint8 speed, uint32 delay) {
    reflectance_digital(sensors);
    
    // Leave intersection forward
    while (sensor_AND(sensors, 1, 1, 1, 1, 1, 1)) {
        motor_forward(speed, delay);
        reflectance_digital(sensors);
    }
    
    // Follow line until next intersection
    while (!sensor_AND(sensors, 1, 1, 1, 1, 1, 1)) {
        while (sensors->R2 == 1 && sensors->L2 == 0) {
            tank_turn(-1);
            reflectance_digital(sensors);
        }
        while (sensors->L2 == 1 && sensors->R2 == 0) {
            tank_turn(1);
            reflectance_digital(sensors);
        }
        motor_forward(speed, delay);
        reflectance_digital(sensors);
    }
    
    // Stop motors
    motor_forward(0, 0);
    
}

// Tank turn by given degrees
void tank_turn(int16 angle) {

    uint8 left_dir = (angle < 0) ? 0 : 1;
    uint8 right_dir = (angle >= 0) ? 0 : 1;
    
    uint8 angle_corrected = ((angle < 0) ? angle * -1 : angle) % 360;
    
    uint32 delay = (angle_corrected * 1048) / 360; // 1048 delay is ~a whole turn at 200 speed
    
    SetMotors(left_dir, right_dir, 100, 100, delay);
    
}

// Wait for infrared (IR) signal
void wait_for_IR(void) {
    printf("\nWaiting for IR to continue.\n");
    IR_flush();
    IR_wait();
    printf("\nIR signal received.\n");
}

// Compare reflectance sensor values with AND
int sensor_AND(struct sensors_ *sensors, int L3, int L2, int L1, int R1, int R2, int R3) {
    if (sensors->L3 == L3 && sensors->L2 == L2 && sensors->L1 == L1 && sensors->R1 == R1 && sensors->R2 == R2 && sensors->R3 == R3) {
        return 1;
    } else {
        return 0;
    }
}

// Compare reflectance sensor values with OR
int sensor_OR(struct sensors_ *sensors, int L3, int L2, int L1, int R1, int R2, int R3) {
    if (sensors->L3 == L3 || sensors->L2 == L2 || sensors->L1 == L1 || sensors->R1 == R1 || sensors->R2 == R2 || sensors->R3 == R3) {
        return 1;
    } else {
        return 0;
    }
}

/* [] END OF FILE */
