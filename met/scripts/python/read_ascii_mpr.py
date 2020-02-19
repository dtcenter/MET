from __future__ import print_function

import pandas as pd
import os
import sys

########################################################################

print('Python Script:\t', sys.argv[0])

   ##
   ##  input file specified on the command line
   ##  load the data into the numpy array
   ##

if len(sys.argv) == 2:
    # Read the input file as the first argument
    input_file = os.path.expandvars(sys.argv[1])
    try:
        print("Input File:\t" + repr(input_file))

        # Read MPR lines, skipping the header row and first column.
        mpr_data = pd.read_csv(input_file, header=None,
                        delim_whitespace=True, keep_default_na=False,
                        skiprows=1, usecols=range(1,36),
                        dtype=str).values.tolist()
        print("Data Length:\t" + repr(len(mpr_data)))
        print("Data Type:\t" + repr(type(mpr_data)))
    except NameError:
        print("Can't find the input file")
else:
    print("ERROR: read_ascii_point.py -> Must specify exactly one input file.")
    sys.exit(1)

########################################################################

