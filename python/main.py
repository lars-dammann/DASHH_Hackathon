from dataclasses import dataclass
import time
import csvReader
import components
import simulator
import sys
import os

def execute(fname):
    dr = '../data/' + fname
    data = csvReader.readCSV(dr)
    s = components.ship(**components.initialship)
    s.display()
    output = simulator.simulate(s, data)
    output['results'].display()
    simulator.writeRecord(output['record'], fname)

if len(sys.argv) == 1:
    yourpath = '../data'

    for root, dirs, files in os.walk(yourpath, topdown=False):
        for name in files:
            if not name[0] == ".":
                fname = os.path.join(root, name)
                execute(fname)


else:
    fname = sys.argv[1]
    execute(fname)
