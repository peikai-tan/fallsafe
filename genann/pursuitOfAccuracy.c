#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include "genann.h"

#define dataSize 90

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

int main(void)
{
    float accuracy = 0;
    long bestTime;
    float bestAccuracy = 0;
    int i = 0;

    for (; accuracy < 0.9; i++)
    {
        long t = time(0);
        srand(t); //1603803954 0.899972

        genann *ann = genann_init(dataSize, 5, 50, 2);

        char buf[1300]; // Slightly oversized just in case.

        double *label;
        double value;
        double *copy;

        int runs = 0;
        float correctCount = 0.0f;

        FILE *ptr = fopen("dataFiles/trainingFiles/combined.csv", "r");
        for (; fgets(buf, 1300, ptr) != NULL; runs++)
        {
            ArrayList row = getValues(buf);
            arraylist_pop(row, &value);
            getOutput(&label, value);
            genann_train(ann, row->_array, label, 0.1);

            const double *output = genann_run(ann, row->_array);

            if (output[(int)value] > output[((int)value + 1) % 2])
                correctCount++;

            // printf("%lf\n", value);
            // printf("%lf, %lf\n", genann_run(ann, row->_array)[0], genann_run(ann, row->_array)[1]);
            free(row);
        }
        accuracy = correctCount / runs;

        if (accuracy > bestAccuracy)
        {
            bestAccuracy = accuracy;
            bestTime = t;
        }

        printf("Best:\t\t%ld\n", bestTime);
        printf("\t\t%lf\n", bestAccuracy);
        printf("Runs:\t\t%d\n", i + 1);

        genann_free(ann);
    }
    return 0;
}