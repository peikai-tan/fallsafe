#if !defined(CLASSIFIER)
#define CLASSIFIER

#include <stdio.h>
#include <stdlib.h>
#include "genann.h"

#include "../common/arraylist.h"
#include "../utils/timer/timer.h"

typedef struct classifier
{
    FILE *fC;
    genann *fClassifier;
    FILE *aC;
    genann *aClassifier;
} * Classifier;

const Classifier classifier;

Classifier predict();
// Classifier vector3_divide(const Vector3 const *vector, double divider);

#endif // COMMON_VECTOR3
