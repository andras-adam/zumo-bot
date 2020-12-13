# Zumo Bot (with Metropolia PSoC Adapter)

Controlling a Zumo Bot using the Metropolia PSoC Adapter. Assignments include line following and obstacle evasion; using reflectance, IR and ultrasonic sensors. The project was meant to be completed as a team assignment for 2-3 persons.

#### Team members
* [András Ádám](https://github.com/NeoAren) ([andras.h](/ZumoBot.cydsn/andras.h), [andras.c](/ZumoBot.cydsn/andras.c))
* [Vasily Davydov](https://github.com/vas-dav) ([vasilydavydov.h](/ZumoBot.cydsn/vasilydavydov.h), [vasilydavydov.c](/ZumoBot.cydsn/vasilydavydov.c))

#### Metropolia PSoC Adapter
- Documentation available [here](/Documentation.md)

#### Table of contents
- [Motor control and distance sensor](#motor-control-and-distance-sensor)
	- [Assignment 1-1](#assignment-1-1)
	- [Assignment 1-2](#assignment-1-2)
	- [Assignment 1-3](#assignment-1-3)
- [Reflectance sensors and IR sensor](#reflectance-sensors-and-ir-sensor)
  	- [Assignment 2-1](#assignment-2-1)
	- [Assignment 2-2](#assignment-2-2)
	- [Assignment 2-3](#assignment-2-3)
- [MQTT](#mqtt)
  	- [Assignment 3-1](#assignment-3-1)
	- [Assignment 3-2](#assignment-3-2)
	- [Assignment 3-3](#assignment-3-3)
- [Projects](#projects)
  	- [Sumo wrestling](#sumo-wrestling)
	- [Line following](#line-following)
	- [Maze solving](#maze-solving)

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

## Projects

### Sumo wrestling
**Track**: sumo_ring_w_obstacle

Robot must drive along the line to the edge of the sumo ring and wait for start signal. The start signal is
given with an IR remote. The robot drives around and avoids hitting the red triangle. Robot uses
reflectance sensors to stay inside the ring. After entering the ring robot must stop when user button is
pressed.

Robot is considered to be out of the ring if the center of the robot crosses the line. Instructors will judge
who is out and who is not.

**Start**: Robot drives to the edge of the ring and waits on the line for a start signal. The start signal is given with an IR remote.

Robot must send the following data over MQTT connection:
- Ready indicator when robot stops on the start line and starts to wait for IR start signal: 
	- Subtopic: ready
	- Data: “zumo”
- Start time (systick based time stamp = number of milliseconds since the robot code started)
	- Subtopic: start
- Stop time (when the user button is pressed robot stops and sends stop time stamp)
	- Subtopic: stop
- Run time (from start to finish, number of milliseconds between start and stop)
	- Subtopic: time
- Timestamp, number of milliseconds since boot, of each turn to avoid the obstacle
	- Subtopic: obstacle

For example, Zumo028 sends:
```
Zumo028/ready zumo
Zumo028/start 2301
Zumo028/obstacle 3570
Zumo028/obstacle 4553
Zumo028/obstacle 4988
Zumo028/stop 5210
Zumo028/time 2909
```

### Line following
**Track**: race_track

Robot follows the black line using reflective sensors in the front of the robot. Track total length is about 10 meters. Robot has six reflective sensors. The center sensors are used mainly to keep the robot on the track and side sensors to detect intersections or tight curves on the track. The robot must follow the start and stop rules stated below. The intersections are in a 90-degree angle on a straight line. The competition track is 2 cm wide. Make the robot run as fast as possible.

**Start**: Robot drives to the line and waits on the line for a start signal. The start signal is given with an IR remote.
**Finish**: Robot stops on the second line.
**Hint**: Implement line following first. When your robot is able to follow the line then implement start and stop logic.

Robot must send the following data over MQTT connection:
- Ready indicator when robot stops on the start line and starts to wait for IR start signal:
	- Subtopic: ready
	- Data: “line”
- Start time (systick based time stamp = number of milliseconds since the robot code started)
	- Subtopic: start
- Stop time (when the robot stops on the finish line it sends stop time stamp)
	- Subtopic: stop
- Run time (from start to finish, number of milliseconds between start and stop)
	- Subtopic: time
- Bonus (optional):
	- Timestamp if both centre sensors go off the track
		- Subtopic: miss
	- Timestamp when both centre sensors go back to the track after a miss
		- Subtopic: line
	- You must implement both timestamps to get a bonus

For example, Zumo028 sends:
```
Zumo028/ready line
Zumo028/start 1210
Zumo028/stop 54290
Zumo028/time 53080
```
```
Zumo028/ready line
Zumo028/start 1210
Zumo028/miss 7912
Zumo028/line 8012
Zumo028/miss 32103
Zumo028/line 32630
Zumo028/miss 53223
Zumo028/line 53933
Zumo028/stop 54290
Zumo028/time 53080
```

### Maze solving
**Track**: Grid_x (where x ∈ {1, 2, 3, 4, 5})

Robot finds a path through a grid by following lines and taking turns at intersections to avoid obstacles.

**Start**: Robot drives to the line and waits on the line for a start signal. The start signal is given with an IR remote.
**End**: Robot has run straight for a while without seeing an intersection.
**Obstacles**: There are four bricks at random locations, never on the first, second or last full-width row. They are never oriented diagonally.

Robot must send the following data over MQTT connection:
- Ready indicator when robot stops on the start line and starts to wait for IR start signal:
	- Subtopic: ready
	- Data: “maze”
- Start time (systick based time stamp = number of milliseconds since the robot code started)
	- Subtopic: start
- Stop time (when the robot stops on the finish line it sends stop time stamp)
	- Subtopic: stop
- Run time (from start to finish, number of milliseconds between start and stop)
	- Subtopic: time
- Bonus (optional):
	- Send coordinates of each intersection the robot visits
		- Subtopic: position

For example, Zumo028 sends:
```
Zumo028/ready maze
Zumo028/start 1210
Zumo028/stop 24290
Zumo028/time 23080
```
```
Zumo028/ready maze
Zumo028/start 1210
Zumo028/position 0 0
Zumo028/position 0 1
Zumo028/position 0 2
Zumo028/position 0 3
Zumo028/position 0 4
Zumo028/position 0 5
Zumo028/position -1 5
Zumo028/position -2 5
Zumo028/position -2 6
Zumo028/position -2 7
…
Zumo028/position 0 13
Zumo028/stop 24290
Zumo028/time 23080
```
