########################################################################
#
#    Adapted from a script provided by George McCabe
#    Adapted by Randy Bullock
#
#    usage:  /path/to/python write_tmp_point.py \
#            tmp_ascii_output_filename <user_python_script>.py <args>
#
########################################################################

import os
import sys
import importlib.util

print('Python Script:\t', sys.argv[0])
print('User Command:\t',  sys.argv[2:])
print('Write Temporary Ascii:\t',  sys.argv[1])

tmp_filename = sys.argv[1] + '.txt'

pyembed_module_name = sys.argv[2]
sys.argv = sys.argv[2:]

user_base = os.path.basename(pyembed_module_name).replace('.py','')

spec = importlib.util.spec_from_file_location(user_base, pyembed_module_name)
met_in = importlib.util.module_from_spec(spec)
spec.loader.exec_module(met_in)

f = open(tmp_filename, 'w')
for line in met_in.point_data:
    f.write(str(line) + '\n')
