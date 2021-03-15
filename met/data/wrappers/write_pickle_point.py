########################################################################
#
#    Adapted from a script provided by George McCabe
#    Adapted by Randy Bullock
#
#    usage:  /path/to/python write_pickle_point.py \
#            pickle_output_filename <user_python_script>.py <args>
#
########################################################################

import os
import sys
import pickle
import importlib.util

print('Python Script:\t', sys.argv[0])
print('User Command:\t',  sys.argv[2:])
print('Write Pickle:\t',  sys.argv[1])

pickle_filename = sys.argv[1]

pyembed_module_name = sys.argv[2]
sys.argv = sys.argv[2:]

user_base = os.path.basename(pyembed_module_name).replace('.py','')

spec = importlib.util.spec_from_file_location(user_base, pyembed_module_name)
met_in = importlib.util.module_from_spec(spec)
spec.loader.exec_module(met_in)

pickle.dump( met_in.point_data, open( pickle_filename, "wb" ) )
