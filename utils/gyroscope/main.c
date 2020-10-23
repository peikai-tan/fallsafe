#include "sensehat.h"
#include "stdio.h"
#include "unistd.h"

int main(int argc, char *argv[])
{
	int fbfd = 0;
    int joystickFB = 0;
	uint16_t * map = 0;
	uint16_t * mapHead = 0;
//	unsigned char isDown = 0;
//  unsigned char dir = 0;
    Joystick joystick;
	if(shInit(1, &fbfd) == 0)
	{
		printf("Unable to open sense, is it connected?\n");
		return -1;
	}
	if(mapLEDFrameBuffer(&map, &fbfd) == 0)
	{
		printf("Unable to map LED to Frame Buffer. \n");
		return -1;
	}
  int val = -1;
  if((val = initJoystick(&joystickFB)) == -1)
  {
    printf("Unable to open joystick event\n");
    return -1;
  }
  else 
  {
    printf("val: %d\n", val);
  }

	mapHead = map;
	printf("map ptr: %p \n", map);
	while(1)
	{
//		isDown = shReadJoystick(&fbfd);
//		printf("Joystick vals: %02x\n", isDown);
//

//		if(shGetGyro(&x, &y, &z))
//		{
//			printf("Gyro: X = %d, Y= %d, Z = %d \n", x, y, z);
//		}
//    if(shGet2GAccel(&accel_x, &accel_y, &accel_z))
//    {
//      printf("Accel: X = %f, Y = %f, Z = %f \n", accel_x, accel_y, accel_z);
//    }

//    printf("Map value: %x\n", fbfd);

		//shSetPixel(3, 3, 0xF800, 1, mapHead, &fbfd);

    readJoystick(&joystickFB, &joystick);
    if(joystick.state == PRESSED)
    {
        printf("Joystick direction: %s\n", joystick.direction);
    }

    //printf("ev code: %d\n", dir);
//		sleep(1);
//		setMap(0xFFFF, map, &fbfd);
//		sleep(1);
//		setMap(0x0000, map, &fbfd);
//		sleep(1);
	}
	shShutdown(&fbfd, map);
	return 0;
}
