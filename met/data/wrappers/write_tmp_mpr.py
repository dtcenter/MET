########################################################################
#
#    Adapted from a script provided by George McCabe
#    Adapted by Randy Bullock
#
#    usage:  /path/to/python write_tmp_mpr.py \
#            tmp_output_filename <user_python_script>.py <args>
#
########################################################################

import os
import sys
import importlib.util

print("Python Script:\t"  + repr(sys.argv[0]))
print("User Command:\t"   + repr(' '.join(sys.argv[2:])))
print("Temporary File:\t" + repr(sys.argv[1]))

tmp_filename = sys.argv[1]
pyembed_module_name = sys.argv[2]
sys.argv = sys.argv[2:]

# append user script dir to system path
pyembed_dir, pyembed_file = os.path.split(pyembed_module_name)
if pyembed_dir:
    sys.path.insert(0, pyembed_dir)

if not pyembed_module_name.endswith('.py'):
    pyembed_module_name += '.py'

user_base = os.path.basename(pyembed_module_name).replace('.py','')

spec = importlib.util.spec_from_file_location(user_base, pyembed_module_name)
met_in = importlib.util.module_from_spec(spec)
spec.loader.exec_module(met_in)

f = open(tmp_filename, 'w')
for line in met_in.mpr_data:
    f.write(str(line) + '\n')
