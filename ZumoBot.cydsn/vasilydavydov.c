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
void launch_system(bool motor, bool IR, bool reflectance, bool ultrasonic)
{
            printf("\nPreparing for a stratup\n");
    
            //motor starts  
            if (motor)
            {
            motor_start();              
            motor_forward(0, 0);
 
            }
            //IR starts
            if (IR)
            {
            IR_Start();
            IR_flush();
            }
            //reflectance startup
            if (reflectance)
            {
                reflectance_start();
                reflectance_set_threshold(15000, 15000, 18000, 18000, 15000, 15000);
            }
            //ultrasonic starts
            if (ultrasonic)
            {
                Ultra_Start();
            }
           printf("\nStartup Done Successfully!\n");
                
}            

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


void assignment_week3_3 ()
{
launch_system(true, true, true, true);
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



void assignment_week4_1()
{ 
    //variables declaration and engine launch
    int lines = 0;
    struct sensors_ sensors;
    launch_system(true, true, true, true);
//Enter to loops of intersections
     while(lines <5)
    {
        line_follower(&sensors);
        lines++;
            printf("We on line %d\n", lines);
            //Witing for the IR-signal
        if(lines == 1)
        {
        
            IR_flush();
            IR_wait();
        }
            
    }  
}   

void assignment_week4_2()
{
     //variables declaration and engine launch
    int lines = 0;
    struct sensors_ sensors;
    launch_system(true, true, true, true);
    //Switching the LED on
        BatteryLed_Write(1);
        //Starting the function, when the button is pressed
        while(SW1_Read() == 1);
        BatteryLed_Write(0);
        vTaskDelay(1000);
        
            while(lines <2)
            {
                line_follower(&sensors);
                lines++;
                //Checking on which line we are
                printf("We on line %d\n", lines);
                     //Witing for the IR-signal
                 if(lines == 1)
                {
        
                    IR_flush();
                    IR_wait();
                }
            
            }
            shut();
         
}
        
void assignment_week4_3()
{
     //variables declaration and engine launch
    int lines = 0;
    struct sensors_ sensors;
    launch_system(true, true, true, true);
    //Switching the LED on
        BatteryLed_Write(1);
        //Starting the function, when the button is pressed
        while(SW1_Read() == 1);
        BatteryLed_Write(0);
        vTaskDelay(1000);
        //stop on the last intersection
        while(lines < 5)
        {
                line_follower(&sensors);
                lines++;
                //Wait for the signal on the first intersection
                if(lines == 1)
                {
                    IR_flush();
                    IR_wait();
                    //turn left on the second intersection
                } else if(lines == 2)
                  {
                    while(!getRefValues(&sensors, 0,0,1,1,0,0))
                    {
                     
                     motor_turn(10,200,10);
                     reflectance_digital(&sensors);
                    }
                    //turn right on the third intersection
                  }else if(lines == 3)
                   {
                    while(!getRefValues(&sensors, 0,0,1,1,0,0))
                    {  
                     motor_turn(200,10,10);
                     reflectance_digital(&sensors);
                    }//turn right on the fourth intersection
                   }else if(lines == 4)
                    {
                    while(!getRefValues(&sensors, 0,0,1,1,0,0))
                    {
                     motor_turn(200,10,10);
                     reflectance_digital(&sensors);
                    }
                    }
        }
            
            //shuttin the system
            shut();
           
}

void assignment_week5_1()
{
    TickType_t press1= xTaskGetTickCount();
    
        //Switching the LED on
        BatteryLed_Write(1);
        //Starting the function, when the button is pressed
        while(SW1_Read() == 1);
        BatteryLed_Write(0);
        vTaskDelay(1000);
        
        while(1)
        {
            //button is not pressed
            while(SW1_Read() == 1)vTaskDelay(10);
            BatteryLed_Write(0);
                TickType_t press2= xTaskGetTickCount();
                //getting the difference
                int difference = (int)(press2) - (int)(press1);
                //Assigning first press value to the next one, so the start point is always new
                press1 = press2;  
                printf("\nMilliseconds from the previous button press: %d\n", difference);
                print_mqtt("Zumo99/button",  "%d", difference);
                //button is pressed
                while(SW1_Read() == 0)vTaskDelay(10);  
                BatteryLed_Write(1);
                
        }
              
                
    
}



//Function that allows to follow the line
void line_follower(struct sensors_ *sensors)
{ 
     reflectance_digital(sensors);
   //Going through the intersection
    while(getRefValues(sensors, 1, 1,1,1,1,1))
    {
        motor_forward(100,10);
        reflectance_digital(sensors);
    }


    while(!getRefValues(sensors, 1, 1, 1, 1, 1, 1))
    {
        //Left Turn
        while(sensors->R2 == 0 && sensors->L2 == 1)
        {
            tank_turn_left(255,1);
            reflectance_digital(sensors);
        }
        //Right Turn
        while(sensors->R2 == 1 && sensors->L2 == 0)
        {
            tank_turn_right(255, 1);
            reflectance_digital(sensors);
        }
        
        //Left turn over 90 degrees
        while(sensors->R2 == 1 && sensors->R3 == 0 && sensors->L2 == 1 && sensors->L3 == 1)
        {
            tank_turn_left(255, 1);
            reflectance_digital(sensors);
        }
        //Right turn over 90 degrees
         while(sensors->R2 == 1 && sensors->R3 == 1 && sensors->L2 == 1 && sensors->L3 == 0)
        {
            tank_turn_right(255, 1);
            reflectance_digital(sensors);
        }
        motor_forward(200, 10);
        reflectance_digital(sensors);
    }
//Stopping the motors
    motor_forward(0,0);
}









int getRefValues (struct sensors_ *sensors, int L3, int L2, int L1, int R1, int R2, int R3)
{
 if (sensors->L1 == L1 && sensors->L2 == L2 && sensors->L3 == L3 && sensors->R1 == R1 && sensors->R2 == R2 &&sensors->R3 == R3 )
{
   
    return 1;
}else
{
    
    return 0;
}
}

/* [] END OF FILE */
