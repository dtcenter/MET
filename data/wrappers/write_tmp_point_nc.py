########################################################################
#
#    Adapted from a script provided by George McCabe
#    Adapted by Randy Bullock
#
#    usage:  /path/to/python write_tmp_point.py \
#            tmp_output_filename <user_python_script>.py <args>
#
########################################################################

import os
import sys
import importlib.util

met_base_dir = os.getenv('MET_BASE',None)
if met_base_dir is not None:
    sys.path.append(os.path.join(met_base_dir, 'python'))

from met_point_obs import met_point_obs
from met_point_obs_nc import nc_point_obs
    
PROMPT = met_point_obs.get_prompt()
print("{p} Python Script:\t".format(p=PROMPT)  + repr(sys.argv[0]))
print("{p} User Command:\t".format(p=PROMPT)   + repr(' '.join(sys.argv[2:])))
print("{p} Temporary File:\t".format(p=PROMPT) + repr(sys.argv[1]))

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

if hasattr(met_in, 'point_obs_data'):
    met_in.point_obs_data.save_ncfile(tmp_filename)
else:
    if hasattr(met_in.met_point_data, 'point_obs_data'):
        met_in.met_point_data['point_obs_data'].save_ncfile(tmp_filename)
    else:
        tmp_point_obs = nc_point_obs()
        tmp_point_obs.put_data(met_in.met_point_data)
        tmp_point_obs.save_ncfile(tmp_filename)

#print('{p} writing {f}'.format(p=PROMPT, f=tmp_filename))
