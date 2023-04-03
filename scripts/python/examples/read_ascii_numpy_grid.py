import os
import sys
from met.dataplane import load_txt, get_grid_metadata_from_env

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

met_data = load_txt(input_file, data_name)

## create the metadata dictionary from the environment variable,
## Default env_name = 'PYTHON_GRID'

attrs = get_grid_metadata_from_env(data_name)

print("Attributes:\t" + repr(attrs))
