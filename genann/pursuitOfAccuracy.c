#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include "genann.h"

#define GETLENGTH 0

#define dataSize 90
#define hiddenLayers 2
#define nPerLayer 20
#define outputSize 2
#define file "dataFiles/trainingFiles/fallClassifier.csv"
// #define file "dataFiles/trainingFiles/activityClassifier.csv"

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
    double r[outputSize] = {0.0, 0.0};
    r[(int)i] = 1;
    *label = r;
}

int prediction(const double *output)
{
    int maxIndex = 0;
    double biggest = 0;
    for (int i = 0; i < outputSize; i++)
        if (output[i] > biggest)
        {
            maxIndex = i;
            biggest = output[i];
        }

    return maxIndex;
}

int main(void)
{
    float accuracy = 0;
    long bestTime;
    float bestAccuracy = 0;
    int i = 0;

    int bufferSize = dataSize * 15 + 12; //Length of data * (possible sign, 2 integer place, decimal point, 10 decimal place, comma) + (integer place, decimal point, 10 decimal place)

    int lengthOfCSV;
    int runningLength;

#if GETLENGTH
    char buf[bufferSize];
    FILE *ptr = fopen(file, "r");
    while (fgets(buf, bufferSize, ptr) != NULL)
    {
        lengthOfCSV++;
    }

    printf("%d, %f\n", lengthOfCSV, lengthOfCSV * 0.75);
    exit(0);
#endif

    if (file == "dataFiles/trainingFiles/fallClassifier.csv")
    {
        lengthOfCSV = 15430;
        runningLength = 11572;
    }
    else
    {
        lengthOfCSV = 27564;
        runningLength = 20673;
    }

    long t = time(0);
    for (; accuracy < 0.9; i++)
    {
        srand(t);

        // Falling
        // 1604904777 0.637118 (2, 20)

        // Activity (updated)
        //1604904318 0.755913 (2, 20)

        genann *ann = genann_init(dataSize, hiddenLayers, nPerLayer, outputSize);

        char buf[bufferSize]; // Slightly oversized just in case.

        double value;

        int runs = 0;
        float correctCount = 0.0f;

        FILE *ptr = fopen(file, "r");

        // Train
        for (; runs < runningLength; runs++) // 3634 = fall  7269 = activity
        {
            double *label;
            fgets(buf, bufferSize, ptr) != NULL;
            ArrayList row = getValues(buf);
            arraylist_pop(row, &value);
            getOutput(&label, value);
            genann_train(ann, row->_array, label, 1);

            arraylist_destroy(row);
        }

        runs = 0;

        // Test Accuracy
        for (; fgets(buf, bufferSize, ptr) != NULL; runs++)
        {
            ArrayList row = getValues(buf);
            arraylist_pop(row, &value);

            const double *output = genann_run(ann, row->_array);

            if (prediction(output) == (int)value)
                correctCount++;

            arraylist_destroy(row);
        }

        accuracy = correctCount / runs;

        if (accuracy > bestAccuracy)
        {
            bestAccuracy = accuracy;
            bestTime = t;

            // Saving Model
            if (file == "dataFiles/trainingFiles/activityClassifier.csv") {
                FILE *saved = fopen("dataFiles/activityClassifier.ann", "w");
                genann_write(ann, saved);
                fclose(saved);
            } else {
                FILE *saved = fopen("dataFiles/fallClassifier.ann", "w");
                genann_write(ann, saved);
                fclose(saved);
            }
        }

        system("clear");
        printf("File: %s\n", file);
        printf("Hidden Layers: \t\t%d\n", hiddenLayers);
        printf("Neurons Per Layer: \t%d\n\n", nPerLayer);
        printf("Best:\t\t%ld\n", bestTime);
        printf("\t\t%lf\n\n", bestAccuracy);
        printf("Runs:\t\t%d\n", i + 1);
        printf("Current:\t%ld\n", t);

        genann_free(ann);
        fclose(ptr);
        t++;
    }
    return 0;
}