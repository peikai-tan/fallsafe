#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include "../common/vector3.h"

int main(void)
{
    vector3_print(&vector3_zero);

    puts("Pseudo Random values");
    Vector3 vector1 = {rand(), rand(), rand()};
    Vector3 vector2 = {rand(), rand(), rand()};
    vector3_print(&vector1);
    vector3_print(&vector2);

    Vector3 normalised1 = vector3_normalise(&vector1);
    Vector3 normalised2 = vector3_normalise(&vector2);
    puts("Normalised: ");
    vector3_print(&normalised1);
    vector3_print(&normalised2);

    puts("Distance: ");
    double distance = vector3_distance(&vector1, &vector2);
    printf("%lf\n", distance);

    puts("Angle: ");
    double angle = vector3_angle(&vector1, &vector2);
    printf("%lf\n", angle);

    puts("Program finished");
    return 0;
}