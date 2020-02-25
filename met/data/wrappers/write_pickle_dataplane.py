########################################################################
#
#    Adapted from a script provided by George McCabe
#    Adapted by Randy Bullock
#
#    usage:  /path/to/python write_pickle_dataplane.py \
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

pyembed_module_name = sys.argv[2].replace('.py','')
sys.argv = sys.argv[2:]

user_dir  = os.path.dirname(pyembed_module_name)
user_base = os.path.basename(pyembed_module_name)

if len(user_dir) != 0:
   sys.path.append(user_dir)

met_in = __import__(user_base)

met_info = { 'attrs': met_in.attrs, 'met_data': met_in.met_data }

print(met_info)

pickle.dump( met_info, open( pickle_filename, "wb" ) )
