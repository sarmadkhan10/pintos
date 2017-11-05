/* Tests producer/consumer communication with different numbers of threads.
 * Automatic checks only catch severe problems like crashes.
 */

#include <stdio.h>
#include "tests/threads/tests.h"
#include "threads/malloc.h"
#include "threads/synch.h"
#include "threads/thread.h"
#include "devices/timer.h"
#include <random.h>


//bridge limit
static const int BRIDGE_LIMIT =3;

//lane and priority 
static const int LEFT_LANE = 0;
static const int RIGHT_LANE = 1;
static const int NORMAL = 0;
static const int PRIORITY = 1;


//allows crossing side
int bridgeDirection = 0;

//counter of right and left vehicles
int on_bridge[2] = {0, 0};
// waiting vehicles {NORMAL,PRIORITY}
int waiting_queue[2][2] = {{0, 0}, {0, 0}};

//semaphores
struct semaphore lock;
struct semaphore wait_lock[2][2];

//user defined method declarations
static void one_vehicle(int direc, int prio);
static void arrive_bridge(int direc, int prio);
static void cross_bridge(int direc, int prio);
static void exit_bridge(int direc, int prio);

static void leftPriorityVehicle(void *aux);
static void rightPriorityVehicle(void *aux);
static void leftVehicle(void *aux);
static void rightVehicle(void *aux);


void narrow_bridge(unsigned int num_vehicles_left, unsigned int num_vehicles_right,
        unsigned int num_emergency_left, unsigned int num_emergency_right);

void test_narrow_bridge(void)
{
    narrow_bridge(0, 0, 0, 0);
    narrow_bridge(1, 0, 0, 0);
    narrow_bridge(0, 0, 0, 1);
    narrow_bridge(0, 4, 0, 0);
    narrow_bridge(0, 0, 4, 0);
    narrow_bridge(3, 3, 3, 3);
    narrow_bridge(4, 3, 4 ,3);
    narrow_bridge(7, 23, 17, 1);
    narrow_bridge(40, 30, 0, 0);
   /* narrow_bridge(30, 40, 0, 0);
    narrow_bridge(23, 23, 1, 11);
    narrow_bridge(22, 22, 10, 10);
    narrow_bridge(0, 0, 11, 12);
    narrow_bridge(0, 10, 0, 10);
    narrow_bridge(0, 10, 10, 0);*/
    pass();
}


static void leftPriorityVehicle(UNUSED void *aux){
    one_vehicle(LEFT_LANE, PRIORITY);
}
static void rightPriorityVehicle(UNUSED void *aux){
    one_vehicle(RIGHT_LANE, PRIORITY);
}
static void leftVehicle(UNUSED void *aux){
    one_vehicle(LEFT_LANE, NORMAL);
}
static void rightVehicle(UNUSED void *aux){
    one_vehicle(RIGHT_LANE, NORMAL);
}

void narrow_bridge(UNUSED unsigned int num_vehicles_left, UNUSED unsigned int num_vehicles_right,
        UNUSED unsigned int num_emergency_left, UNUSED unsigned int num_emergency_right)
{
    
  
    //random init
    random_init (timer_ticks());
    
    // init semaphores
    sema_init(&lock, 1); 
    sema_init(&wait_lock[LEFT_LANE][NORMAL], 0);
    sema_init(&wait_lock[LEFT_LANE][PRIORITY], 0);
    sema_init(&wait_lock[RIGHT_LANE][NORMAL], 0);
    sema_init(&wait_lock[RIGHT_LANE][PRIORITY], 0);

    char thread_name[20];
    
    unsigned int i;
    
    //threads for left normal vehicles
    for (i = 0; i < num_vehicles_left; i++) {
        // create thread name
        snprintf((char *)&thread_name, sizeof thread_name, "Left-Normal:%05d", i);
        thread_create((char *)&thread_name, 0, leftVehicle, NULL);
    }
    
    //threads for right normal vehicles
    for (i = 0; i < num_vehicles_right; i++) {
        // create thread name
        snprintf((char *)&thread_name, sizeof thread_name, "Right-Normal:%05d", i);
        thread_create((char *)&thread_name, 0, rightVehicle, NULL);
    }
    //threads for left priority vehicles
    for (i = 0; i < num_emergency_left; i++) {
        // create thread name
        snprintf((char *)&thread_name, sizeof thread_name, "Left-Priority:%05d", i);
        thread_create((char *)&thread_name, 0, leftPriorityVehicle, NULL);
    }
    
    //threads for right priority vehicles
    for (i = 0; i < num_emergency_right; i++) {
        // create thread name
        snprintf((char *)&thread_name, sizeof thread_name, "Right-Priority:%05d", i);
        thread_create((char *)&thread_name, 0, rightPriorityVehicle, NULL);
    }
}

//for debug print
static void print_bridge_status(void) {
    
    printf("=======\n");
    if(bridgeDirection){
        printf("Direction: Right -> \n");
    }else{
        printf("Direction: <- Left\n");
    }
    printf("Left: Normal: %d, Priority: %d\n",waiting_queue[0][0], waiting_queue[0][1]);
    printf("Right= Normal: %d, Priority: %d\n", waiting_queue[1][0], waiting_queue[1][1]);
    printf("On Bridge= Left: %d, Right: %d\n", on_bridge[0], on_bridge[1]);
    printf("=======\n\n");
    
  
}


static void one_vehicle(int direc, int prio){
    arrive_bridge(direc, prio);
    cross_bridge(direc, prio);
    exit_bridge(direc, prio);
}

static void arrive_bridge(int direc, int prio){
    // acquire lock
    sema_down(&lock);
    
    //check valid direction
    // change direction if bridge not used and no priority vehicles
    if (bridgeDirection != direc && on_bridge[1-direc] == 0 && waiting_queue[1-direc][PRIORITY] == 0) {
        bridgeDirection = 1-bridgeDirection;
    }
    
    //enter bridge
    while (bridgeDirection != direc // our turn  
            || on_bridge[direc] == BRIDGE_LIMIT //bridge is under limit
            || (prio == NORMAL && //no priority vehicles are waiting
                (waiting_queue[LEFT_LANE][PRIORITY] + waiting_queue[RIGHT_LANE][PRIORITY]) > 0)) {
                
        // wait
        waiting_queue[direc][prio]++;
        printf("Waiting: %s", thread_name());
        print_bridge_status();
        // release lock
        sema_up(&lock);
        // wait till condition is met
        sema_down(&wait_lock[direc][prio]);
        // acquire lock
        sema_down(&lock);
        waiting_queue[direc][prio]--;
    }
    
    // update state
    on_bridge[direc]++;
    ASSERT(on_bridge[LEFT_LANE] + on_bridge[RIGHT_LANE] <= BRIDGE_LIMIT);
    // release lock
    sema_up(&lock);

}
static void cross_bridge(UNUSED int direc, UNUSED int prio){
    printf("Entering Bridge: %s \n",thread_name());
    //wait for vehicle to cross
    timer_msleep(random_ulong() % 500);
    printf("Leaving Bridge: %s \n", thread_name());
}

static void exit_bridge(int direc, UNUSED int prio){
    // acquire lock
    sema_down(&lock);
    
    //check direction
    if (on_bridge[direc] == 1) {
        
        // priority vehicle waiting on the opposite direction, change direction
        if (waiting_queue[1-bridgeDirection][PRIORITY] > 0 && waiting_queue[bridgeDirection][PRIORITY] == 0) {
            bridgeDirection = 1-bridgeDirection;
        } else
        //normal vehicle waiting on the opposite side, not on our side, change direction
        if (waiting_queue[1-bridgeDirection][NORMAL] > 0 && waiting_queue[bridgeDirection][PRIORITY] == 0
                && waiting_queue[bridgeDirection][NORMAL] == 0) {
            bridgeDirection = 1-bridgeDirection;
        }
    }
    
    // update state
    on_bridge[direc]--;
    print_bridge_status();
        
    int to_wake_up = BRIDGE_LIMIT - on_bridge[direc];
    
    //signal priority threads
    int i=0;
    for (i = 0;i < waiting_queue[bridgeDirection][PRIORITY] && to_wake_up > 0;i++) {
        sema_up(&wait_lock[bridgeDirection][PRIORITY]);
        to_wake_up--;
    }
   
    //signal normal threads
    for (i = 0;i < waiting_queue[bridgeDirection][NORMAL] && to_wake_up > 0; i++) {
        sema_up(&wait_lock[bridgeDirection][NORMAL]);
        to_wake_up--;
    }
    
    // release lock
    sema_up(&lock);
}


