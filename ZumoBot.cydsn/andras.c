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

// Function for assignment 2/4
void assignment_4(void) {

    struct sensors_ sensors;
    int count = 0;
    int touching = 0;
    
    // Start up robot
    printf("\nStarting up.\n");
    motor_start();
    motor_forward(0, 0);
    IR_Start();
    IR_flush();
    reflectance_start();
    reflectance_set_threshold(9000, 9000, 11000, 11000, 9000, 9000);
    
    // Wait for start button
    printf("\nPress start.\n");
    BatteryLed_Write(1);
    while(SW1_Read() == 1);
    BatteryLed_Write(0);
    vTaskDelay(1000);
    
    // Navigate track
    while (count < 5) {
        reflectance_digital(&sensors);   
        if (touching != 1 && sensors.L3 == 1 && sensors.L2 == 1 && sensors.L1 == 1 && sensors.R1 == 1 && sensors.R2 == 1 && sensors.R1 == 1) {
            touching = 1;
            count++;
            if (count == 1) {
                printf("\nWaiting for IR.\n");
                motor_forward(0, 0);
                IR_wait();
                printf("\nIR signal received.\n");
            }
        } else if (touching == 1 && (sensors.L3 == 0 || sensors.L2 == 0 || sensors.L1 == 0 || sensors.R1 == 0 || sensors.R2 == 0 || sensors.R1 == 0)) {
            touching = 0;
        }
        motor_forward(100, 10); 
    }
    
    // Shut down robot
    printf("\nShutting down.\n");
    motor_stop();

}

// Tank turn by x degrees
void tank_turn(int16 angle) {

    uint8 left_dir = (angle < 0) ? 0 : 1;
    uint8 right_dir = (angle >= 0) ? 0 : 1;
    
    uint8 angle_corrected = ((angle < 0) ? angle * -1 : angle) % 360;
    
    uint32 delay = (angle_corrected * 1048) / 360; // 1048 delay is ~a whole turn at 100 speed
    
    SetMotors(left_dir, right_dir, 100, 100, delay);
    
}

/* [] END OF FILE */
