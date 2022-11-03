import numpy as np
import os
import sys

###########################################

print("Python Script:\t" + repr(sys.argv[0]))

   ##
   ##  input file specified on the command line
   ##  load the data into the numpy array
   ##

if len(sys.argv) != 3:
    print("ERROR: read_ascii_numpy.py -> Must specify exactly one input file and a name for the data.")
    sys.exit(1)

# Read the input file as the first argument
input_file = os.path.expandvars(sys.argv[1])
data_name  = sys.argv[2]
try:
    # Print some output to verify that this script ran
    print("Input File:\t" + repr(input_file))
    print("Data Name:\t" + repr(data_name))
    met_data = np.loadtxt(input_file)
    print("Data Shape:\t" + repr(met_data.shape))
    print("Data Type:\t" + repr(met_data.dtype))
except NameError:
    print("Can't find the input file")

###########################################

   ##
   ##  create the metadata dictionary
   ##

attrs = {

   'valid': '20050807_120000',
   'init':  '20050807_000000',
   'lead':  '120000',
   'accum': '120000',

   'name':      data_name,
   'long_name': data_name + '_word',
   'level':     'Surface',
   'units':     'None',
   'grid':      os.path.expandvars(os.getenv('PYTHON_GRID'))
}

print("Attributes:\t" + repr(attrs))
