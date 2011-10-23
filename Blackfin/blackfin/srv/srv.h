#ifndef SRV_H_
#define SRV_H_

#include "config.h"

/* general macros */

#define countof(a)      (sizeof(a)/sizeof(a[0]))

#define PWM_OFF 0
#define PWM_PWM 1
#define PWM_PPM 2
#define PWM_UART 3

#define FOUR_TWO_ZERO 1
#define FOUR_TWO_TWO  2



/* Misc Init */
//void init_io ();
//void init_videoBuffer(void);
extern void srv_initPWM();
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
void init_analog();
unsigned int analog(unsigned int);
void read_analog();
void read_analog_4wd();
unsigned int analog_4wd(unsigned int);

/* Relay*/
void srv_relayOn();
void srv_relayOff();

/* Image */
void srv_sendFrame();
void srv_grabProcessAndSendFrame();
void srv_sendRawYUV(unsigned char *srcBuffer);
void srv_sendRawYUVVideo(unsigned char **srcBuffer);
void srv_relayOn();
void srv_overlayOn();
void srv_overlayOff();
void srv_changeImageQuality ();
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
void initPWM ();
void setPWM (int mleft, int mright);


/* Clock */
/*void initTMR4 ();
void delayMS (int delay);  // delay up to 100000 millisecs (100 secs)
void delayUS (int delay);  // delay up to 100000 microseconds (.1 sec)
void delayNS (int delay);  // delay up to 100000 nanoseconds (.0001 sec)
*/

/* Globals */
extern int pwm1_mode, pwm2_mode, pwm1_init, pwm2_init, xwd_init, tilt_init, analog_init;
extern int lspeed, rspeed, lspeed2, rspeed2, base_speed, base_speed2, lcount, rcount;
extern int move_start_time, move_stop_time, move_time_mS, robot_moving;
extern int sonar_data[];
extern unsigned int frame_diff_flag, horizon_detect_flag, invert_flag;
extern unsigned int uart1_flag, thumbnail_flag;
extern unsigned int segmentation_flag, edge_detect_flag, frame_diff_flag, horizon_detect_flag;
extern unsigned int obstacle_detect_flag;
extern unsigned int blob_display_flag;
extern unsigned int blob_display_num;
extern unsigned int edge_thresh;
extern unsigned int master;  // SVS master or slave ?
extern unsigned int stereo_sync_flag;
extern unsigned int stereo_processing_flag;
extern int svs_sensor_width_mmx100;
extern int svs_right_turn_percent;
extern int svs_turn_tollerance_percent;
extern int svs_calibration_offset_x, svs_calibration_offset_y;
extern int svs_centre_of_disortion_x, svs_centre_of_disortion_x;
extern int svs_scale_num, svs_scale_denom, svs_coeff_degree;
extern long* svs_coeff;
extern int svs_width, svs_height;
extern int svs_enable_horizontal;
extern int svs_ground_y_percent;
extern int svs_ground_slope_percent;
extern int svs_enable_ground_priors;
extern int svs_enable_mapping;
extern int svs_disp_left, svs_disp_right, svs_steer;
extern unsigned char version_string[];

#endif

