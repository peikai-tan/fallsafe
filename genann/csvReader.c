#include <stdio.h>
#include <stdlib.h>

#include "../common/arraylist.h"

ArrayList getValues(char buf[])
{
    ArrayList row = arraylist_new(double, 90);
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

void read(double ***data, double **labels)
{
    ArrayList trainingData = arraylist_new(double *, 0);
    ArrayList trainingLabels = arraylist_new(double, 0);

    char buf[1300]; // Slightly oversized just in case.
    int i = 0;

    double label;

    FILE *ptr = fopen("dataFiles/trainingFiles/combined.csv", "r");
    for (; fgets(buf, 1300, ptr) != NULL; i++)
    {
        ArrayList row = getValues(buf);
        arraylist_pop(row, &label);
        arraylist_push(trainingLabels, &label);
        arraylist_push(trainingData, &row->_array);
        free(row);
    }

    *data = (double **)trainingData->_array;
    *labels = (double *)trainingLabels->_array;
}

int main(void)
{
    double **data;
    double *labels;

    read(&data, &labels);

    printf("%lf\n", data[0][0]);

    free(data);
    free(labels);
}
