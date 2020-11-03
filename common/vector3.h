#if !defined(COMMON_VECTOR)
#define COMMON_VECTOR

typedef struct vector3
{
    double x;
    double y;
    double z;
} Vector3;

extern const Vector3 vector3_zero;

Vector3 vector3_multiply(const Vector3 const *vector, double scaler);
Vector3 vector3_divide(const Vector3 const *vector, double divider);

double vector3_magnitude(const Vector3 const *vector);
double vector3_sqrMagnitude(const Vector3 const *vector);
Vector3 vector3_normalise(const Vector3 const *vector);

double vector3_dot(const Vector3 const *a, const Vector3 const *b);
double vector3_angle(const Vector3 const *from, const Vector3 const *to);
double vector3_distance(const Vector3 const *a, const Vector3 const *b);

void vector3_print(const Vector3 const *vector);

#endif
