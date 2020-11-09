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

labels = ["walking", "running", "stationary", "jumping"]


datasets = {
    "falling": list(),
    "jumping": list(),
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

    # Sets the length of trimming for the other datasets according to the length of falling
    lenFalling = None
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

    return lenFalling, minBTWOthers


def trimTo(lF, lO, pollingFreq):
    """
    lF = Length to trim Falling to\n
    lO = Length to trim everything else to
    """
    print("Trimming...")

    # How many samples to skip depending on poling frequency
    rateToCut = int(60 / pollingFreq)

    for key in datasets.keys():
        trimmedDatasets = [[] for x in range(rateToCut)]
        dataset = datasets[key]

        end = lF if key == "falling" else lO

        for startingIndex in range(rateToCut):
            # Shift dataset to the right
            dataset = dataset[startingIndex:]

            for i in range(0, (end - 1), rateToCut):
                trimmedDatasets[startingIndex].append(
                    [float(x) for x in dataset[i]])

        datasets[key].clear()

        for tD in trimmedDatasets:
            datasets[key] += tD


def generateTrainingFilesOf(duration, pollingFreq):
    print("Generating...")

    fallClassifier = []
    activityClassifier = []

    def addToDataset(key, array):
        """
        Seperates into the 2 different datasets.
        """
        if key == "falling":
            fallClassifier.append(x + y + z + [1])
        else:
            activityClassifier.append(x + y + z + [labels.index(key)])

    for key in datasets.keys():
        x = []
        y = []
        z = []

        # Initialise iterator
        i = 0

        # Fill up axes until correct length
        while i < duration * pollingFreq:
            row = datasets[key][i]

            x.append(float(row[0]))
            y.append(float(row[1]))
            z.append(float(row[2]))

            i += 1

        # Add the first sample
        addToDataset(key, [x+y+z])

        # Continue until the end of the dataset
        while i < len(datasets[key]):
            row = datasets[key][i]

            x.append(float(row[0]))
            y.append(float(row[1]))
            z.append(float(row[2]))

            x.pop(0)
            y.pop(0)
            z.pop(0)

            addToDataset(key, [x+y+z])

            i += 1

    # Shuffling activityClassifier Data for training
    for x in range(5):
        random.shuffle(activityClassifier)

    # Adding same length of other activities into fallClassifer data
    lengthOfFallClassifier = len(fallClassifier)
    for i in range(lengthOfFallClassifier):
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
    lF, lO = combineIndividualActivities()
    trimTo(lF, lO, pollingFreq)
    generateTrainingFilesOf(duration, pollingFreq)
    print("Row Size: {0}".format(int(np.ceil(pollingFreq * duration)) * 3))
