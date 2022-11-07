from dataclasses import dataclass
import time
import csvReader
import components
import simulator
import pandas as pd
import sys
import os


def execute(fname):

    # Read in the data
    dr = '../data/' + fname
    data = csvReader.readCSV(dr)

    # Create a ship with default settings
    s = components.ship(**components.initialship)
    # s.display() # Outputs ship characteristics

    # Start the simulator
    output = simulator.simulate(s, data)
    output['results'].display()

    return output

# This loops through every .csv in the folder you choose
yourpath = sys.argv[1]
allResults = []
counter = 0
for root, dirs, files in os.walk(yourpath, topdown=False):
    for name in files:
        if not name[0] == ".": # Prevents selecting hidden files
            fname = os.path.join(root, name)
            out = execute(fname)
            simulator.writeRecord(out['record'], fname)
            allResults.append(out['results'])
            counter += 1


allResults = pd.DataFrame(allResults)
print(allResults.describe())
