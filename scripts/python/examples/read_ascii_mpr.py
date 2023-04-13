import os
import sys
from met.mprbase import mpr_data


########################################################################

print("Python Script:\t" + repr(sys.argv[0]))

   ##
   ##  input file specified on the command line
   ##  load the data into the numpy array
   ##

if len(sys.argv) != 2:
   print("ERROR: read_ascii_mpr.py -> Must specify exactly one input file.")
   sys.exit(1)

# Read the input file as the first argument
input_file = os.path.expandvars(sys.argv[1])
try:
   print("Input File:\t" + repr(input_file))

   # Read MPR lines by using pandas package, skipping the header row and
   # first column. Input should be a 36 column text data.
   mpr_data = mpr_data.read_mpr(input_file, col_start=1, col_last=36, skiprows=1)
   print("Data Length:\t" + repr(len(mpr_data)))
   print("Data Type:\t" + repr(type(mpr_data)))
except NameError:
   print("Can't find the input file")

########################################################################
