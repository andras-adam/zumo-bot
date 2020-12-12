/* ========================================
 * Maze Project by Vasily Davydov & András Ádám
 * Metropolia 2020
 * ========================================
*/


#include<andras.h>
#include <vasilydavydov.h>
struct position{int x; int y; int direction;};

void maze_main (void);
void turn_maze(struct sensors_ *sensors, struct position *pos,int final_direction, int dist_obstacle);
void maze_line_follow(struct sensors_ *sensors);



/* [] END OF FILE */
