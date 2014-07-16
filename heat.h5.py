import h5py
import sys

import scipy as sp
import matplotlib
matplotlib.use("GTKAgg")
import gobject
from pylab import *

if len(sys.argv) != 4:
  print( "Error: invalid arguments" )
  print( "usage: heat <h5filename> <dimension([x->1|y->2|z->3])> <sheet>" )
  exit()
  
# Obtain filename from command-line parameters
filename = sys.argv[1]

# Open file
file = h5py.File( filename, "r" )

# Extract temperature data
temperature = file["temperatura"]

# Function called for updating the figure
def updatefig(*args):
  global temperature, frame, sheet
  if sys.argv[2] == '1':
    im.set_array(temperature[frame,sheet,:,:])
  elif sys.argv[2] == '2':
     im.set_array(temperature[frame,:,sheet,:])
  else:
     im.set_array(temperature[frame,:,:,sheet])
  manager.canvas.draw()
  frame+=1
  print "Rendering timestep t=",frame
  if(frame>=len(temperature)):
    return False
  return True

# Create figure and plot initial data
fig = plt.figure(1)
img = subplot(111)

sheet = int(sys.argv[3])

if sys.argv[2] == '1':
   im  = img.imshow( temperature[0,sheet,:,:], cmap=cm.hot, interpolation="nearest",
                  origin="lower")
elif sys.argv[2] == '2':
   im  = img.imshow( temperature[0,:,sheet,:], cmap=cm.hot, interpolation="nearest",
                  origin="lower")
else:
   im  = img.imshow( temperature[0,:,:,sheet], cmap=cm.hot, interpolation="nearest",
                  origin="lower")
manager = get_current_fig_manager()

frame = 1
fig.colorbar(im)

# Whenever idle, update the figure while updatefig returns True
gobject.idle_add( updatefig )
show()
