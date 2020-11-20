# Documentation and example code

Code examples demonstrating the usage of the sensors available in the Metropolia PSoC Adapter.

#### Table of contents
- [Hello world](#hello-world)
- [Name and age](#name-and-age)
- [Tick timer](#tick-timer)
- [Button](#button)
- [Button and tick timer](#button-and-tick-timer)
- [Ultrasonic sensor](#ultrasonic-sensor)
- [IR receiver](#ir-receiver)
- [IR receiver with raw data](#ir-receiver-with-raw-data)
- [Reflectance](#reflectance)
- [Motor](#motor)
- [Accelerometer](#accelerometer)
- [MQTT](#mqtt)
- [Accelerometer, reflectance and MQTT](#accelerometer-reflectance-and-mqtt)
- [Real time clock](#real-time-clock)
- [Battery level](#battery-level)

### Hello world
```c
void zmain(void) {
   
   printf("\nHello, world!\n");
   
   while (true) {
      vTaskDelay(100); // sleep (in an infinite loop)
   }
}
```
---

### Name and age
```c
void zmain(void) {
   char name[32];
   int age;
   
   printf("Enter your name: ");
   // fflush(stdout);
   scanf("%s", name);
   printf("Enter your age: ");
   // fflush(stdout);
   scanf("%d", &age);
   
   printf("You are [%s], age = %d\n", name, age);
   
   while (true) {  
      BatteryLed_Write(!SW1_Read()); // Switch led off (0) or on (1)
      vTaskDelay(100);  
   }  
}
```
---

### Tick timer
```c
void zmain(void) {

   TickType_t Ttime = xTaskGetTickCount(); // Get current time
   TickType_t PreviousTtime = 0;

   while (true) {
      while (SW1_Read()) vTaskDelay(1); // Loop until user presses button
      Ttime = xTaskGetTickCount();      // Take time at button press
      int diff1 = (int)(Ttime - PreviousTtime) / 1000 % 60;
      int diff2 = (int)(Ttime - PreviousTtime) % 1000;
      printf("The amount of time between button presses is: %d.%d seconds\n", diff1, diff2);
      while (!SW1_Read()) vTaskDelay(1); // Loop while user is pressing the button
      PreviousTtime = Ttime;
   }
   
}
```
---


### Button
```c
void zmain(void) {

   BatteryLed_Write(0); // Switch led off (0) or on (1)
   
   // SW1_Read() returns zero when button is pressed
   // SW1_Read() returns one when button is not pressed
   
   bool led = false;
   
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
}  
```
---

### Button and tick timer
```c
void zmain(void) {
   while (true) {
      printf("Press button within 5 seconds!\n");
      TickType_t Ttime = xTaskGetTickCount();
      bool timeout = false;
      while (SW1_Read() == 1) {
         if (xTaskGetTickCount() - Ttime > 5000U) {
            timeout = true;
            break;
         }
         vTaskDelay(10);
      }
      if (timeout) {
         printf("You didn't press the button\n");
      } else {
         printf("Good job\n");
         while (SW1_Read() == 0) vTaskDelay(10);
      }
   }
}
```
---

### Ultrasonic sensor
```c
void zmain(void) {
   
   Ultra_Start();
   
   while (true) {
      int d = Ultra_GetDistance(); // Get distance in centimeters
      printf("distance = %d\r\n", d);
      vTaskDelay(200);
   }
}
```
---

### IR receiver
```c
void zmain(void) {
   
   IR_Start();
   IR_flush(); // Clear IR receive buffer
   
   bool led = false;
   while(true) {
      IR_wait();  // Wait for IR command
      led = !led;
      BatteryLed_Write(led);
      if (led) {
         printf("Led is ON\n");
      } else {
         printf("Led is OFF\n");
      }
   }    
}
```
---

### IR receiver with raw data
**Note**: raw data is used when you know how your remote modulates data and you want to be able detect which button was actually pressed. Typical remote control protocols requires a protocol specific state machine to decode button presses. Writing such a state machine is not trivial and requires that you have the specification of your remotes modulation and communication protocol
```c
void zmain(void) {
   
   uint32_t IR_val;
   
   IR_Start();
   IR_flush();
   
   // print received IR pulses and their lengths
   while (true) {
      if (IR_get(&IR_val, portMAX_DELAY)) {
         int l = IR_val & IR_SIGNAL_MASK; // get pulse length
         int b = 0;
         if ((IR_val & IR_SIGNAL_HIGH) != 0) b = 1; // get pulse state (0/1)
         printf("%d %d\r\n", b, l);
      }
   }    
} 
```
---

### Reflectance
```c
void zmain(void) {
   struct sensors_ ref;
   struct sensors_ dig;

   // Set center sensor threshold to 11000 and others to 9000
   reflectance_start();
   reflectance_set_threshold(9000, 9000, 11000, 11000, 9000, 9000);

   while (true) {
      
      // Read and print raw sensor values
      reflectance_read(&ref);
      printf("%5d %5d %5d %5d %5d %5d\r\n", ref.L3, ref.L2, ref.L1, ref.R1, ref.R2, ref.R3);       
      
      // Read and print digital values that are based on threshold. 0 = white, 1 = black
      reflectance_digital(&dig);
      printf("%5d %5d %5d %5d %5d %5d\r\n", dig.L3, dig.L2, dig.L1, dig.R1, dig.R2, dig.R3);        
      
      vTaskDelay(200);
   }
}
```
---

### Motor
```c
void zmain(void) {
   motor_start();              // Enable motor controller
   motor_forward(0, 0);        // Set speed to zero to stop motors

   vTaskDelay(3000);
   
   motor_forward(100, 2000);   // Moving forward
   motor_turn(200, 50, 2000);  // Turn
   motor_turn(50, 200, 2000);  // Turn
   motor_backward(100, 2000);  // Moving backward
   
   motor_forward(0, 0);        // Stop motors

   motor_stop();               // Disable motor controller
   
   while (true) vTaskDelay(100);
}
```
---

### Accelerometer
```c
void zmain(void) {

   struct accData_ data;
   
   printf("Accelerometer test...\n");
   if (!LSM303D_Start()){
      printf("LSM303D failed to initialize! Program is ending!\n");
      vTaskSuspend(NULL);
   } else {
      printf("Device OK.\n");
   }
   
   while (true) {
      LSM303D_Read_Acc(&data);
      printf("%8d %8d %8d\n",data.accX, data.accY, data.accZ);
      vTaskDelay(50);
   }
}
```
---

### MQTT
```c
void zmain(void) {

   int ctr = 0;

   printf("\nBoot\n");
   send_mqtt("Zumo01/debug", "Boot");
   BatteryLed_Write(0);

   while (true) {
      printf("Ctr: %d, Button: %d\n", ctr, SW1_Read());
      print_mqtt("Zumo01/debug", "Ctr: %d, Button: %d", ctr, SW1_Read());
      vTaskDelay(1000);
      ctr++;
   }
}
```
---

### Accelerometer, reflectance and MQTT
```c
void zmain(void) {
   
   struct accData_ data;
   struct sensors_ ref;
   struct sensors_ dig;
   
   if (!LSM303D_Start()) {
      printf("LSM303D failed to initialize! Program is ending!\n");
      vTaskSuspend(NULL);
   } else {
      printf("Accelerometer Ok...\n");
   }
   
   int ctr = 0;
   reflectance_start();
   while (true) {
      LSM303D_Read_Acc(&data);
      // send data when we detect a hit and at 10 second intervals
      if (data.accX > 1500 || ++ctr > 1000) {
         printf("Acc: %8d %8d %8d\n", data.accX, data.accY, data.accZ);
         print_mqtt("Zumo01/acc", "%d,%d,%d", data.accX, data.accY, data.accZ);
         reflectance_read(&ref);
         printf("Ref: %8d %8d %8d %8d %8d %8d\n", ref.L3, ref.L2, ref.L1, ref.R1, ref.R2, ref.R3);       
         print_mqtt("Zumo01/ref", "%d,%d,%d,%d,%d,%d", ref.L3, ref.L2, ref.L1, ref.R1, ref.R2, ref.R3);
         reflectance_digital(&dig);
         printf("Dig: %8d %8d %8d %8d %8d %8d\n", dig.L3, dig.L2, dig.L1, dig.R1, dig.R2, dig.R3);
         print_mqtt("Zumo01/dig", "%d,%d,%d,%d,%d,%d", dig.L3, dig.L2, dig.L1, dig.R1, dig.R2, dig.R3);
         ctr = 0;
      }
      vTaskDelay(10);
   }
}  
```
---

### Real time clock
```c
void zmain(void) {

   RTC_Start();
   RTC_TIME_DATE now;

   // Set current time
   now.Hour = 12;
   now.Min = 34;
   now.Sec = 56;
   now.DayOfMonth = 25;
   now.Month = 9;
   now.Year = 2018;
   RTC_WriteTime(&now); // Write the time to real time clock

   while (true) {
      if (SW1_Read() == 0) {
      
         RTC_DisableInt();       // Disable Interrupt of RTC Component
         now = *RTC_ReadTime();  // Copy the current time to a local variable
         RTC_EnableInt();        // Enable Interrupt of RTC Component
         
         printf("%2d:%02d.%02d\n", now.Hour, now.Min, now.Sec);
         while (SW1_Read() == 0) vTaskDelay(50);
      }
      vTaskDelay(50);
   }
}
```
---

### Battery level
```c
void zmain(void) {
   
   int16 adcresult = 0;
   float volts = 0.0;
   ADC_Battery_Start();
   BatteryLed_Write(0);

   while (true) {
      char msg[80];
      ADC_Battery_StartConvert(); // Start sampling
      
      // wait for ADC converted value
      if (ADC_Battery_IsEndConversion(ADC_Battery_WAIT_FOR_RESULT)) {
         
         adcresult = ADC_Battery_GetResult16(); // get the ADC value (0 - 4095)
         
         // convert value to Volts, you need to implement the conversion
         
         // Print both ADC results and converted value
         printf("%d %f\r\n", adcresult, volts);
      }
      
      vTaskDelay(500);
   }
}  
```
