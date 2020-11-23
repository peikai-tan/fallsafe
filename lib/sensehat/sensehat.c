//
// Combination of Sense Hat Unchained drivers and customed drivers for FallSafe
// Usage.
// 
// A simple set of C functions to initialize and
// read data from the sensors on the Sense Hat
//
// These functions are not meant to be comprehensive
// but merely to be a simple example of using
// standard Linux file system drivers to talk to
// I2C devices.
//
// The Sense Hat consists of the following:
// 8x8 LED array mapped to a microcontroller at 0x46
// 5-way joystick mapped to the same microcontroller
// HTS221 humidity/temp sensor at 0x5F// LPS25H pressure/temp sensor at 0x5C
// LPS25H pressure/temp sensor at 0x5C
// LSM9DS1 accel/gyro/mag mapped to 0x1C (mag) 0x6A (accel)
//
// Written by Larry Bank - 11/10/2017
// Copyright (c) 2017 BitBank Software, Inc.
// bitbank@pobox.com
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
// Any functions which are already fully written by the Sensehat Unchained
// author will be indicated in function header. Else, its written by
// students of FallSafe project.
#include "sensehat.h"
//#define DEBUG
static int file_acc = -1;	   // accelerometer/gyro
static int i2cRead(int iHandle, unsigned char ucAddr, unsigned char *buf, int iLen);
static int i2cWrite(int iHandle, unsigned char ucAddr, unsigned char *buf, int iLen);
char name[256];
/*
 * shInit() - Opens file system handles to the I2C devices
 *
 * @args1 : Channel of i2c, pi3 should be using 1.
 * @args2 : Pointer to framebuffer file descriptor
 *
 * Setup all the sensor settings, open file descriptors and handles before reading and writing
 * to/from sensors.
 * 
 * Return: Integer value if initialisation has succeeded without errors
 * 1 - No error
 * 0 - Error occured.
 */
int shInit(int iChannel, int *pfbfd)
{
	unsigned char ucTemp[32];
	char filename[32];
	struct fb_fix_screeninfo fix_info;

	sprintf(filename, "/dev/i2c-%d", iChannel);
#if defined(DEBUG)
	printf("filename %s\n", filename);
#endif
	*pfbfd = open(FILEPATH, O_RDWR);
	if (*pfbfd == -1)
	{
		perror("Error (call to 'open')");
		goto badexit;
	}

	if (ioctl(*pfbfd, FBIOGET_FSCREENINFO, &fix_info) == -1)
	{
		perror("Error (call to 'ioctl')");
		close(*pfbfd);
		goto badexit;
	}

	if (strcmp(fix_info.id, "RPi-Sense FB") != 0)
	{
		fprintf(stderr, "%s \n", "Error: RPi-Sense FB not found");
		close(*pfbfd);
		goto badexit;
	}

	file_acc = open(filename, O_RDWR);
	if (ioctl(file_acc, I2C_SLAVE, ACCEL_ADDR) < 0)
	{
		fprintf(stderr, "Failed to acquire bus for accelerometer\n");
		goto badexit;
	}
#if defined(DEBUG)
	printf("Aceel %d\n", file_acc);
#endif
	// Init accelerometer/gyroscope
	ucTemp[0] = 0x60; // 119hz accel
	i2cWrite(file_acc, 0x20, ucTemp, 1);
	ucTemp[0] = 0x38; // enable gyro on all axes
	i2cWrite(file_acc, 0x1e, ucTemp, 1);
	ucTemp[0] = 0x28; // data rate + full scale + bw selection
	// bits:        ODR_G2 | ODR_G1 | ODR_G0 | FS_G1 | FS_G0 | 0 | BW_G1 | BW_G0
	// 0x28 = 14.9hz, 500dps
	i2cWrite(file_acc, 0x10, ucTemp, 1); // gyro ctrl_reg1
	return 1;
	// problems opening the I2C handles/addresses
badexit:
	if (file_acc != -1)
	{
		close(file_acc);
		file_acc = -1;
	}
	return 0;
}
/**
 * shSetPixel() - Set a pixel on the 8x8 sensehat matrix with preferred colour
 * @arg1: Integer value of the pixel's column to set. X coordinate
 * @arg2: Integer value of the pixel's row to set. Y coordinate
 * @arg3: 16 bits of colour to set for the pixel
 * @arg4: 0 or 1 value, if 1 set the pixel, else don't set.
 * @arg5: Pointer to the map's head
 * @arg6: Pointer to frame buffer file descriptor 
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
int shSetPixel(int x, int y, uint16_t color, int bUpdate, uint16_t *map_headptr, int *pfbfd)
{
    /* Temporary variable to store the formatted 1D index of given x & y */
	int i = 0;
    /**
     * Check if x and y are within range and the provided framebuffer 
     * is valid. As each row has 8 pixel, 1D index can be calculated by
     * multiplying 8 by the y and finally, adding x which is the offset from
     * the first pixel of each row.
     */
	if (x >= 0 && x < 8 && y >= 0 && y < 8 && *pfbfd >= 0)
	{
		i = (y * 8) + x; 
		if (bUpdate) /* if bUpdate is not 0, set the pixel's colour. */
			map_headptr[i] = color;
		return 1;
	}
	return 0;
}
/**
 * drawActivity() - Takes in a state enum and draws on the LED matrix.
 * @arg1: Enumerator of the state, e.g. STATIONARY, WALKING, etc.
 * @arg2: Pointer to the map's head
 * @arg3: Pointer to frame buffer file descriptor 
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
 * 0 - Error occured
 */
int drawActivity(ActivityState state, uint16_t *map_headptr, int *pfbfd)
{
    /* Counter for loop */
	int i = 0;
    /* 
     * Depending on the state received and utilise a for loop to draw
     * straight lines. Repetitively call shSetPixel to set the pixels on
     * matrix.
     * Return 1 if the drawing is successfully called.
     * If the switch enters default case, invalid state must have been entered
     * and return 0.
     */ 
	switch (state)
	{
	case FALLING:
		for (i = 1; i < 7; ++i)
			shSetPixel(2, i, RED, 1, map_headptr, pfbfd);
		for (i = 3; i < 6; ++i)
			shSetPixel(i, 1, RED, 1, map_headptr, pfbfd);
		for (i = 3; i < 6; ++i)
			shSetPixel(i, 3, RED, 1, map_headptr, pfbfd);
		return 1;
	case WALKING:
		for (i = 1; i < 7; ++i)
		{
			shSetPixel(1, i, GREEN, 1, map_headptr, pfbfd);
			shSetPixel(5, i, GREEN, 1, map_headptr, pfbfd);
		}
		shSetPixel(2, 5, GREEN, 1, map_headptr, pfbfd);
		shSetPixel(3, 4, GREEN, 1, map_headptr, pfbfd);
		shSetPixel(3, 3, GREEN, 1, map_headptr, pfbfd);
		shSetPixel(4, 5, GREEN, 1, map_headptr, pfbfd);
		return 1;
	case RUNNING:
		for (i = 1; i < 8; ++i)
		{
			shSetPixel(1, i, PURPLE, 1, map_headptr, pfbfd);
			if (i == 2 || i == 3 || i == 7)
				shSetPixel(5, i, PURPLE, 1, map_headptr, pfbfd);
		}
		for (i = 2; i < 5; ++i)
		{
			shSetPixel(i, 1, PURPLE, 1, map_headptr, pfbfd);
			shSetPixel(i, 4, PURPLE, 1, map_headptr, pfbfd);
		}
		shSetPixel(3, 5, PURPLE, 1, map_headptr, pfbfd);
		shSetPixel(4, 6, PURPLE, 1, map_headptr, pfbfd);
		return 1;
	case JUMPING:
		for (i = 3; i < 6; ++i)
			shSetPixel(i, 1, ORANGE, 1, map_headptr, pfbfd);
		for (i = 2; i < 7; ++i)
			shSetPixel(4, i, ORANGE, 1, map_headptr, pfbfd);
		shSetPixel(1, 6, ORANGE, 1, map_headptr, pfbfd);
		shSetPixel(2, 7, ORANGE, 1, map_headptr, pfbfd);
		shSetPixel(3, 7, ORANGE, 1, map_headptr, pfbfd);
		return 1;
	case STATIONARY:
		for (i = 2; i < 5; ++i)
		{
			shSetPixel(i, 1, OLIVE, 1, map_headptr, pfbfd);
			shSetPixel(i, 4, OLIVE, 1, map_headptr, pfbfd);
			shSetPixel(i, 7, OLIVE, 1, map_headptr, pfbfd);
		}
		shSetPixel(5, 1, OLIVE, 1, map_headptr, pfbfd);
		shSetPixel(1, 2, OLIVE, 1, map_headptr, pfbfd);
		shSetPixel(1, 3, OLIVE, 1, map_headptr, pfbfd);
		shSetPixel(5, 5, OLIVE, 1, map_headptr, pfbfd);
		shSetPixel(5, 6, OLIVE, 1, map_headptr, pfbfd);
		shSetPixel(1, 7, OLIVE, 1, map_headptr, pfbfd);
		return 1;
	case UNKNOWN:
		for (i = 1; i < 7; ++i)
		{
			shSetPixel(2, i, DIM_BLUE, 1, map_headptr, pfbfd);
			shSetPixel(5, i, DIM_BLUE, 1, map_headptr, pfbfd);
		}
		for (i = 3; i < 5; i++)
		{
			shSetPixel(i, 6, DIM_BLUE, 1, map_headptr, pfbfd);
		}
		return 1;
	default:
		return 0;
	}
}
/**
 * setMap() - Set all the pixels in the 8x8 sensehat to a specific colour.
 * Useful to wipe all pixels.
 * 
 * @args1: 16 bit colour representation to set to the entire sensehat matrix
 * @args2: Pointer of the matrix map
 * @args3: Pointer to framebuffer file descriptor
 * 
 * Helper function to set all pixels to a specific colour.
 * 
 * Return: Integer value if the function called without errors.
 * 1 - No errors
 * 0 - Error occured, probably invalid pointer for frame buffer.
 */
int setMap(uint16_t color, uint16_t *map, int *pfbfd)
{
	if (*pfbfd >= 0) /* If framebuffer file descriptor pointer is valid */
	{
		memset(map, color, FILESIZE); /* Set all colours in the map */
		return 1;
	}
	return 0;
}
/**
 * shGetAccel() - Get accelerometer sensors via i2c.
 *
 * @args1: Pointer to the raw value of X provided by the sensor.
 * @args2: Pointer to the raw value of Y provided by the sensor.
 * @args3: Pointer to the raw value of Z provided by the sensor.
 * 
 * Provided by the sensehat unchained library. Allocate an unsigned char
 * array as buffer for the data from the sensor. Read from address using
 * i2c read and check if received all chars. Format the data and set back
 * to output pointers.
 *
 * Return: Integer value if function called without errors.
 * 1 - No errors.
 * 0 - Error occured.
 */
int shGetAccel(int *Ax, int *Ay, int *Az)
{
    /* Temporary buffer of unsigned chars to store sensor values */
	unsigned char ucTemp[8]; 
    /* Value of received data */
	int rc;
    /**
     * Read from sensors via i2c by providing file descriptor and
     * address, buffer to store data in and how much to write in the buffer 
     */
	rc = i2cRead(file_acc, 0x28 + 0x80, ucTemp, 6);
    /* If received full 6 unsigned char and format the data */
	if (rc == 6)
	{
		int x, y, z;
		x = ucTemp[0] + (ucTemp[1] << 8);
		y = ucTemp[2] + (ucTemp[3] << 8);
		z = ucTemp[4] + (ucTemp[5] << 8);
		/* Fix the signed values */
		if (x > 32767)
			x -= 65536;
		if (y > 32767)
			y -= 65536;
		if (z > 32767)
			z -= 65536;
        /* Write back the values as output */
		*Ax = x;
		*Ay = y;
		*Az = z;
		return 1;
	}
	return 0;
}
/**
 * shGet2GAccel() - Convert the raw accelerometer data to sensible data with 2G sensitivity
 * 
 * @args1 - Pointer to a Vector3 of accelerometer array 
 *
 * Based on sensitivty set initially on setup, we need to format the raw data based on the
 * settings we have made. In the setup, we set to 2G sensitivity therefore we need to scale
 * the data by twoG_LSB. The return value will be in milli-Gs, we divide again to get in G
 * units. To match the open source fall data which was collected by python drivers, we
 * need to negate the values of x and y to match theirs.
 * 
 * Return: Integer value to indicate if function has been called without errors.
 * 1 - No errors
 * 0 - Error occured
 */
int shGet2GAccel(Vector3 * accelArr)
{
    /* Declare & initialise 3 variables to store x,y,z values */
	int x = accelArr->x;
	int y = accelArr->y;
	int z = accelArr->z;
    /* Attempt to get the raw accelerometer data */
	if (shGetAccel(&x, &y, &z))
	{
        /* Scale and convert from milli-Gs to Gs and negate to match python data */
		accelArr->x = -((double)(x * twoG_LSB) / 1000);
		accelArr->y = -((double)(y * twoG_LSB) / 1000);
		accelArr->z = (double)(z * twoG_LSB) / 1000;
		return 1;
	}
	return 0;
}
/**
 * shShutDown() - Properly close all file descriptors when not in used for
 * better resource management.
 *
 * @args1: Pointer to the framebuffer file descriptor
 * @args2: Pointer to the 8x8 sensehat map array
 *
 * Sanitisation function to close all opened file descriptors and release
 * files descriptors and resources back.
 */
void shShutdown(int *pfbfd, uint16_t *map)
{
    /**
     * Check if the files descriptors are not closed, then close.
     * Unmap the map address, and set all file descriptor to -1.
     */
	if (file_acc != -1)
		close(file_acc);
	if (*pfbfd != 1)
		close(*pfbfd);
	if (munmap(map, FILESIZE) == -1)
		perror("Error un-mapping the file");
	file_acc = -1;
}
/**
 * i2cRead() - Read from addresses of sensors via i2c
 * @args1: File descriptor
 * @args2: Address to read from
 * @args3: Buffer to write sensor data to
 * @args4: Number of unsigned chars (data) to write
 * 
 * Utilise I2C to read from sensors by first writing which address to read from
 * and then read from the address.
 * Written by sensehat unchained author.
 * 
 * Return: Integer value indicating if function called without errors
 */
static int i2cRead(int iHandle, unsigned char ucAddr, unsigned char *buf, int iLen)
{
    /* Variable to store if the i2c read and write is successful */
	int rc;
    /* 
     * Write the address to read by providing address to read  and file descriptor 
     * and how much to read
     */
	rc = write(iHandle, &ucAddr, 1);
    /* Debugging for joystick */
	if (ucAddr == 0xf2)
	{
#if defined(DEBUG)
		printf("joy stick i2c read: %d \n", rc);
#endif // DEBUG
	}
    /* If writing of address to read from is successful */
	if (rc == 1)
	{
        /* Attempt to read and store the data to buffer */
		rc = read(iHandle, buf, iLen);
#if defined(DEBUG)
		printf("i2c read: %d \n", rc);
		printf("i2c buf: %d \n", buf[0]);
		printf("i2c buf: %d \n", buf[1]);
#endif // DEBUG
	}
	return rc; /* If successful, should return 1. Else, unsuccessful */
}
/**
 * mapLEDFrameBuffer() - Map the entire framebuffer to given memory range
 * 
 * @args1 : Pointer to an array of uint16_t which is storing the LED map
 * @args2 : Pointer to framebuffer file descriptor
 * 
 * To manipulate the LED matrix, we need to map the framebuffer to the map.
 * 
 * Return - Integer value to indicate if mapping has succeed
 * 1 - Mapping succeeded
 * 0 - Error occured
 */
int mapLEDFrameBuffer(uint16_t **map, int *pfbfd)
{
    /* 
     * Map with the following settings, let kernel choose address to create
     * mapping, pages can be read and written, share the map. Enter the 
     * file descripter and 0 offset.
     */
	*map = mmap(NULL, FILESIZE, PROT_READ | PROT_WRITE, MAP_SHARED, *pfbfd, 0);
	/* If mapping fail, remember to close file descriptor. */
    if (map == MAP_FAILED)
	{
		close(*pfbfd);
		perror("Error mapping the file");
		return 0;
	}
	return 1;
}
/**
 * i2cWrite() - Write to address via i2c
 * 
 * @args1: File descriptor to write from
 * @args2: Address to write to
 * @args3: Buffer to store the data to write from
 * @args4: How much to write
 * 
 * Written by author of sensehat unchain
 * Return - Integer value to indicate function call successful
 * 1 - Succeed
 * 0 - Error occured 
 */
int i2cWrite(int iHandle, unsigned char ucAddr, unsigned char *buf, int iLen)
{
    /* Buffer to store what to write over */
	unsigned char ucTemp[512];
	/* Variable to store result of writing */
    int rc;
    /* Invalid parameters to write */
	if (iLen > 511 || iLen < 1 || buf == NULL)
		return -1; 

	ucTemp[0] = ucAddr; // send the register number first
#if defined(DEBUG)
	printf("Register number: %d %x\n", ucAddr, ucAddr);
#endif
	memcpy(&ucTemp[1], buf, iLen); // followed by the data
	rc = write(iHandle, ucTemp, iLen + 1);
	return rc - 1;
}
/*
 * Initialise joystick with input event buffer
 */
int initJoystick(int *fd)
{
    // Open joystick event buffer
	*fd = open(JOYSTICK_FILE, O_RDONLY);
	if (*fd == -1)
	{
		printf("Unable to open file! Errno: %d\n", errno);
		close(*fd);
		return -1;
	}
    // I/O control with event buffer
	int retVal = ioctl(*fd, EVIOCGNAME(sizeof(name)), name);
	if (retVal == -1)
	{
		printf("Unable to get event!!\n");
		close(*fd);
		return -1;
	}
	return 0;
}
/*
 * Get joystick information from input event buffer
 */
int readJoystick(int *fd, Joystick *joy)
{
    // Init code size
	int codeSize = -1;
	struct input_event ev[64];
	codeSize = read(*fd, ev, sizeof(struct input_event) * 64);

	if (codeSize < (int)sizeof(struct input_event))
	{
		fprintf(stderr, "expected %d bytes, got %d\n", (int)sizeof(struct input_event), codeSize);
		return 0;
	}

    // Get values frcom event buffer
	for (size_t i = 0; i < codeSize / sizeof(struct input_event); i++)
	{
		if (ev->type != EV_KEY)
			continue;
		checkJoystickState(ev->value, joy);
		checkJoystickDir(ev->code, joy);
		return 1;
	}

	return 0;
}

/*
 * Determine joystick direction
 */ 
void checkJoystickDir(int evCode, Joystick *joy)
{
	switch (evCode)
	{
	case UP:
		joy->dir = UP;
		joy->direction = "UP";
		break;
	case DOWN:
		joy->dir = DOWN;
		joy->direction = "DOWN";
		break;
	case LEFT:
		joy->dir = LEFT;
		joy->direction = "LEFT";
		break;
	case RIGHT:
		joy->dir = RIGHT;
		joy->direction = "RIGHT";
		break;
	case ENTER:
		joy->dir = ENTER;
		joy->direction = "ENTER";
		break;
	}
}

/*
 * Determine joystick state
 */
void checkJoystickState(int evType, Joystick *joy)
{
	switch (evType)
	{
	case RELEASE:
		joy->state = RELEASE;
		break;
	case PRESSED:
		joy->state = PRESSED;
		break;
	case HOLD:
		joy->state = HOLD;
		break;
	}
}

void readi2cCommand()
{
	char path[10];
	/* If you want to read output from command */
	FILE *fp = popen("sudo i2cget -f -y 1 0x46", "r");
	/* read output from command */
	while (fgets(path, 10, fp) != NULL)
		printf("%s", path);
}
