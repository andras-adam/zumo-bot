/* =========================================
 *
 * Assignment solutions by András Ádám, 2020
 *
 * =========================================
*/

#include <project.h>
#include <stdio.h>
#include <stdlib.h>
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

void assignment_1(void);
void assignment_2(void);
void assignment_3(void);
void assignment_4(void);
void assignment_5(void);
void assignment_6(void);
void assignment_7(void);
void assignment_8(void);
void assignment_9(void);

void startup(bool launch_button, bool motor, bool IR, bool reflectance, bool ultrasound);
void shutdown(void);
void follow_line(struct sensors_ *sensors, uint8 speed, uint32 delay);
void tank_turn(int16 angle);
void wait_for_IR(void);
int sensor_AND(struct sensors_ *sensors, int L3, int L2, int L1, int R1, int R2, int R3);
int sensor_OR(struct sensors_ *sensors, int L3, int L2, int L1, int R1, int R2, int R3);

/* [] END OF FILE */
