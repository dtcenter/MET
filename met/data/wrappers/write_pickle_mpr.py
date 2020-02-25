########################################################################
#
#    Adapted from a script provided by George McCabe
#    Adapted by Randy Bullock
#
#    usage:  /path/to/python write_pickle_mpr.py \
#            pickle_output_filename <user_python_script>.py <args>
#
########################################################################

import os
import sys
import pickle

print('Python Script:\t', sys.argv[0])
print('User Command:\t',  sys.argv[2:])
print('Write Pickle:\t',  sys.argv[1])

pickle_filename = sys.argv[1];

pyembed_module_dir  = os.path.dirname(sys.argv[2])
pyembed_module_name = os.path.basename(sys.argv[2]).replace('.py','')

sys.argv = sys.argv[2:]

sys.path.append(pyembed_module_dir)
met_in = __import__(pyembed_module_name)

print(met_in)

pickle.dump( met_in.mpr_data, open( pickle_filename, "wb" ) )
