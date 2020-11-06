#ifndef _SENSEHAT_H_
#define _SENSEHAT_H_
//
// Sense Hat Unchained - function definitions
//
// Copyright (c) 2017 Larry Bank
// email: bitbank@pobox.com
//
// This program is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with this program.  If not, see <http://www.gnu.org/licenses/>.
//
//
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>

#include <fcntl.h>

#include <sys/ioctl.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <sys/types.h>

#include <linux/i2c.h>
#include <linux/i2c-dev.h>
#include <linux/fb.h>
#include <math.h>
#include <sys/time.h>
#include <time.h>
#include <linux/input.h>
#include <i2c/smbus.h>
#include <errno.h>
#include "../../common/vector3.h"


// bit definitions for joystick
#define JOY_DOWN 1
#define JOY_UP   4
#define JOY_LEFT 16
#define JOY_RIGHT 2
#define JOY_ENTER 8
#define FILEPATH "/dev/fb1"
#define JOYSTICK_FILE "/dev/input/event0"
#define LED_MAX_CUST 64
#define FILESIZE (LED_MAX_CUST * sizeof(uint16_t))

#define twoG_LSB 0.061
#define G_GAIN  0.017453293
#define PI 3.141592f
#define RAD_TO_DEG 57.29578f
#define AA 0.97

#define Q_ANGLE 0.01
#define Q_GYRO 0.0003
#define R_ANGLE 0.01

#define ACCEL_ADDR 0x6a
#define MAGN_ADDR 0x1c
#define JOY_ADDR 0xf2

#define RED 0xF800

// Joystick enum based on event codes
typedef enum
{
    UP = 103,
    DOWN = 108,
    LEFT = 105,
    RIGHT = 106,
    ENTER = 28
} JoystickDir;

typedef enum
{
    RELEASE = 0,
    PRESSED = 1,
    HOLD = 2
} JoystickState;

typedef enum
{
  FALLING = 0,
  WALKING = 1,
  RUNNING = 2,
  JUMPING = 3,
  STATIONARY = 4

} ActivityState;

typedef struct
{
    JoystickDir dir;
    char *direction;
    JoystickState state;
    char *status;
} Joystick;

int mymillis();

//
// Read the magnetometer values for x/y/z
//
int shGetMagneto(int *Mx, int *My, int *Mz);
//
// Set the pixel the given color. The display will flicker
// if it's updated for each pixel change, so it's best
// to change the pixels for the current "frame", then
// enable the update to the array. The color is in RGB565
// format and the valid range of x/y is 0-7
//
int shSetPixel(int x, int y, uint16_t color, int bUpdate, uint16_t * map_headptr, int * pfbfd);
int drawActivity(ActivityState state, uint16_t * map_headptr, int * pfbfd); 
int setMap(uint16_t color, uint16_t * map, int * pfbfd);
//
// Initialize the sensors on the Sense Hat
// iChannel = I2C channel number (0/1/2)
//
int shInit(int iChannel, int * pfbfd);
//
// Frees resources and closes handles
//
void shShutdown(int * pfbfd, uint16_t * map);
int shGetGyro(int * gyroRates);
int shGetAccel(int *Ax, int *Ay, int *Az);
int shGet2GAccel(Vector3 * anglesArr);
int shGet500DPSComplementary(float * CFAnglesArr, int * rateGArr, float * anglesArr, int * intStart);
int mapLEDFrameBuffer(uint16_t **  map, int * pfbfd);
void accelToAngle(float * accelAngles, float * accelRadians);
unsigned char shReadJoystick(int * pfbfd);
int initJoystick(int *fd);
int readJoystick(int *fd, Joystick *joy);
void checkJoystickDir(int evCode, Joystick *joy);
void checkJoystickState(int evType, Joystick *joy);
int shGet500DPSKalman(float * kalmanAngles, int * rateGArr, float * anglesArr, int * startInt, float * bias, float * XP, float * YP);

#endif // _SENSEHAT_H_
