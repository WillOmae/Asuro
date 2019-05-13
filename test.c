#include "asuro.h"

#define RUN_SPEED 200
#define STOP_SPEED 0

enum obstacle{OBST_NONE = 100, OBST_LEFT = 101, OBST_RIGHT = 102, OBST_AHEAD = 103};
enum motor{MOTOR_NONE = 0, MOTOR_LEFT = 1, MOTOR_RIGHT = 2, MOTOR_BOTH = 3};

void Initialise(void);
void Accelerate(int);
void Decelerate(int);
int GetPreciseObstPosition(void);
int GetSingleObstacleDir(void);
void Manoeuvre(int);
void FollowLine(void);
int CompareSpeeds(void);
unsigned long GetTimeFromStart(void);
unsigned long CountDigits(unsigned long);
unsigned long CountChar(unsigned char*);
void NumToString(unsigned char*, unsigned long, unsigned long);
void WriteLine(unsigned char*, unsigned long, unsigned char*);
void Write(unsigned char*, unsigned long, unsigned char*);

unsigned int speedRight, speedLeft;
unsigned int dirRight, dirLeft;
int startAgain = 0;

int main(void)
{
	Init();
	int obstDir = OBST_NONE;
	speedLeft = speedRight = RUN_SPEED;
	dirLeft = dirRight = FWD;
		
	while(1)
	{
		obstDir = GetPreciseObstPosition();
		
		unsigned char prefix[] = "Obstacle position";
		unsigned char suffix[] = "";
		WriteLine(prefix, obstDir, suffix);
		Manoeuvre(obstDir);
	}
	return 0;
}

/* Function to increment the motor speed */
/* Max speed is 255 */
void Accelerate(int motorSelect)
{
	switch(motorSelect)
	{
		case MOTOR_LEFT:
			if(speedLeft < 255) speedLeft++;
			break;
		case MOTOR_RIGHT:
			if(speedRight < 255) speedRight++;
			break;
		case MOTOR_BOTH:
			if(speedLeft < 255) speedLeft++;
			if(speedRight < 255) speedRight++;
			break;
	}
	MotorSpeed(speedRight, speedLeft);
}

/* Function to decelerate */
/* Min speed is 0 */
void Decelerate(int motorSelect)
{
	switch(motorSelect)
	{
		case MOTOR_LEFT:
			if(speedLeft > 0) speedLeft--;
			break;
		case MOTOR_RIGHT:
			if(speedRight > 0) speedRight--;
			break;
		case MOTOR_BOTH:
			if(speedLeft > 0) speedLeft--;
			if(speedRight > 0) speedRight--;
			break;
	}
	MotorSpeed(speedRight, speedLeft);
}

/* Function to determine a more precise obstacle position */
/* Does [10] runs, then gets the average, rounding it down */
/* Returns an enum of type obstacle */
int GetPreciseObstPosition()
{
	int trialRuns = 0, count, runs = 10;
	for(count = 0; count < runs; count++)
	{
		trialRuns += GetSingleObstacleDir();
	}
	return (trialRuns / runs);
}

/* Function to determine the obstacle position based on switch polling */
/* The value is not very accurate due to capacitor charge/discharge time */
/* Return an enum of type obstacle */
int GetSingleObstacleDir()
{
	int switchState = PollSwitch();
	Msleep(5);
	switchState = PollSwitch();
	if(switchState == 0)
	{
		return OBST_NONE;
	}
	else if(switchState < 8)
	{
		return OBST_RIGHT;
	}
	else
	{
		return OBST_LEFT;
	}
/*	switch(switchState)
	{
		case 0:
			return OBST_NONE;
		case 1:
		case 2:
		case 4:
			return OBST_RIGHT;
		case 8:
		case 16:
		case 32:
			return OBST_LEFT;
		default:
			return OBST_NONE;
	}*/
/*	switch(switchState)
	{
		// Any combination of the right switches.
		case 1: case 2: case 4: case (1+2): case (2+4): case (1+2+4):
			return OBST_RIGHT;
		
		// Any combination of the left switches.
		case 8: case 16: case 32: case (8+16): case (16+32): case (8+16+32):
			return OBST_LEFT;
		
		// Any combination of the 'front' switches.
		case (2+4+8+16): case (4+8+16): case (2+8+16): case (2+4+16): case (2+4+8): case (2+16): case (2+8): case (4+16): case (4+8):
			return OBST_AHEAD;
		
		// No switch.
		case 0:
		default:
			return OBST_NONE;
	}*/
}

/* Function to manoeuvre around obstacles */
/* Pass it the obstacle position */
void Manoeuvre(int obstDir)
{
	switch(obstDir)
	{
		case OBST_LEFT:
			StatusLED(GREEN);
			MotorSpeed(STOP_SPEED, STOP_SPEED);
			Msleep(100);
			MotorDir(RWD, RWD);
			MotorSpeed(RUN_SPEED, RUN_SPEED);
			Msleep(1000);
			MotorSpeed(STOP_SPEED, STOP_SPEED);
			Msleep(100);
			MotorDir(FWD, BREAK);
			MotorSpeed(RUN_SPEED, STOP_SPEED);
			Msleep(1000);
			MotorSpeed(STOP_SPEED, STOP_SPEED);
			Msleep(100);
			MotorDir(FWD, FWD);
			MotorSpeed(RUN_SPEED, RUN_SPEED);
			
			startAgain = 1;
			break;
		case OBST_RIGHT:
			StatusLED(RED);
			MotorSpeed(STOP_SPEED, STOP_SPEED);
			Msleep(100);
			MotorDir(RWD, RWD);
			MotorSpeed(RUN_SPEED, RUN_SPEED);
			Msleep(1000);
			MotorSpeed(STOP_SPEED, STOP_SPEED);
			Msleep(100);
			MotorDir(BREAK, FWD);
			MotorSpeed(STOP_SPEED, RUN_SPEED);
			Msleep(1000);
			MotorSpeed(STOP_SPEED, STOP_SPEED);
			Msleep(100);
			MotorDir(FWD, FWD);
			MotorSpeed(RUN_SPEED, RUN_SPEED);
			
			startAgain = 1;
			break;
		case OBST_NONE:
		default:
			StatusLED(OFF);
			MotorDir(FWD, FWD);
			if(startAgain)
			{
				speedLeft = speedRight = RUN_SPEED;
			}
			Accelerate(MOTOR_BOTH);
			startAgain = 0;
			break;
	}
}

// not working as expected
void FollowLine()
{
	unsigned int lineData[2];
	FrontLED(ON);
	MotorDir(FWD, FWD);
	MotorSpeed(RUN_SPEED, RUN_SPEED);
	while(1)
	{
		LineData(lineData);
		
		if(lineData[0] > (lineData[1]))
		{
			Accelerate(MOTOR_LEFT);
			Decelerate(MOTOR_RIGHT);
		}
		else
		{
			Accelerate(MOTOR_RIGHT);
			Decelerate(MOTOR_LEFT);
		}
	}
}

// not complete; to compare odometry data to equalise speeds
int CompareSpeeds()
{
	unsigned int odoData[2];
	OdometrieData(odoData);
	return MOTOR_BOTH;
}

/* Function to convert a number to a zero-terminated char array */
/* Pass it the predefined char array, length including zero terminator and the number to be converted */
/* Returns void because the string is manipulated internally (pass by reference??) */
void NumToString(unsigned char* string, unsigned long length, unsigned long num)
{
	int count, temp;
	// start from the least place value position
	for(count = (length - 2); count >= 0; count--)
	{
		// find the modulus, the remainder
		temp = num % 10;
		// 0 in ASCII is 48
		// So to convert the int to char, add 48
		string[count] = 48 + temp;
		// divide by 10
		num = num / 10;
	}
	// terminate the char array with the zero
	string[length - 1] = '\0';
}

/* Function to count the number of digits in any number */
/* Pass it the number */
/* Return the count of digits */
unsigned long CountDigits(unsigned long num)
{
	unsigned long count = 1;
	while(1)
	{
		num = num/10;
		if(num == 0)
		{
			break;
		}
		else
		{
			count++;
		}
	}
	return count;
}

/* Function to count the number of char in a string */
/* Pass it the string */
/* Return the char count minus zero-terminator */
unsigned long CountChar(unsigned char* string)
{
	unsigned long count = 0;
	while(string[count] != '\0')
	{
		count++;
	}
	return count;
}

/* Function to write to serial */
/* Pass it the prefix string, num and suffix string */
void Write(unsigned char* prefix, unsigned long num, unsigned char* suffix)
{
	unsigned long length = CountDigits(num) + 1;
	unsigned char string[length];
	NumToString(string, length, num);
	
	SerWrite(prefix, CountChar(prefix));
	SerWrite(string, length);
	SerWrite(suffix, CountChar(suffix));
}

/* Function to write a line to serial */
/* Pass it the prefix string, num and suffix string */
void WriteLine(unsigned char* prefix, unsigned long num, unsigned char* suffix)
{
	unsigned long length = CountDigits(num) + 1;
	unsigned char string[length];
	NumToString(string, length, num);
	unsigned char newline[] = "\n\r";
	
	SerWrite(prefix, CountChar(prefix));
	SerWrite(string, length);
	SerWrite(suffix, CountChar(suffix));
	SerWrite(newline, 2);
}