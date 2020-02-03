from __future__ import print_function

import numpy as np
import sys
import pickle

###########################################

   ##
   ##  usage generic_pickle pickle_filename
   ##

if len(sys.argv) == 2:
    # Read the input file as the first argument
    pickle_filename = sys.argv[1];
    try:
        met_info = pickle.load(open(pickle_filename, "rb"))
    except NameError:
        print("Can't find the input file")
else:
    print("ERROR: load_pickle.py -> Must specify the pickle filename.")
    sys.exit(1)

###########################################
