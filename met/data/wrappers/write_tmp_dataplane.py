########################################################################
#
#    Adapted from a script provided by George McCabe
#    Adapted by Randy Bullock
#
#    usage:  /path/to/python write_tmp_dataplane.py \
#            tmp_output_filename <user_python_script>.py <args>
#
########################################################################

import os
import sys
import importlib.util
import netCDF4 as nc

print('Python Script:\t', sys.argv[0])
print('User Command:\t',  sys.argv[2:])

netcdf_filename = sys.argv[1]

print('Write NetCDF:\t',  netcdf_filename)

pyembed_module_name = sys.argv[2]
sys.argv = sys.argv[2:]

if not pyembed_module_name.endswith('.py'):
    pyembed_module_name += '.py'

user_base = os.path.basename(pyembed_module_name).replace('.py','')

spec = importlib.util.spec_from_file_location(user_base, pyembed_module_name)
met_in = importlib.util.module_from_spec(spec)
spec.loader.exec_module(met_in)

met_info = {'met_data': met_in.met_data}
if hasattr(met_in.met_data, 'attrs') and met_in.met_data.attrs:
    attrs = met_in.met_data.attrs
else:
    attrs = met_in.attrs
met_info['attrs'] = attrs

print('write_tmp_dataplane')
print(met_info)

# write NetCDF file
ds = nc.Dataset(netcdf_filename, 'w')

nx, ny = met_in.met_data.shape
ds.createDimension('x', nx)
ds.createDimension('y', ny)
dp = ds.createVariable('met_data', met_in.met_data.dtype, ('x', 'y'))
dp[:] = met_in.met_data

for attr, attr_val in met_info['attrs'].items():
    print(attr, attr_val, type(attr_val))
    if attr == 'name':
        setattr(ds, '_name', attr_val)
    if type(attr_val) == str:
        setattr(ds, attr, attr_val)
    if type(attr_val) == dict:
        for key in attr_val:
            setattr(ds, attr + '.' + key, attr_val[key])
ds.close()
