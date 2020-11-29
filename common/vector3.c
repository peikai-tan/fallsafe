// This file contains the function definition for the
// Vector3 library.
//
// Used arithmetic operations for Vector3
#include <stdio.h>
#include <math.h>
#include "./math.h"
#include "./vector3.h"
/**
 * Global variable for commonly used epsilon values
 * and zero vector
 */
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

/**
 * vector3_multiply() - Scale the vectors
 * @vector: Pointer to read only vector
 *
 * Utilise macro to do the scaling of vectors
 * Return: New scaled vector
 */
Vector3 vector3_multiply(const Vector3 *vector, double scaler)
{
    scaling(vector, scaler, *);
}

/**
 * vector3_divide() - Divide the vectors
 * @vector: Pointer to read only vector
 *
 * Utilise macro to do the division of vectors
 * Return: New divided vector
 */
Vector3 vector3_divide(const Vector3 *vector, double divider)
{
    scaling(vector, divider, /);
}

/**
 * vector3_sqrMagnitude() - Get the squared magnitude of given vector3
 * @vector: Pointer to read only vector
 *
 * Utilise macro to do the magnitude calculation
 * Return: double value of vector's square magnitude
 */
double vector3_sqrMagnitude(const Vector3 *vector)
{
    return squareMagnitude(vector);
}

/**
 * vector3_magnitude() - Get the magnitude of given vector3
 * @vector: Pointer to read only vector
 *
 * Return: double value of vector's magnitude
 */
double vector3_magnitude(const Vector3 *vector)
{
    return sqrt(squareMagnitude(vector));
}

/**
 * vector3_normalise() - Get the normalised vector of given vector3
 * @vector: Pointer to read only vector
 *
 * If the provided vector's magnitude is very close to zero (smaller than kEpislion),
 * return as zero vector. Else, do division for normalised vector.
 *
 * Return: Normalised vector of provided vector
 */
Vector3 vector3_normalise(const Vector3 *vector)
{
    double magnitude = vector3_magnitude(vector);
    return magnitude > kEpsilon ? vector3_divide(vector, magnitude) : vector3_zero;
}

/**
 * vector3_dot() - Get the dot product of two vectors
 * @a: Pointer to read only first vector
 * @b: Pointer to read only second vector
 *
 * Return: Dot product value of the two vectors.
 */
double vector3_dot(const Vector3 *a, const Vector3 *b)
{
    return a->x * b->x + a->y * b->y + a->z * b->z;
}

/**
 * vector3_angle() - Find the angle between the two angles
 * @a: Pointer to read only first vector
 * @b: Pointer to read only second vector
 *
 * Return: The angle in degrees between the two vectors.
 */
double vector3_angle(const Vector3 *from, const Vector3 *to)
{
    double denominator = sqrt(vector3_sqrMagnitude(from) * vector3_sqrMagnitude(to));
    if (denominator < kEpsilonNormalSqrt)
    {
        return 0;
    }
    double dot = vector3_dot(from, to) / denominator;
    dot = clamp(dot, -1, 1);
    return acos(dot) * 180.0 / 3.14159265358979323846;
}

/**
 * vector3_distance() - Find the distance between two vectors
 * @a: Pointer to read only first vector
 * @b: Pointer to read only second vector
 *
 * Return: Double value of distance between two vectors
 */
double vector3_distance(const Vector3 *a, const Vector3 *b)
{
    double x = a->x - b->x;
    double y = a->y - b->y;
    double z = a->z - b->z;
    return sqrt(x * x + y * y + z * z);
}

/**
 * vector3_print() - Printing the x, y, z values of the vector3
 * @vector: Pointer to read only vector
 */
void vector3_print(const Vector3 *vector)
{
    printf("{ x: %lf, y: %lf, z: %lf }\n", vector->x, vector->y, vector->z);
}
