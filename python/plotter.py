import numpy as np
import matplotlib.pyplot as plt
import sys
import pandas as pd
from matplotlib.widgets import Slider

fname = sys.argv[1]


data = pd.read_csv(fname, sep=',')

# Setting Plot and Axis variables as subplots()
# function returns tuple(fig, ax)
Plot, Axis = plt.subplots()

# Adjust the bottom size according to the
# requirement of the user
plt.subplots_adjust(bottom=0.25)

# Set the x and y axis to some dummy data
print(data)
if len(sys.argv) == 2:
    plt_lst = ['load', 'batPwr', 'engPwr', 'powerDeficit', 'charge']
else:
    plt_lst = sys.argv[2:len(sys.argv)]

time = data['time'].to_numpy()
if 'load' in plt_lst:
    load = data['load'].to_numpy()
    load = load/load.max()
    plt.plot(time, load, label='load')
if 'batPwr' in plt_lst:
    batPwr = data['batPwr'].to_numpy()
    batPwr = batPwr/load.max()
    plt.plot(time, batPwr, label='batPwr')
    plt.fill_between(
        x= time,
        y1= batPwr,
        color= "blue",
        alpha= 0.2)
if 'engPwr' in plt_lst:
    engPwr = data['engPwr'].to_numpy()
    engPwr = engPwr/load.max()
    plt.plot(time, engPwr, label='engPwr')
    plt.fill_between(
        x= time,
        y1= engPwr,
        color= "yellow",
        alpha= 0.2)
if 'powerDeficit' in plt_lst:
    powerDeficit = data['powerDeficit'].to_numpy()
    powerDeficit = powerDeficit/powerDeficit.max()
    plt.plot(time, powerDeficit, label='powerDeficit')
    plt.fill_between(
        x= time,
        y1= powerDeficit,
        color= "red",
        alpha= 0.7)
if 'charge' in plt_lst:
    charge = data['charge'].to_numpy()
    charge = charge/charge.max()
    plt.plot(time, charge, label='charge')



plt.legend(loc="lower left")
# Choose the Slider color
slider_color = 'White'

# Set the axis and slider position in the plot
axis_position = plt.axes([0.2, 0.1, 0.65, 0.03],
                         facecolor = slider_color)
slider_position = Slider(axis_position,
                         'Pos', 0, len(data.index))

# update() function to change the graph when the
# slider is in use
def update(val):
    pos = slider_position.val
    Axis.axis([pos, pos+1000, -1.5, 1.5])
    Plot.canvas.draw_idle()

# update function called using on_changed() function
slider_position.on_changed(update)

# Display the plot
plt.show()
