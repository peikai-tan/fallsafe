#include "sensehat.h"
#include "stdio.h"
#include "unistd.h"

int main(int argc, char *argv[])
{

  int fbfd = 0;
  int joystickFB = 0;
  uint16_t * map = 0;
  uint16_t * mapHead = 0;
  //unsigned char isDown = 0;
  //unsigned char dir = 0;
  Joystick joystick;
  Vector3 accels = {0.0, 0.0 , 0.0};
  //float accelAngles[2] = {0.0f};
  //int gyroRates[3] = {0};
  // Uncomment for Complementary Angles
  //float complementaryAngles[2] = {0.0f};
  int startInt = mymillis();
  struct timeval tvBegin, tvEnd, tvDiff;
  gettimeofday(&tvBegin, NULL);
  unsigned char bufSize = 0;
  // Uncomment for Kalman
  //float XP[4] = {0.0f};
  //float YP[4] = {0.0f};
  //float kalmanAngles[2] = {0.0f};
  //float bias[2] = {0.0f}; 

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
    drawActivity(FALLING, mapHead, &fbfd);
    sleep(1);
    setMap(0x0000, map, &fbfd);
    sleep(1);
    //if(shGet2GAccel(&accels))
    //{
    //  vector3_print(&accels);  
    //}

    //if(shGetGyro(gyroRates) && shGet2GAccel(accelRadians))
    //{
    //    accelToAngle(accelAngles, accelRadians);
    //printf("Accel (rad) x = %f, y = %f, z = %f \n", accelRadians[0], accelRadians[1], accelRadians[2]);
    //if(shGet500DPSComplementary(complementaryAngles, gyroRates, accelAngles, &startInt))
    //		printf("Complementary Angles: x =  %f, y = %f \n", complementaryAngles[0], complementaryAngles[1]);
    //	if(shGet500DPSKalman(kalmanAngles, gyroRates, accelAngles, &startInt, bias, XP, YP))
    //			printf("Kalman Angles: x = %f, y = %f \n", kalmanAngles[0], kalmanAngles[1]);



    //}
    //    printf("Map value: %x\n", fbfd);

    //shSetPixel(3, 3, 0xF800, 1, mapHead, &fbfd);

    // readJoystick(&joystickFB, &joystick);
    // if(joystick.state == PRESSED)
    // {
    // printf("Joystick direction: %s\n", joystick.direction);
    // }

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
