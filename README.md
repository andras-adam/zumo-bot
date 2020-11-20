# Zumo Bot (with Metropolia PSoC Adapter)

A project to control a Zumo Bot using the Metropolia-PSoC Adapter. Assignments include line following and obstacle evasion, using reflectance, IR and ultrasonic sensors. The project was meant to be completed as a team assignment for 2-3 persons.

#### Team members
* [András Ádám](https://github.com/NeoAren)
* [Vasily Davydov](https://github.com/vas-dav)

#### Metropolia PSoC Adapter
- Example code available [here](/Documentation.md)

## Motor control and distance sensor
In all following exercises the robot must wait for user button to be pressed before it starts running

### Assignment 1-1
Robot control without sensors. Make the robot run the track that is taped to class room floor. Start from the first intersection and try to hit the last one as close as possible. To control motors, you may use only the following motor functions provided in the library: `motor_start`, `motor_stop`, `motor_forward`, `motor_turn`, and `motor_backward`. If you use a real robot you may need to balance the motors for example, make one motor run slightly faster than the other, to make the robot run straight. In the simulator the robot runs in an ideal world with no friction and no variation in the motor quality which removes the need for motor balancing.

### Assignment 1-2
Robot control with ultrasonic distance sensor. Write a program that runs forward and checks periodically the distance sensor reading. If an obstacle is closer than 10 cm robot stops, reverses a bit, and turns ~120 degrees left. Then robot continues to run forward and check for obstacles again. To control motors, you may use only the following motor functions provided in the library: `motor_start`, `motor_stop`, `motor_forward`, `motor_turn`, and `motor_backward`.

### Assignment 1-3
Robot control with ultrasonic distance sensor. Write a program that runs forward and checks periodically the distance sensor reading. If an obstacle is closer than 10 cm robot stops, reverses a bit, and makes a random tank turn of 90 to 270 degrees. Then robot continues to run forward and check for obstacles again. Tank turn is a turn where one motor runs forward and the other backwards. There is no ready-made function for a tank turn in the library so you have to implement your own tank turn function. You must always use function `SetMotors()` to set the motor control values in your own functions. In the library there are two implementations on `SetMotors()` one for the simulator and the other for the real robot. Using Cypress library functions directly will prevent the simulator from working correctly.

## Reflectance sensors and IR sensor

### Assignment 2-1
When the user button (SW1, in the center of the PSoC board) is pressed the robot drives to the first line. On the first line the robot waits for a command from IR-remote using `IR_wait()`. When any IR command is received the robot starts to run forward and stops on the fifth line.

### Assignment 2-2
When the user button is pressed the robot drives to the first line. On the first line the robot waits for a command from IR-remote. When any IR command is received the robot starts to run forward following the line and stops on the second line.

### Assignment 2-3
When user button is pressed the robot drives to the first line. On the first line the robot waits for a command from IR-remote. When any IR command is received the robot starts to run forward. When the robot comes to the first intersection it turns left and then takes the next two intersections to the right. The robot stops on the fourth intersection. Use reflectance sensors for turns. Make turns in very small steps and use sensors to determine when you have turned enough. Algorithm: Turn until your center sensors see white then turn more until at least one center sensor sees black. When you run forward align the robot on the line so that robot sees the next intersection in right angle. In the simulator the turns are accurate because the robot’s physics model is very limited and does not account for battery charge which has a big impact on turn rate on a real robot.

## MQTT
The following assignments require that you have a WiFi network set up, an MQTT broker, and a client that can subscribe to messages that your robot sends. In the simulator you don’t need WiFi or MQTT broker, the MQTT messages you send will be handled by the simulator.

### Assignment 3-1
Write a program that measures the interval between two button presses. The program enters an infinite loop and sends the number of milliseconds since the last button press every time when the user presses the PSoC user button. Use topic `<robot serial number>/button`. If the user holds the button the message is sent only once. Sending multiple messages must require button to be released between messages. You can get the number of milliseconds since boot by calling `xTaskGetTickCount()`. See Zumo library documentation and examples for details.

### Assignment 3-2
Write a program that uses ultrasonic sensor to detect obstacles. When an obstacle is detected the robot reverses and makes a random 90 degree turn to left or right. The turn direction (left/right) is sent to topic `<robot serial number>/turn`.

### Assignment 3-3
Write a program that runs the robot to a line. When the line is reached the robot waits for an IR-remote command. After receiving the IR-command the robot runs forward until it sees another line. The robot stops on the line and sends the elapsed time (from IR-command to the stop line) to topic `<robot serial number>/lap`.
