#include "sensehat.h"
#include "stdio.h"
#include "unistd.h"

int main(int argc, char *argv[])
{
	int fbfd = 0;
	uint16_t * map = 0;
	uint16_t * mapHead = 0;
	int x = 0, y = 0, z = 0;
  	float accel_x = 0, accel_y = 0, accel_z = 0;
	float angleX = 0.0f, angleY = 0.0f;
	int startInt = mymillis();
	struct timeval tvBegin, tvEnd, tvDiff;
	gettimeofday(&tvBegin, NULL);


	//unsigned char isDown = 0;
	if(shInit(1, &fbfd) == 0)
	{
		printf("Unable to open sense, is it connected?\n");
		return -1;
	}
	//if(mapLEDFrameBuffer(&map, &fbfd) == 0)
	//{
	//	printf("Unable to map LED to Frame Buffer. \n");
	//	return -1;
	//}
	//mapHead = map;
	//printf("map ptr: %p \n", map);
	while(1)
	{
		//isDown = shReadJoystick(&fbfd);
		//printf("Joystick vals: %02x\n", isDown);
		if(shGet500DPSGyro(&accel_x, &accel_y, &accel_z, &startInt))
		{
			printf("Gyro: X = %f, Y= %f, Z = %f \n", accel_x, accel_y, accel_z);
		}
    		//if(shGet2GAccel(&accel_x, &accel_y, &accel_z))
    		//{
      		//	printf("Accel: X = %f, Y = %f, Z = %f \n", accel_x, accel_y, accel_z);
		//	accelToAngle(&angleX, &angleY, &accel_x, &accel_y, &accel_z);
		//	printf("Angle: X = %f, Y + %f \n", angleX, angleY);
    		//}

		//shSetPixel(3, 3, 0xF800, 1, mapHead, &fbfd);
		//sleep(1);
		//setMap(0xFFFF, map, &fbfd);
		//sleep(1);
		//setMap(0x0000, map, &fbfd);
		//sleep(1);
		sleep(1);
	}
	shShutdown(&fbfd, map);
	return 0;
}
