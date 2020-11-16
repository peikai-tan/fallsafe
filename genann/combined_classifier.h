#if !defined(CLASSIFIER)
#define CLASSIFIER

#include <stdio.h>
#include <stdlib.h>
#include "genann.h"

typedef struct classifier
{
    FILE *cC;
    genann *cClassifier;
} * Classifier;

Classifier classifier_new();
void classifier_destroy(Classifier);
int classifier_predict(Classifier, double *);
void classifier_reinforce(Classifier, double *, int);

#endif // CLASSIFIER
