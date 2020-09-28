#include "sensehat.h"
#include "stdio.h"
#include "unistd.h"

int main(int argc, char *argv[])
{
	int x = 0, y = 0, z = 0;
	unsigned char isDown = 0;
	if(shInit(1) == 0)
	{
		printf("Unable to open sense, is it connected?\n");
		return -1;
	}
	while(isDown == 0)
	{
		x = y = z = 0;
		isDown = shReadJoystick();
		printf("Joystick = %02x \n", isDown);
		if(shGetGyro(&x, &y, &z))
		{
		printf("Gyro: X = %d, Y= %d, Z = %d \n", x, y, z);
		}
		usleep(100000);
	}
	shShutdown();
	return 0;
}
