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
#include <vasilydavydov.h>

// Function for assignment 1-1
/*void assignment_week3_1(void) {


motor_start();
motor_forward(0, 0);
    
    // Track navigation
    motor_forward(255, 4000);
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
    shut();

}
*/
/*void start(bool motor, bool IR, bool reflectance, bool ultrasonic, bool led){
    BatteryLed_Write(0);
    while (true) {
      if (SW1_Read() == 0) {
         led = !led;
         BatteryLed_Write(led);
         if (led) {
            printf("Led is ON\n");
         } else {
            printf("Led is OFF\n");
         }
         Beep(1000, 150);
         while(SW1_Read() == 0) vTaskDelay(10); // Wait while button is being pressed
      }        
   }

motor_start();              
motor_forward(0, 0);
 }
*/
void shut(void){
    motor_forward(0,0);
    motor_stop();
}
    


// function for recongnising obstacles
void obstacle (){
    motor_forward(0,10);
    motor_backward(100, 150);
    motor_turn(0, 150, 462);
}



void assignment_week3_2 (void){
    
    //starting motors and Ultrasonic sensor
    motor_start();
    Ultra_Start();
    //starting from a zero speed
    motor_forward(0, 0);
    //entering an infinite loop
    while (true) {
        motor_forward(120, 50); //giving speed to motors
        // vTaskDelay(100);
        if (Ultra_GetDistance() < 11){ //assigning the value of the function (10cm) to an if-statement
            obstacle(); 
        }
    }    
}


//These functions allow to make a tankturn either left or right:
//200 speed 262 delay is 90 degrees
void tank_turn_right(uint8 speed,uint32 delay){
//   SetMotors(0,0, l_speed, r_speed, delay);
    SetMotors(0,1, speed, speed, delay);
}
void tank_turn_left(uint8 speed,uint32 delay){
//   SetMotors(0,0, l_speed, r_speed, delay);
    SetMotors(1,0, speed, speed, delay);

}


void assignment_week3_3 (){
 //starting motors and Ultrasonic sensor
    motor_start();
    Ultra_Start();
    //starting from a zero speed
    motor_forward(0, 0);
    //entering an infinite loop
    while (SW1_Read()==1) {
       
        // vTaskDelay(100);
        if (Ultra_GetDistance() < 11){ //assigning the value of the function (10cm) to an if-statement
            motor_backward(100,150);
            int angle = rand()%524+262;
            if (rand()%2 == 1){
                tank_turn_right(200, angle);
            }else {
                tank_turn_left(200, angle);
            }    
        }
        motor_forward(200, 50); //giving speed to motors
    }    

motor_forward(0,0);//stoppimg the motor
motor_stop();
}


/* [] END OF FILE */
