/* ========================================
 *
 * Copyright YOUR COMPANY, THE YEAR
 * All Rights Reserved
 * UNPUBLISHED, LICENSED SOFTWARE.
 *
 * CONFIDENTIAL AND PROPRIETARY INFORMATION
 * WHICH IS THE PROPERTY OF your company.
 *
 * ========================================
*/

#include <andras.h>

// Function for assignment 1/1
void assignment_1(void) {
    
    // Start up robot
    printf("\nStarting up.\n");
    motor_start();
    motor_forward(0, 0);
    
    // Wait for start button
    printf("\nPress start.\n");
    BatteryLed_Write(1);
    while(SW1_Read() == 1);
    BatteryLed_Write(0);
    vTaskDelay(1000);
    
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
    printf("\nShutting down.\n");
    motor_stop();

}

// Function for assignment 1/2
void assignment_2(void) {

    // Start up robot
    printf("\nStarting up.\n");
    Ultra_Start();
    motor_start();
    motor_forward(0, 0);
    
    // Wait for start button
    printf("\nPress start.\n");
    BatteryLed_Write(1);
    while(SW1_Read() == 1);
    BatteryLed_Write(0);
    vTaskDelay(1000);
    
    // Navigate track
    printf("\nPress the button to stop.\n");
    while (SW1_Read() == 1) {
        int d = Ultra_GetDistance();
        if (d < 10) {
            motor_backward(255, 10);
            motor_turn(0, 255, 240);
        }
        motor_forward(255, 10);
    }
    
    // Shut down robot
    printf("\nShutting down.\n");
    motor_stop();

}

// Function for assignment 1/3
void assignment_3(void) {

    // Start up robot
    printf("\nStarting up.\n");
    Ultra_Start();
    motor_start();
    motor_forward(0, 0);
    
    // Wait for start button
    printf("\nPress start.\n");
    BatteryLed_Write(1);
    while(SW1_Read() == 1);
    BatteryLed_Write(0);
    vTaskDelay(1000);
    
    // Navigate track
    printf("\nPress the button to stop.\n");
    while (SW1_Read() == 1) {
        int d = Ultra_GetDistance();
        if (d < 10) {
            motor_backward(255, 100);
            do {
                int angle = rand() % 180 + 90;
                tank_turn(angle);
            } while (Ultra_GetDistance() < 10);
        }
        motor_forward(255, 100);
    }
    
    // Shut down robot
    printf("\nShutting down.\n");
    motor_stop();

}

// Function for assignment 2/1
void assignment_4(void) {

    // Define variables
    struct sensors_ sensors;
    int count = 0;
    
    // Start up robot
    printf("\nStarting up.\n");
    motor_start();
    motor_forward(0, 0);
    IR_Start();
    IR_flush();
    reflectance_start();
    reflectance_set_threshold(15000, 15000, 18000, 18000, 15000, 15000);
    
    // Wait for start button
    printf("\nPress start.\n");
    BatteryLed_Write(1);
    while(SW1_Read() == 1);
    BatteryLed_Write(0);
    vTaskDelay(1000);
    
    // Navigate track
    while (count < 5) {
        follow_line(&sensors, 255, 10);
        count++;
        if (count == 1) {
            printf("\nWaiting for IR.\n");
            motor_forward(0, 0);
            IR_wait();
            printf("\nIR signal received.\n");
        }
    }
    
    // Shut down robot
    printf("\nShutting down.\n");
    motor_stop();

}

// Function for assignment 2/2
void assignment_5(void) {

    // Define variables
    struct sensors_ sensors;
    int count = 0;
    
    // Start up robot
    printf("\nStarting up.\n");
    motor_start();
    motor_forward(0, 0);
    IR_Start();
    IR_flush();
    reflectance_start();
    reflectance_set_threshold(15000, 15000, 18000, 18000, 15000, 15000);
    
    // Wait for start button
    printf("\nPress start.\n");
    BatteryLed_Write(1);
    while(SW1_Read() == 1);
    BatteryLed_Write(0);
    vTaskDelay(1000);
    
    // Navigate track
    while (count < 2) {
        follow_line(&sensors, 255, 10);
        count++;
        if (count == 1) {
            printf("\nWaiting for IR.\n");
            motor_forward(0, 0);
            IR_wait();
            printf("\nIR signal received.\n");
        }
    }
    
    // Shut down robot
    printf("\nShutting down.\n");
    motor_stop();

}

// Function for assignment 2/3
void assignment_6(void) {

    // Define variables
    struct sensors_ sensors;
    int count = 0;
    
    // Start up robot
    printf("\nStarting up.\n");
    motor_start();
    motor_forward(0, 0);
    IR_Start();
    IR_flush();
    reflectance_start();
    reflectance_set_threshold(15000, 15000, 18000, 18000, 15000, 15000);
    
    // Wait for start button
    printf("\nPress start.\n");
    BatteryLed_Write(1);
    while(SW1_Read() == 1);
    BatteryLed_Write(0);
    vTaskDelay(1000);
    
    // Navigate track
    while (count < 5) {
        follow_line(&sensors, 255, 10);
        count++;
        if (count == 1) {
            printf("\nWaiting for IR.\n");
            motor_forward(0, 0);
            IR_wait();
            printf("\nIR signal received.\n");
        } else if (count == 2) {
            while (!sensor_AND(&sensors, 0, 0, 1, 1, 0, 0)) {
                motor_turn(0, 200, 10);
                reflectance_digital(&sensors);
            }
        } else if (count == 3 || count == 4) {
            while (!sensor_AND(&sensors, 0, 0, 1, 1, 0, 0)) {
                motor_turn(200, 0, 10);
                reflectance_digital(&sensors);
            }
        }
    }
    
    // Shut down robot
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
}

// Compare sensor values with AND
int sensor_AND(struct sensors_ *sensors, int L3, int L2, int L1, int R1, int R2, int R3) {
    if (sensors->L3 == L3 && sensors->L2 == L2 && sensors->L1 == L1 && sensors->R1 == R1 && sensors->R2 == R2 && sensors->R3 == R3) {
        return 1;
    } else {
        return 0;
    }
}

// Compare sensor values with OR
int sensor_OR(struct sensors_ *sensors, int L3, int L2, int L1, int R1, int R2, int R3) {
    if (sensors->L3 == L3 || sensors->L2 == L2 || sensors->L1 == L1 || sensors->R1 == R1 || sensors->R2 == R2 || sensors->R3 == R3) {
        return 1;
    } else {
        return 0;
    }
}

// Tank turn by x degrees
void tank_turn(int16 angle) {

    uint8 left_dir = (angle < 0) ? 0 : 1;
    uint8 right_dir = (angle >= 0) ? 0 : 1;
    
    uint8 angle_corrected = ((angle < 0) ? angle * -1 : angle) % 360;
    
    uint32 delay = (angle_corrected * 1048) / 360; // 1048 delay is ~a whole turn at 200 speed
    
    SetMotors(left_dir, right_dir, 100, 100, delay);
    
}

/* [] END OF FILE */
