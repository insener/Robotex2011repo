/*
 * motion.c
 *
 */

#include "motion.h"
#include "print.h"
//////////////////////////////
// Type definitions
//////////////////////////////

//////////////////////////////
// Private global defines
//////////////////////////////
#define MOTION_CMD_SIZE  4

//////////////////////////////
// Private global functions
//////////////////////////////
void sendMotion(unsigned char motor1, unsigned char motor2, unsigned char motor3);

//////////////////////////////
// Private global variables
//////////////////////////////


/*
 * Sends motion command to motor driver
 */
void sendMotion(unsigned char motor1, unsigned char motor2, unsigned char motor3)
{
	unsigned char motionCommand[MOTION_CMD_SIZE];

	motionCommand[0] = 128;  // packet start
	motionCommand[1] = motor1;
	motionCommand[2] = motor2;
	motionCommand[3] = motor3;

	uart_uart1SendChars(motionCommand, MOTION_CMD_SIZE);
}

/*
 * Move forward/backward in given speed. Bigger number
 * in absolute scale means higher speed.
 * -127 ...  -1 backward
 *       0      stop
 *    1 ... 127 forward
 */
void motion_moveStraight(char speed)
{
	int ySpeed;
	int motor1Speed, motor2Speed, motor3Speed;

	motor1Speed = 0;
	motor2Speed = 0;
	motor3Speed = 0;

	// do not allow speed out of limits, upper limit not needed to check
	// due to char max value
	if (speed < -127) speed = -127;
	//if (speed >  127) speed = 127;

	// calculate speeds for motors, only y-scale speed needed
	if (speed > 0)
	{
		// forward
		ySpeed = speed;
		motor3Speed = ySpeed + 128;  // left  motor, right turn direction
		motor2Speed = ySpeed;        // right motor, left  turn direction
		motor1Speed = 0;
	}
	if (speed < 0)
	{
		// backward
		ySpeed = -speed;      		 // take absolute value
		motor3Speed = ySpeed;  		 // left  motor, left  turn direction
		motor2Speed = ySpeed + 128;  // right motor, right turn direction
		motor1Speed = 0;  			 // back
	}

	// values should not exceed 255 in absolute, so conversion to char is OK
	sendMotion((unsigned char)motor1Speed, (unsigned char)motor2Speed, (unsigned char)motor3Speed);
}

/*
 * Move left/right only on x scale in given speed.  Bigger number
 * in absolute scale means higher speed.
 * -127 ...  -1 left
 *       0      stop
 *    1 ... 127 right
 */
void motion_moveSide(char speed)
{
	int xSpeed;
	int motor1Speed, motor2Speed, motor3Speed;

	motor1Speed = 0;
	motor2Speed = 0;
	motor3Speed = 0;
	// do not allow speed out of limits, upper limit not needed to check
	// due to char max value
	if (speed < -127) speed = -127;
	//if (speed >  127) speed = 127;

	// calculate speeds for motors, only x-scale speed needed
	if (speed > 0)
	{
		//right side
		xSpeed = speed;
		motor3Speed = xSpeed + 128;  // left  motor, right turn direction
		motor2Speed = xSpeed + 128;  // right motor, right turn direction
		motor1Speed = xSpeed;  		 // back  motor, left  turn direction
	}
	if (speed < 0)
	{
		//left side
		xSpeed = -speed;
		motor3Speed = xSpeed;  		// left  motor, left  turn direction
		motor2Speed = xSpeed;  		// right motor, left  turn direction
		motor1Speed = xSpeed + 128; // back  motor, right turn direction
	}

	// values should not exceed 255 in absolute, so conversion to char is OK
	sendMotion((unsigned char)motor1Speed, (unsigned char)motor2Speed, (unsigned char)motor3Speed);
}

/*
 *
 */
void motion_moveXY(char xSpeed, char ySpeed)
{
	int rSpeed;
	int motor1Speed, motor2Speed, motor3Speed;

	// do not allow speed out of limits
	if (xSpeed < -127) xSpeed = -127;
	//if (xSpeed >  127) xSpeed = 127;
	if (ySpeed < -127) ySpeed = -127;
	//if (ySpeed >  127) ySpeed = 127;

	// no rotation speed needed
	rSpeed = 0;

	// calculate speeds for motors
	motor1Speed = -2 * xSpeed - ySpeed * 172 / 100 - 2 * rSpeed;  // left
	motor2Speed = -2 * xSpeed + ySpeed * 172 / 100 - 2 * rSpeed;  // right
	motor3Speed = 2 * xSpeed - 2 * rSpeed;  // back
}
/*
 * Stop motors
 */
void motion_stop()
{
	unsigned char motor1Speed, motor2Speed, motor3Speed;

	motor1Speed = 0;
	motor2Speed = 0;
	motor3Speed = 0;

	sendMotion(motor1Speed, motor2Speed, motor3Speed);
}


