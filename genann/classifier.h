#if !defined(CLASSIFIER)
#define CLASSIFIER

#include <stdio.h>
#include <stdlib.h>
#include "genann.h"

typedef struct classifier
{
    FILE *fC;
    genann *fClassifier;
    FILE *aC;
    genann *aClassifier;
} * Classifier;

Classifier classifier_new();
void classifier_destroy(Classifier);
int classifier_predict(Classifier, double *);

#endif // CLASSIFIER
