#include <stdio.h>
#include <math.h>
#include "vector3.h"

static const double kEpsilon = 0.00001;
static const double kEpsilonNormalSqrt = 1e-15;

const Vector3 vector3_zero = {0, 0, 0};

#define scaling(vector, number, operation) \
    Vector3 output = *vector;              \
    output.x = output.x operation number;  \
    output.y = output.y operation number;  \
    output.z = output.y operation number;  \
    return output;

#define squareMagnitude(vector) \
    (vector->x * vector->x + vector->y * vector->y + vector->z * vector->z)

Vector3 vector3_multiply(const Vector3 const *vector, double scaler)
{
    scaling(vector, scaler, *);
}

Vector3 vector3_divide(const Vector3 const *vector, double divider)
{
    scaling(vector, divider, /);
}

double vector3_sqrMagnitude(const Vector3 const *vector)
{
    return squareMagnitude(vector);
}

double vector3_magnitude(const Vector3 const *vector)
{
    return sqrt(squareMagnitude(vector));
}

Vector3 vector3_normalise(const Vector3 const *vector)
{
    double magnitude = vector3_magnitude(vector);
    return magnitude > kEpsilon ? vector3_divide(vector, magnitude) : vector3_zero;
}

double vector3_dot(const Vector3 const *a, const Vector3 const *b)
{
    return a->x * b->x + a->y * b->y + a->z * b->z;
}

double vector3_angle(const Vector3 const *from, const Vector3 const *to)
{
    double denominator = sqrt(vector3_sqrMagnitude(from) * vector3_sqrMagnitude(to));
    if (denominator < kEpsilonNormalSqrt)
    {
        return 0;
    }
    double dot = vector3_dot(from, to) / denominator;
    dot = dot < -1 ? -1 : dot > 1 ? 1 : dot;
    return acos(dot) * 180.0 / 3.14159265358979323846;
}

double vector3_distance(const Vector3 const *a, const Vector3 const *b)
{
    double x = a->x - b->x;
    double y = a->y - b->y;
    double z = a->z - b->z;
    return sqrt(x * x + y * y + z * z);
}

void vector3_print(const Vector3 const *vector)
{
    printf("{ x: %lf, y: %lf, z: %lf }\n", vector->x, vector->y, vector->z);
}