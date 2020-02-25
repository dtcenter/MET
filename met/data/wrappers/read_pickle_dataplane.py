########################################################################
#
#    Reads temporary pickle file into memory.
#
#    usage:  /path/to/python read_pickle_dataplane.py pickle.tmp
#
########################################################################

import sys
import numpy as np
import pickle

print('Python Script:\t', sys.argv[0])
print('Load Pickle:\t', sys.argv[1])
met_info = pickle.load(open(sys.argv[1], "rb"))
