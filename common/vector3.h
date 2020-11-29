#if !defined(COMMON_VECTOR)
#define COMMON_VECTOR
// This file contains the function declaration for the
// Vector3 library.
//
// Used arithmetic operations for Vector3
/**
 * struct vector3 - Vector3 to store the x,y,z values
 * @x: x value
 * @y: y value
 * @z: z value
 *
 * Contains the 3 following x, y, z values for representation of
 * 3 axis data.
 */
typedef struct vector3
{
    double x;
    double y;
    double z;
} Vector3;

extern const Vector3 vector3_zero;

/**
 * vector3_multiply() - Scale the vectors
 * @vector: Pointer to read only vector
 *
 * Utilise macro to do the scaling of vectors
 * Return: New scaled vector
 */
Vector3 vector3_multiply(const Vector3 *vector, double scaler);

/**
 * vector3_divide() - Divide the vectors
 * @vector: Pointer to read only vector
 *
 * Utilise macro to do the division of vectors
 * Return: New divided vector
 */
Vector3 vector3_divide(const Vector3 *vector, double divider);

/**
 * vector3_magnitude() - Get the magnitude of given vector3
 * @vector: Pointer to read only vector
 *
 * Return: double value of vector's magnitude
 */
double vector3_magnitude(const Vector3 *vector);

/**
 * vector3_sqrMagnitude() - Get the squared magnitude of given vector3
 * @vector: Pointer to read only vector
 *
 * Utilise macro to do the magnitude calculation
 * Return: double value of vector's square magnitude
 */
double vector3_sqrMagnitude(const Vector3 *vector);

/**
 * vector3_normalise() - Get the normalised vector of given vector3
 * @vector: Pointer to read only vector
 *
 * If the provided vector's magnitude is very close to zero (smaller than kEpislion),
 * return as zero vector. Else, do division for normalised vector.
 *
 * Return: Normalised vector of provided vector
 */
Vector3 vector3_normalise(const Vector3 *vector);

/**
 * vector3_dot() - Get the dot product of two vectors
 * @a: Pointer to read only first vector
 * @b: Pointer to read only second vector
 *
 * Return: Dot product value of the two vectors.
 */
double vector3_dot(const Vector3 *a, const Vector3 *b);

/**
 * vector3_angle() - Find the angle between the two angles
 * @a: Pointer to read only first vector
 * @b: Pointer to read only second vector
 *
 * Return: The angle in degrees between the two vectors.
 */
double vector3_angle(const Vector3 *from, const Vector3 *to);

/**
 * vector3_distance() - Find the distance between two vectors
 * @a: Pointer to read only first vector
 * @b: Pointer to read only second vector
 *
 * Return: Double value of distance between two vectors
 */
double vector3_distance(const Vector3 *a, const Vector3 *b);

/**
 * vector3_print() - Printing the x, y, z values of the vector3
 * @vector: Pointer to read only vector
 */
void vector3_print(const Vector3 *vector);

#endif
