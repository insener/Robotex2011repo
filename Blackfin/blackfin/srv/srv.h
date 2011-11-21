#ifndef SRV_H_
#define SRV_H_

#include "config.h"

//////////////////////////////
// Public global constant definitions
//////////////////////////////
#define countof(a)      (sizeof(a)/sizeof(a[0]))

#define FOUR_TWO_ZERO 1
#define FOUR_TWO_TWO  2
#define OBSTACLE      0
#define OBSTACLE_NON  1

//////////////////////////////
// Public global type definitions
//////////////////////////////
enum SwitchState
{
    switchNotKnown,
    switchOn,
    switchOff
};

/* Misc Init */
//void init_io ();
//void init_videoBuffer(void);
extern void srv_initRobotFunctions();
extern void srv_initVariables();
void clear_sdram ();
void init_heap ();
void show_stack_ptr ();
void show_heap_ptr ();
void reset_cpu ();
unsigned int stack_remaining();
unsigned int ctoi(unsigned char);


/* Serial outputs */
void serial_out_version ();
void serial_out_time ();
void serial_out_flashbuffer ();

/* I2C */
void process_i2c();

/* Analog */
void 		 init_analog();
unsigned int analog(unsigned int);
void 		 read_analog();
void 		 read_analog_4wd();
unsigned int analog_4wd(unsigned int);

/* Relay*/
void srv_relayOn(void);
void srv_relayOff(void);

/* Switch */
int	srv_getPlaySwitchState(void);
int srv_isBlueGoalSelected(void);

/* Image */
void srv_sendFrame(void);
void srv_grabProcessAndSendFrame(void);
void srv_sendRawYUV(unsigned char *srcBuffer);
void srv_sendRawYUVVideo(unsigned char **srcBuffer);
void srv_overlayOn(void);
void srv_overlayOff(void);
void srv_changeImageQuality (void);
void set_caption (unsigned char *str, unsigned int width);

/* Colors */
void init_colors();


/* Failsafe */
void check_battery();

/* Motors */
void init_motors ();
void init_servos();
void motor_set (unsigned char cc, int speed, int *ls, int *rs);
void ppm1_command ();
void ppm2_command ();
void setPWM (int mleft, int mright);


/* Distance sensors */
void srv_getDistanceSensorResults(int *left, int *right);


#endif

