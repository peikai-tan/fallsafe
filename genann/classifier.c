#include "classifier.h"

#define fallClassifier "dataFiles/fallClassifier.ann"
#define activityClassifier "dataFiles/activityClassifier.ann"

void classifier_new(Classifier c)
{
    c.fC = fopen(fallClassifier, "r");
    c.fClassifier = genann_read(c.fC);

    c.aC = fopen(activityClassifier, "r");
    c.aClassifier = genann_read(c.aC);

    fclose(c.fC);
    fclose(c.aC);
}

void classifier_destroy(Classifier c)
{
    genann_free(c.fClassifier);
    genann_free(c.aClassifier);
}

int prediction(const double *output)
{
    int maxIndex = 0;
    double biggest = 0;
    for (int i = 0; i < 5; i++)
        if (output[i] > biggest)
        {
            maxIndex = i;
            biggest = output[i];
        }
    return maxIndex;
}

int classifier_predict(Classifier c, double *sample)
{
    /**
    * Prediction:
    * 0 -> Fall
    * 1 -> Walk
    * 2 -> Run
    * 3 -> Station
    * 4 -> Jump
   */
    const double *fOutput = genann_run(c.fClassifier, sample);

    if (prediction(fOutput) == 1)
        return 0;

    const double *aOutput = genann_run(c.aClassifier, sample);

    return prediction(aOutput) + 1;
}
