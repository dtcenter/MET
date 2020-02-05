##
##  When MET_PYTHON_EXE is defined, this script initializes the Python
##  environment for reading the temporary pickle file back into MET.
##

import sys
import numpy as np
import pickle

#
#  If an argument was provided, load it as a pickle file.
#

if len(sys.argv) > 1:
    print('Python Script:\t', sys.argv[0])
    print('Load Pickle:\t', sys.argv[1])
    met_info = pickle.load(open(sys.argv[1], "rb"))
