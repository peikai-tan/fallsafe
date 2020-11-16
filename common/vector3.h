#if !defined(COMMON_VECTOR)
#define COMMON_VECTOR

typedef struct vector3
{
    double x;
    double y;
    double z;
} Vector3;

extern const Vector3 vector3_zero;

Vector3 vector3_multiply(const Vector3 *vector, double scaler);
Vector3 vector3_divide(const Vector3 *vector, double divider);

double vector3_magnitude(const Vector3 *vector);
double vector3_sqrMagnitude(const Vector3 *vector);
Vector3 vector3_normalise(const Vector3 *vector);

double vector3_dot(const Vector3 *a, const Vector3 *b);
double vector3_angle(const Vector3 *from, const Vector3 *to);
double vector3_distance(const Vector3 *a, const Vector3 *b);

void vector3_print(const Vector3 *vector);

#endif
