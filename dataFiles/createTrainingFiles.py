import csv
import os
import numpy as np
import random

"""
    pollingFreq = polling rate to emulate
    duration = duration of second of data per prediction
"""
pollingFreq = 30
duration = 1

labels = ["walking", "running", "stationary"]

datasets = {
    "falling": list(),
    "running": list(),
    "stationary": list(),
    "walking": list()
}


def setup():
    os.chdir(os.getcwd() + "/dataFiles/")
    os.system("rm -rf trainingFiles")
    os.system("mkdir trainingFiles")


def combineIndividualActivities():
    print("Creating Input Files...")

    # Sets the length of trimming for the other datasets to the shortest one
    minBTWOthers = None

    for folder in datasets.keys():
        files = os.listdir(folder)
        for f in files:
            f = csv.reader(open(folder + "/" + f, "r"))
            next(f)

            for row in f:
                datasets[folder].append([float(dat) for dat in row])

        if folder != "falling":
            minBTWOthers = len(datasets[folder]) if minBTWOthers == None else min(
                minBTWOthers, len(datasets[folder]))
        else:
            lenFalling = len(datasets[folder])

    return minBTWOthers - pollingFreq


def quantiseTo(pollingFreq):
    print("Trimming...")

    # How many samples to skip depending on poling frequency
    rateToCut = int(60 / pollingFreq)

    for key in datasets.keys():
        quantisedDatasets = [[] for x in range(rateToCut)]
        dataset = datasets[key]

        for startingIndex in range(rateToCut):
            # Shift dataset to the right
            dataset = dataset[startingIndex:]

            for i in range(0, len(dataset), rateToCut):
                quantisedDatasets[startingIndex].append(
                    [float(x) for x in dataset[i]])

        datasets[key].clear()

        for tD in quantisedDatasets:
            datasets[key] += tD


def loadExternalFiles():
    # Add external data to datasets
    with open("externalData/scienceDirectFalls.csv", "r") as file:
        csvReader = csv.reader(file)
        for row in csvReader:
            datasets["falling"].append(
                [float(row[0]), float(row[1]), float(row[2])])

    return len(datasets["falling"])


def LST(lF, lO, duration, pollingFreq):
    """
    Label. shuffle and Trim
    """

    def addLabel(key, array):
        """
        Add the correct label to the data
        """
        return array + [1] if key == "falling" else array + [labels.index(key)]

    def appendRow(x, y, z, row):
        x.append(float(row[0]))
        y.append(float(row[1]))
        z.append(float(row[2]))

        return x, y, z

    for key in datasets.keys():
        dataset = []
        x = []
        y = []
        z = []

        end = lF if key == "falling" else lO

        # Initialise iterator
        i = 0

        # Fill up axes until correct length
        while i < duration * pollingFreq:
            x, y, z = appendRow(x, y, z, datasets[key][i])
            i += 1

        # Add the first sample
        dataset.append(addLabel(key, x+y+z))

        # Continue until the end of the dataset
        while i < len(datasets[key]):
            x, y, z = appendRow(x, y, z, datasets[key][i])

            x.pop(0)
            y.pop(0)
            z.pop(0)

            dataset.append(addLabel(key, x+y+z))

            i += 1

        # Shuffle
        for x in range(5):
            random.shuffle(dataset)

        # Trim
        datasets[key] = dataset[:end]


def generateTrainingFilesOf(lF):
    """
    lF = Length to trim Falling to\n
    lO = Length to trim everything else to
    """
    print("Generating...")

    fallClassifier = datasets["falling"]
    activityClassifier = []

    for label in labels:
        activityClassifier += datasets[label]

    # Shuffling activityClassifier Data for training
    for x in range(5):
        random.shuffle(activityClassifier)

    # Adding same length of other activities into fallClassifer data
    for i in range(lF):
        fallClassifier.append(activityClassifier[i][:-1] + [0])

    # Shuffling fallClassifier Data for training
    for x in range(5):
        random.shuffle(fallClassifier)

    # Formatting and saving data in 10 decimal places
    np.savetxt("trainingFiles/activityClassifier.csv",
               activityClassifier, delimiter=",", fmt="%02.10f")
    np.savetxt("trainingFiles/fallClassifier.csv",
               fallClassifier, delimiter=",", fmt="%02.10f")


if __name__ == "__main__":
    setup()
    lO = combineIndividualActivities()
    quantiseTo(pollingFreq)
    lF = loadExternalFiles()
    LST(lF, lO, duration, pollingFreq)
    generateTrainingFilesOf(lF)
    print("Row Size: {0}".format(int(np.ceil(pollingFreq * duration)) * 3))
