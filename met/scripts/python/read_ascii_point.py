from __future__ import print_function

import pandas as pd
import os
import sys

###########################################

print('Python Script:\t', sys.argv[0])

   ##
   ##  input file specified on the command line
   ##  load the data into the numpy array
   ##

if len(sys.argv) == 2:
    # Read the input file as the first argument
    input_file = os.path.expandvars(sys.argv[1])
    try:
        # Print some output to verify that this script ran
        print("Input File:\t" + repr(input_file))
        # TODO: make this work
        #point_data = pd.read_csv(input_file, header=None, delim_whitespace=True).values.tolist()
        
        point_data = [[ 'ADPUPA', '72365', '20070331_120000',   35.03, -106.62, 1618.0,  7, 837.0,  1618, 'NA', 1618    ], 
                      [ 'ADPUPA', '72365', '20070331_120000',   35.03, -106.62, 1618.0, 11, 837.0,  1618, 'NA', 273.05  ], 
                      [ 'ADPUPA', '72365', '20070331_120000',   35.03, -106.62, 1618.0, 17, 837.0,  1618, 'NA', 271.85  ], 
                      [ 'ADPUPA', '72365', '20070331_120000',   35.03, -106.62, 1618.0, 52, 837.0,  1618, 'NA', 92      ], 
                      [ 'ADPUPA', '72365', '20070331_120000',   35.03, -106.62, 1618.0, 53, 837.0,  1618, 'NA', 0.00417 ], 
                      [ 'ADPUPA', '72365', '20070331_120000',   35.03, -106.62, 1618.0,  7, 826.0,  1724, 'NA', 1724    ], 
                      [ 'ADPUPA', '72365', '20070331_120000',   35.03, -106.62, 1618.0, 11, 826.0,  1724, 'NA', 274.55  ]]
        
        print("Data Length:\t" + repr(len(point_data)))
        print("Data Type:\t" + repr(type(point_data)))
    except NameError:
        print("Can't find the input file")
else:
    print("ERROR: read_ascii_point.py -> Must specify exactly one input file.")
    sys.exit(1)

###########################################
