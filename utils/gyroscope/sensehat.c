
//
// Sense Hat Unchained
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
#include "sensehat.h"

static int file_acc = -1; // accelerometer/gyro
static int file_mag = -1; // magnetometer
//static int file_joystick = -1;
static int i2cRead(int iHandle, unsigned char ucAddr, unsigned char *buf, int iLen);
static int i2cWrite(int iHandle, unsigned char ucAddr, unsigned char *buf, int iLen);
//
// Opens file system handles to the I2C devices
//
int shInit(int iChannel, int * pfbfd)
{
unsigned char ucTemp[32];
char filename[32];
struct fb_fix_screeninfo fix_info;

	sprintf(filename, "/dev/i2c-%d", iChannel);
        
	*pfbfd = open(FILEPATH, O_RDWR);
	if(*pfbfd == -1)
	{
		perror("Error (call to 'open')");
		goto badexit;
	}

	if(ioctl(*pfbfd, FBIOGET_FSCREENINFO, &fix_info) == -1)
	{
		perror("Error (call to 'ioctl')");
		close(*pfbfd);
		goto badexit;
	}

	if(strcmp(fix_info.id, "RPi-Sense FB") != 0)
	{
		printf("%s \n", "Error: RPi-Sense FB not found");
		close(*pfbfd);
		goto badexit;
	}

	file_acc = open(filename, O_RDWR);
	if (ioctl(file_acc, I2C_SLAVE, 0x6a) < 0)
	{
		fprintf(stderr, "Failed to acquire bus for accelerometer\n");
		goto badexit;
	}
	file_mag = open(filename, O_RDWR);
	if (ioctl(file_mag, I2C_SLAVE, 0x1c) < 0)
	{
		fprintf(stderr, "Failed to acquire bus for magnetometer\n");
		goto badexit;
	}

	//file_joystick = open(filename, O_RDWR);
	//if(file_joystick < 0)
	//{
	//	fprintf(stderr, "Failed to open joystick bus \n");
	//	goto badexit;
	//}

	//if(ioctl(file_joystick, I2C_SLAVE, 0xf2) < 0)
	//{
	//	fprintf(stderr, "failed to acquire bus for joystick \n");
	//	goto badexit;
	//}

	// Init magnetometer
	ucTemp[0] = 0x48; // output data rate/power mode
	ucTemp[1] = 0x00; // default scale
	ucTemp[2] = 0x00; // continuous conversion
	ucTemp[3] = 0x08; // high performance mode
	i2cWrite(file_mag, 0x20+0x80, ucTemp, 4);

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
	if (file_mag != -1)
	{
		close(file_mag);
		file_mag = -1;
	}

	//if(file_joystick != -1)
	//{
	//	close(file_joystick);
	//	file_joystick = -1;
	//}
	return 0;
} /* shInit() */

//
// Set a single pixel on the 8x8 LED Array
//
int shSetPixel(int x, int y, uint16_t color, int bUpdate, uint16_t * map_headptr, int * pfbfd)
{
int i = 0;

	if (x >= 0 && x < 8 && y >= 0 && y < 8 && *pfbfd >= 0)
	{
		printf("hello\n");
		i = (y*8)+x; // offset into array
		if (bUpdate)
			map_headptr[i] = color;
		return 1;
	}
	return 0;
} /* shSetPixel() */

int setMap(uint16_t color,  uint16_t * map, int * pfbfd)
{
	if(*pfbfd >= 0)
	{
		memset(map, color, FILESIZE);
		return 1;
	}
	return 0;
}

int shGetAccel(int *Ax, int *Ay, int *Az)
{
unsigned char ucTemp[8];
int rc;

	rc = i2cRead(file_acc, 0x28+0x80, ucTemp, 6);
	if (rc == 6)
	{
		int x, y, z;

		x = ucTemp[0] + (ucTemp[1] << 8);
		y = ucTemp[2] + (ucTemp[3] << 8);
		z = ucTemp[4] + (ucTemp[5] << 8);
		// fix the signed values
		if (x > 32767) x -= 65536;
		if (y > 32767) y -= 65536;
		if (z > 32767) z -= 65536;
		*Ax = x; *Ay = y; *Az = z;
		return 1;
	}
	return 0;
} /* shGetAccel() */

int shGet2GAccel(float *Ax, float *Ay, float *Az)
{
  int x = *Ax;
  int y = *Ay;
  int z = *Az;

  if(shGetAccel(&x, &y, &z))
  {
    *Ax = (x * twoG_LSB) / 1000;
    *Ay = (y * twoG_LSB) / 1000;
    *Az = (z * twoG_LSB) / 1000;
    
    return 1;
  }
  
  return 0;
} /* shGet2GAccel() */

int shGetGyro(int *Gx, int *Gy, int *Gz)
{
unsigned char ucTemp[8];
int rc;

	rc = i2cRead(file_acc, 0x18+0x80, ucTemp, 6);
	if (rc == 6)
	{
		*Gx = ucTemp[0] + (ucTemp[1] << 8);
		*Gy = ucTemp[2] + (ucTemp[3] << 8);
		*Gz = ucTemp[4] + (ucTemp[5] << 8);
		return 1;
	}
	return 0;
} /* shGetGyro() */

int shGetMagneto(int *Mx, int *My, int *Mz)
{
unsigned char ucTemp[8];
int rc;

	rc = i2cRead(file_mag, 0x28+0x80, ucTemp, 6);
	if (rc == 6)
	{
		int x, y, z;
		x = ucTemp[0] + (ucTemp[1] << 8);
		y = ucTemp[2] + (ucTemp[3] << 8);
		z = ucTemp[4] + (ucTemp[5] << 8);
		// fix signed values
		if (x > 32767) x -= 65536;
		if (y > 32767) y -= 65536;
		if (z > 32767) z -= 65536;
		*Mx = z; *My = y; *Mz = z;
		return 1;
	}
	return 0;
} /* shGetMagneto() */

void shShutdown(int * pfbfd, uint16_t * map)
{
	if (file_acc != -1) close(file_acc);
	if (file_mag != -1) close(file_mag);
	if (*pfbfd != 1) close (*pfbfd);

	if(munmap(map, FILESIZE) == -1)
		perror("Error un-mapping the file");
	file_acc = file_mag = -1;
} /* shShutdown() */

static int i2cRead(int iHandle, unsigned char ucAddr, unsigned char *buf, int iLen)
{
int rc;

	rc = write(iHandle, &ucAddr, 1);
	if(ucAddr == 0xf2)
		printf("joy stick i2c read: %d \n", rc);
	if (rc == 1)
	{
		rc = read(iHandle, buf, iLen);
	}
	return rc;
} /* i2cRead() */

int mapLEDFrameBuffer(uint16_t ** map, int * pfbfd)
{
	*map = mmap(NULL, FILESIZE, PROT_READ | PROT_WRITE, MAP_SHARED, *pfbfd, 0);
	if(map == MAP_FAILED)
	{
		close(*pfbfd);
		perror("Error mapping the file");
		return 0;
	}
	return 1;
}

int i2cWrite(int iHandle, unsigned char ucAddr, unsigned char *buf, int iLen)
{
unsigned char ucTemp[512];
int rc;

	if (iLen > 511 || iLen < 1 || buf == NULL)
		return -1; // invalid write

	ucTemp[0] = ucAddr; // send the register number first 
	memcpy(&ucTemp[1], buf, iLen); // followed by the data
	rc = write(iHandle, ucTemp, iLen+1);
	return rc-1;

} /* i2cWrite() */

//unsigned char shReadJoystick(int * pfbfd)
//{
//	unsigned char ucBuf[2];
//        int rc = 0;
//	if(*pfbfd != -1)
//	{
//		rc = i2cRead(*pfbfd, 0xf2, ucBuf, 1);
//		if(rc == 1)
//			return ucBuf[0];
//	}
//	return 0;
//}
