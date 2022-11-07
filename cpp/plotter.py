import numpy as np
import pandas as pd
import matplotlib.pyplot as plt

data = []
with open('temp.csv') as f:
    for line in f.readlines():
        line = line.rstrip("\n")
        data.append(int(line))

plt.plot(data)
plt.show()
