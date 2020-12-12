/* ========================================
 * Maze Project by Vasily Davydov & András Ádám
 * Metropolia 2020
 * ========================================
*/

#include <maze.h>



void maze_main () 
{
    //define variables
    // west -> left, east -> right, north -> up
    const int north = 0;
    const int west = -1;
    const int east = 1;
    const int x_limit = 3;
    const int y_limit = 11;
    const int dist_obstacle = 15;
    //define structure variables
    struct sensors_ sensors; 
    struct position pos = {0, 0, north};
    
    //engine launch
    startup(true, true, true, true, true);
    //completing the maze till the top
    while(pos.y < 11)
    {
        //Going north if no obstacle
        if (Ultra_GetDistance() > dist_obstacle)
        {
            printf("if runs\n");
            maze_line_follow(&sensors);
            pos.y++;
        }else
        {
            if(pos.direction == north)
            {
                if(pos.x == x_limit)
                {
                    turn_maze(&sensors, &pos, west, dist_obstacle);
                }else if(pos.x == -x_limit)
                {
                    turn_maze(&sensors, &pos, east, dist_obstacle);
                }
            }
            //turn robot west if possible, otherwise east
            if(pos.direction == north)
            {
                turn_maze(&sensors, &pos, west, dist_obstacle);
                if(pos.direction != west)
                {
                    turn_maze(&sensors, &pos, east, dist_obstacle);
                }
            }
            printf("else runs\n");
           
            //find path towards north
            while(pos.direction != north)
            {
                //counting on x-axis & moving into next intersection
                maze_line_follow(&sensors);
                pos.x+=pos.direction;
                turn_maze(&sensors, &pos, north, dist_obstacle);
                if(pos.direction != north && pos.x == x_limit*pos.direction)
                {
                    //changng the direction on x_axis
                    tank_turn(180);
                    pos.direction *= -1;
                }
            }
        }
        
         
    }
    shut();
    
}





void maze_line_follow(struct sensors_ *sensors)
{
    reflectance_digital(sensors);
    //staying on the line
    while(!getRefValues(sensors, 0,0,1,1,0,0))
    {
        while(sensors->L1 == 0 && sensors->R1 == 1)
        {
            tank_turn(-1);
            reflectance_digital(sensors);
        }
        while(sensors->L1 == 1 && sensors->R1 == 0)
        {
            tank_turn(1);
            reflectance_digital(sensors);
        }
        printf("We are on the line\n");
        motor_forward(150,5);
        reflectance_digital(sensors);
    }
    while(sensors->L3 == 0 && sensors->R3 == 0)
    {
        if(sensors->R2 == 0 && sensors->L2 == 1)
        { 
            //Left Turn
            while(sensors->R2 == 0 && sensors->L2 == 1)
            {
                tank_turn(1);
                reflectance_digital(sensors);
            }
            //correction left
            //tank_turn(2);
        }
        if(sensors->R2 == 1 && sensors->L2 == 0)
        { 
            //Right Turn
            while(sensors->R2 == 1 && sensors->L2 == 0)
            {
                tank_turn(-1);
                reflectance_digital(sensors);
            }
            //corection right
            //tank_turn(-2);
        }
        motor_forward(150,5);
        reflectance_digital(sensors);
    }
    motor_forward(0,0);
    
    
}
















void turn_maze(struct sensors_ *sensors, struct position *pos,int final_direction, int dist_obstacle)
{
    int left_speed = pos->direction >= 0 ? 20 : 200 ;
    int right_speed = pos->direction >= 0 ? 200 : 20;
    uint16 delay = 0;
    //going on the line
    while(!getRefValues(sensors, 0, 0, 1, 1, 0, 0))
    {
        SetMotors(0,0,left_speed, right_speed, 10);
        reflectance_digital(sensors);
        delay+=10;
    }
    printf("Distance is %d\n", Ultra_GetDistance());

    //Checking for the obstacle
    if (Ultra_GetDistance() > dist_obstacle)
    {
        pos->direction = final_direction;
    }else 
    {
        SetMotors(1, 1,pos->direction >= 0 ? 20 : 200, pos->direction >= 0 ? 200 : 20, delay);
    }
    //Stopping motors
    motor_forward(0,0);
}

        

    
    
    
    
    












/* [] END OF FILE */
