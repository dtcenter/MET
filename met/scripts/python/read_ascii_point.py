from __future__ import print_function

import pandas as pd
import os
import sys

###########################################

   ##
   ##  input file specified on the command line
   ##  load the data into the numpy array
   ##

if len(sys.argv) == 2:
    # Read the input file as the first argument
    input_file = os.path.expandvars(sys.argv[1])
    try:
        # Print some output to verify that this script ran
        print("Input File:  " + repr(input_file))
        point_data = pd.read_csv(input_file, header=None, delim_whitespace=True).values.tolist()
        print("Data Length: " + repr(len(point_data)))
        print("Data Type:   " + repr(type(point_data)))
    except NameError:
        print("Can't find the input file")
else:
    print("ERROR: Must specify exactly one input file and a name for the data.")
    sys.exit(1)

###########################################
