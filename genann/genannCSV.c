#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include "genann.h"

#define dataSize 90

#define DEBUG 1
#define PRINT 0

#include "../common/arraylist.h"

ArrayList getValues(char buf[])
{
    ArrayList row = arraylist_new(double, dataSize);
    double elem;
    while (sscanf(buf, "%le,", &elem) != EOF)
    {
        arraylist_push(row, &elem);
        if (elem < 0)
            buf += 14;
        else
            buf += 13;
    }
    return row;
}

void getOutput(double **label, double i)
{
    double r[2] = {0.0, 0.0};
    r[(int)i] = 1;

    *label = r;
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

int main(void)
{
    // srand(time(0)); //1603803954 0.899972
    srand(1604396818);
    // Highest for fallClassifier = 1604397616 0.790514

    int bufferSize = dataSize * 15 + 12;

    genann *ann = genann_init(dataSize, 5, 100, 2);

    char buf[bufferSize]; // Slightly oversized just in case.

    double *label;
    double value;

    int runs = 0;
    float correctCount = 0.0f;

    FILE *ptr = fopen("dataFiles/trainingFiles/fallClassifier.csv", "r");
    for (; fgets(buf, bufferSize, ptr) != NULL; runs++)
    {
        ArrayList row = getValues(buf);
        arraylist_pop(row, &value);
        getOutput(&label, value);
        genann_train(ann, row->_array, label, 0.1);

        const double *output = genann_run(ann, row->_array);

        // printf("Prediction: %d\n", prediction(output));
        // printf("Actual: %lf\n", value);

        if (prediction(output) == (int)value)
            correctCount++;
            // else
            // {
            //     printf("Run: %d, %d, %d\n", runs, prediction(output), (int)value);
            //     exit(0);
            // }

#if PRINT
        printf("%lf\n", value);
        printf("%lf, %lf\n", genann_run(ann, row->_array)[0], genann_run(ann, row->_array)[1]);
#endif
        free(row);
    }

#if DEBUG
    printf("Accuracy: %lf%\nRuns: %d\n\n", correctCount / runs, runs);
#endif

    genann_free(ann);
    return 0;
}