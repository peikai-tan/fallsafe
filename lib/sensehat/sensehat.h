#ifndef _SENSEHAT_H_
#define _SENSEHAT_H_
// This file contains the function declaration, macros required
// to initialise and read data from SenseHat sensors.
//
// Ng Cheuk Fung and Edwin has added several function to support
// the formatting of accelerometer sensor data, event listener
// to read joystick inputs and setting SenseHat LED matrix.
// The file only contains necessary functions required for the FallSafe
// fall detection program. Therefore, it contains partial implementation
// from the original library.
//
// Functions written and provided by Larry Bank (author of library)
// shInit()
// shGetAccel()
// i2cRead()
// i2cWrite()
//
// Functions written and modified by Ng Cheuk Fung and Edwin Tang
// shSetPixel()
// drawActivity()
// setMap()
// shGet2GAccel()
// shShutdown()
// mapLEDFrameBuffer()
// initJoystick()
// readJoystick()
// checkJoystickDir()
// checkJoystickState()
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

#define RED 0xF800
#define GREEN 0x07E0
#define PURPLE 0x780F
#define ORANGE 0xFD20
#define OLIVE 0x7BE0
#define DIM_BLUE 0x1111

/**
* typedef JoystickDir - Joystick direction enum based on event codes.
*
* Joystick direction enum based on event codes
*/
typedef enum
{
    UP = 103,
    DOWN = 108,
    LEFT = 105,
    RIGHT = 106,
    ENTER = 28
} JoystickDir;

/**
* typedef JoystickState - Joystick state enum based on event codes.
*
* Joystick state enum based on event codes
*/
typedef enum
{
    RELEASE = 0,
    PRESSED = 1,
    HOLD = 2
} JoystickState;

/**
* typedef ActivityState - Possible activity states of the user
*
* Possible activity states of the user
*/
typedef enum
{
  FALLING = 0,
  WALKING = 1,
  RUNNING = 2,
  JUMPING = 3,
  STATIONARY = 4,
  UNKNOWN = 5

} ActivityState;

/**
 * typedef struct Joystick - Store joystick information
 * @dir: Joystick Direction Enumerator.
 * @direction: Joystick Direction
 * @state: Joystick State
 * @status: Joystick status
 */
typedef struct
{
    JoystickDir dir;
    char *direction;
    JoystickState state;
    char *status;
} Joystick;

/**
 * shSetPixel() - Set a pixel on the 8x8 sensehat matrix with preferred colour
 * @x: Integer value of the pixel's column to set. X coordinate
 * @y: Integer value of the pixel's row to set. Y coordinate
 * @color: 16 bits of colour to set for the pixel
 * @bUpdate: 0 or 1 value, if 1 set the pixel, else don't set.
 * @map_headptr: Pointer to the map's head
 * @pfbfd: Pointer to frame buffer file descriptor
 *
 * This function takes in x and y coordinate system of the pixel to write.
 * Technically, the pixel is in a 1D array starting from the top of the
 * sensehat. Some formatting has be done to write the appropriate 1D index
 * to write.
 *
 * Return: Int value if the function has been successfully called.
 *
 * 1 - Parameters are valid and successfully called.
 * 0 - One of the parameters provided are invalid, function cannot be called.
 */
int shSetPixel(int x, int y, uint16_t color, int bUpdate, uint16_t * map_headptr, int * pfbfd);

/**
 * drawActivity() - Takes in a state enum and draws on the LED matrix.
 * @state: Enumerator of the state, e.g. STATIONARY, WALKING, etc.
 * @map_headptr: Pointer to the map's head
 * @pfbfd: Pointer to frame buffer file descriptor
 *
 * The main program will constantly collect accelerometer and predict the data
 * by the model. It will then draw to the matrix as an output of the state.
 * Draw S on matrix if state is stationary.
 * Draw R on matrix if state is running.
 * Draw W on matrix if state is walking.
 * Draw U on matrix if state is unknown.
 * Draw J on matrix if state is jumping.
 *
 * Return: An integer value to indicate if the drawActivity function has
 * ran without errors.
 *
 * 1 - No errors
 * 0 - Error occurred
 */
int drawActivity(ActivityState state, uint16_t * map_headptr, int * pfbfd);

/**
 * setMap() - Set all the pixels in the 8x8 sensehat to a specific colour.
 * Useful to wipe all pixels.
 *
 * @color: 16 bit colour representation to set to the entire sensehat matrix
 * @map: Pointer of the matrix map
 * @pfbfd: Pointer to framebuffer file descriptor
 *
 * Helper function to set all pixels to a specific colour.
 *
 * Return: Integer value if the function called without errors.
 * 1 - No errors
 * 0 - Error occurred, probably invalid pointer for frame buffer.
 */
int setMap(uint16_t color, uint16_t * map, int * pfbfd);

/**
 * shInit() - Opens file system handles to the I2C devices
 *
 * @iChannel: Channel of i2c, pi3 should be using 1.
 * @pfbfd: Pointer to framebuffer file descriptor
 *
 * Setup all the sensor settings, open file descriptors and handles before reading and writing
 * to/from sensors.
 *
 * Return: Integer value if initialisation has succeeded without errors
 * 1 - No error
 * 0 - Error occurred.
 */
int shInit(int iChannel, int * pfbfd);

/**
 * shShutDown() - Properly close all file descriptors when not in used for
 * better resource management.
 *
 * @pfbfd: Pointer to the framebuffer file descriptor
 * @map: Pointer to the 8x8 sensehat map array
 *
 * Sanitisation function to close all opened file descriptors and release
 * files descriptors and resources back.
 */
void shShutdown(int * pfbfd, uint16_t * map);

/**
 * shGetAccel() - Get accelerometer sensors via i2c.
 *
 * @Ax: Pointer to the raw value of X provided by the sensor.
 * @Ay: Pointer to the raw value of Y provided by the sensor.
 * @Az: Pointer to the raw value of Z provided by the sensor.
 *
 * Provided by the SenseHat unchained library. Allocate an unsigned char
 * array as buffer for the data from the sensor. Read from address using
 * i2c read and check if received all chars. Format the data and set back
 * to output pointers.
 *
 * Return: Integer value if function called without errors.
 * 1 - No errors.
 * 0 - Error occurred.
 */
int shGetAccel(int *Ax, int *Ay, int *Az);

/**
 * shGet2GAccel() - Convert the raw accelerometer data to sensible data with 2G sensitivity
 *
 * @accelArr: Pointer to a Vector3 of accelerometer array
 *
 * Based on sensitivty set initially on setup, we need to format the raw data based on the
 * settings we have made. In the setup, we set to 2G sensitivity therefore we need to scale
 * the data by twoG_LSB. The return value will be in milli-Gs, we divide again to get in G
 * units. To match the open source fall data which was collected by python drivers, we
 * need to negate the values of x and y to match theirs.
 *
 * Return: Integer value to indicate if function has been called without errors.
 * 1 - No errors
 * 0 - Error occurred
 */
int shGet2GAccel(Vector3 * accelArr);

/**
 * mapLEDFrameBuffer() - Map the entire framebuffer to given memory range
 *
 * @map: Pointer to an array of uint16_t which is storing the LED map
 * @pfbfd: Pointer to framebuffer file descriptor
 *
 * To manipulate the LED matrix, we need to map the framebuffer to the map.
 *
 * Return - Integer value to indicate if mapping has succeed
 * 1 - Mapping succeeded
 * 0 - Error occurred
 */
int mapLEDFrameBuffer(uint16_t **  map, int * pfbfd);

/**
 * initJoystick() - Initialise joystick with input event buffer
 *
 * @fd: File descriptor to write to
 *
 * Initialises the joystick input event by opening the file to read and to
 * gain I/O control
 *
 * Return - Integer value to indicate function call successfull
 * 0 - Succeed
 * -1 - Error occurred
 */
int initJoystick(int *fd);

/**
 * readJoystick() - Get joystick information from input event buffer
 *
 * @fd: File descriptor to read from
 * @joy: Joystick object to state and direction of the joystick to
 *
 * Reads joystick state from input event buffer via input event struct
 *
 * Return - Integer value to indicate function call successful
 * 1 - Successful
 * 0 - Error occurred
 */
int readJoystick(int *fd, Joystick *joy);

/**
 * checkJoystickDir() - Determine joystick direction
 *
 * @evCode: Event code of joystick in input event to determine joystick direction
 * @joy: Joystick struct
 *
 * Helper function to get direction and store in joystick struct
 * The directions are:
 * UP, DOWN, LEFT, RIGHT, ENTER
 */
void checkJoystickDir(int evCode, Joystick *joy);

/**
 * checkJoystickState() - Determine joystick state
 *
 * @evType: Event type of joystick
 * @joy: Joystick struct
 *
 * Helper function to interpret event type with three states:
 * RELEASE: When the joystick is being released
 * PRESSED: When the joystick is being pressed
 * HOLD: When the joystick is being held in place
 */
void checkJoystickState(int evType, Joystick *joy);

#endif // _SENSEHAT_H_
