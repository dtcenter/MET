

##
##  usage generic_pickle pickle_filename
##

import sys
import numpy as np
import pickle

pickle_filename = sys.argv[1];

#print ('pickle_filename = ' + pickle_filename)

met_info = pickle.load(open(pickle_filename, "rb"))



