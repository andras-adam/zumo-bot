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
#include <project.h>
#include <stdio.h>
#include "FreeRTOS.h"
#include "task.h"
#include "Motor.h"
#include "Ultra.h"
#include "Nunchuk.h"
#include "Reflectance.h"
#include "Gyro.h"
#include "Accel_magnet.h"
#include "LSM303D.h"
#include "IR.h"
#include "Beep.h"
#include "mqtt_sender.h"
#include <time.h>
#include <sys/time.h>
#include "serial1.h"
#include <unistd.h>
#include <stdlib.h>


//function declarations:
void shut(void);
void launch_system(bool motor, bool IR, bool reflectance, bool ultrasonic);
int getRefValues (struct sensors_ *sensors, int SL3, int SL2, int SL1, int SR1, int SR2, int SR3);
void line_follower(struct sensors_ *sensors);
void obstacle();
void tank_turn_right(uint8 speed,uint32 delay);
void tank_turn_left(uint8 speed,uint32 delay);

void assignment_week3_1(void);
void assignment_week3_2 (void);
void assignment_week3_3 (void);
void assignment_week4_2 (void);

/* [] END OF FILE */
