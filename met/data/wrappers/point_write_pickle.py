

################################################
#
#    Adapted from a script provided by George McCabe
#    Adapted by Randy Bullock
#
#    usage:  /path/to/python write_pickle.py pickle_output_filename <user_python_script>.py <args>
#
################################################

import os
import sys
import pickle

pickle_filename = sys.argv[1];

print('old args')
print(sys.argv)

pyembed_module_dir  = os.path.dirname(sys.argv[2])
pyembed_module_name = os.path.basename(sys.argv[2]).replace('.py','')
sys.argv = sys.argv[2:]

print('new args')
print(sys.argv)

sys.path.append(pyembed_module_dir)
met_in = __import__(pyembed_module_name)

pickle.dump( met_in.point_data, open( pickle_filename, "wb" ) )
