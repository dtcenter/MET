import os
import sys
import json
import numpy as np
import xarray as xr

from importlib import util as import_util
from met.logger import met_base, met_base_tools

###########################################

class dataplane(met_base):

   KEEP_XARRAY = True
   class_name = "dataplane"

   ATTR_USER_FILL_VALUE = 'user_fill_value'

   @staticmethod
   def call_python(argv):
       # argv[0] is the python wrapper script (caller)
       met_base.log_message(f"Module:\t{repr(argv[0])}")
       if 1 == len(argv):
          met_base.quit_msg(f"User python command is missing")
          sys.exit(1)

       met_base.log_message(f"User python command:\t{repr(' '.join(argv[1:]))}")

       # argv[1] contains the user defined python script
       pyembed_module_name = argv[1]
       # append user script dir to system path
       pyembed_dir, pyembed_name = os.path.split(pyembed_module_name)
       if pyembed_dir:
          sys.path.insert(0, pyembed_dir)

       if not pyembed_module_name.endswith('.py'):
          pyembed_module_name += '.py'

       user_base = pyembed_name.replace('.py','')

       argv_org = sys.argv  # save sys.argv
       sys.argv = argv[1:]
       spec = import_util.spec_from_file_location(user_base, pyembed_module_name)
       met_in = import_util.module_from_spec(spec)
       spec.loader.exec_module(met_in)
       sys.argv = argv_org  # restore sys.argv
       return met_in

   #@staticmethod
   #def get_numpy_filename(tmp_filename):
   #   return met_base_tools.replace_extension(tmp_filename, "json", "npy") if tmp_filename.endswith(".json") else \
   #          met_base_tools.replace_extension(tmp_filename, "nc", "npy") if tmp_filename.endswith(".nc") else f'{tmp_filename}.npy'

   @staticmethod
   def is_integer(a_data):
      return isinstance(a_data, int)

   @staticmethod
   def is_xarray_dataarray(a_data):
      return isinstance(a_data, xr.DataArray)

   ##
   ##  create the metadata dictionary
   ##

   # Python dictionary items:
   #      'name': data name
   # 'long_name': descriptive name
   #     'valid': valid time (format = 'yyyymmdd_hhmmss')
   #      'init': init time (format = 'yyyymmdd_hhmmss')
   #      'lead': lead time (format = 'hhmmss')
   #     'accum': accumulation time (format = 'hhmmss')
   #     'level': vertilcal level
   #     'units': units of the data
   #      'grid': contains the grid information
   #              - a grid name (G212)
   #              - a gridded data file name
   #              - MET specific grid string, "lambert 185 129 12.19 -133.459 -95 40.635 6371.2 25 25 N"
   #              - a dictionary for the grid information
   @staticmethod
   def set_dataplane_attrs(data_name, valid_time, init_time, lead_time,
                           accum_time, v_level, units, grid_info, long_name=None):
      hdr_attrs = {

         'valid': valid_time,
         'init':  init_time,
         'lead':  lead_time,
         'accum': accum_time,

         'name':      data_name,
         'long_name': long_name if long_name is not None and long_name != "" else data_name + '_long',
         'level':     v_level,
         'units':     units,

         'grid': grid_info

      }
      return hdr_attrs

   @staticmethod
   def read_2d_text_input(input_file):
      if os.path.exists(input_file):
          met_data = np.loadtxt(input_file)
      else:
          met_data = None
      return met_data

   @staticmethod
   def read_dataplane(tmp_filename):
      # Default is JSON for attributes and NUMPY serialization for 2D array
      return dataplane.read_dataplane_nc(tmp_filename) if met_base_tools.use_netcdf_format() \
             else dataplane.read_dataplane_json_numpy(tmp_filename)

   @staticmethod
   def read_dataplane_json_numpy(tmp_filename):
      if met_base_tools.is_debug_enabled("dataplane"):
         met_base.log_message(f"Read from a temporary JSON file and a temporary numpy output (dataplane)")

      met_info = {}
      with open(tmp_filename) as json_fh:
         met_info['attrs'] = json.load(json_fh)
      # read 2D numeric data
      numpy_dump_name = met_base_tools.get_numpy_filename(tmp_filename)
      met_dp_data = np.load(numpy_dump_name)
      met_info['met_data'] = met_dp_data
      if numpy_dump_name != tmp_filename:
         met_base_tools.remove_temp_file(numpy_dump_name)
      return met_info

   @staticmethod
   def read_dataplane_nc(netcdf_filename):
      import netCDF4 as nc

      if met_base_tools.is_debug_enabled("dataplane"):
         met_base.log_message(f"Read from a temporary NetCDF file (dataplane)")

      # read NetCDF file
      ds = nc.Dataset(netcdf_filename, 'r')

      dp = ds['met_data']
      met_data = dp[:]
      attr_name = dataplane.ATTR_USER_FILL_VALUE
      user_fill_value = dp.getncattr(attr_name) if hasattr(dp, attr_name) else None

      met_attrs = {}

      # grid is defined as a dictionary or string
      grid = {}
      for attr, attr_val in ds.__dict__.items():
         if 'grid.' in attr:
            grid_attr = attr.split('.')[1]
            grid[grid_attr] = attr_val
         else:
            met_attrs[attr] = attr_val

      if grid:
         met_attrs['grid'] = grid

      met_attrs['name'] = met_attrs['name_str']
      del met_attrs['name_str']

      met_info = {}
      met_info['met_data'] = met_data
      if user_fill_value is not None:
         met_attrs['fill_value'] = user_fill_value
      met_info['attrs'] = met_attrs

      return met_info

   @staticmethod
   def validate_met_data(met_data, fill_value=None):
      method_name = f"{dataplane.class_name}.validate()"
      #met_base.log_msg(f"{method_name} type(met_data)= {type(met_data)}")
      attrs = None
      from_xarray = False
      from_ndarray = False
      if met_data is None:
         met_base.quit(f"{method_name} The met_data is None")
         sys.exit(1)

      if hasattr(met_data, 'shape'):
          nx, ny = met_data.shape
      else:
         met_base.quit(f"{method_name} The met_data does not have shape member")
         sys.exit(1)

      met_fill_value = met_base.MET_FILL_VALUE
      if dataplane.is_xarray_dataarray(met_data):
         from_xarray = True
         attrs = met_data.attrs
         met_data = met_data.data
         modified_met_data = True
      if isinstance(met_data, np.ndarray):
         from_ndarray = True
         met_data = np.ma.array(met_data)

      if isinstance(met_data, np.ma.MaskedArray):
         is_int_data = dataplane.is_integer(met_data[0,0]) or dataplane.is_integer(met_data[int(nx/2),int(ny/2)])
         met_data = np.ma.masked_equal(met_data, float('nan'))
         met_data = np.ma.masked_equal(met_data, float('inf'))
         if fill_value is not None:
            met_data = np.ma.masked_equal(met_data, fill_value)
         met_data = met_data.filled(int(met_fill_value) if is_int_data else met_fill_value)
      else:
         met_base.log_message(f"{method_name} unknown datatype {type(met_data)}")

      if dataplane.KEEP_XARRAY:
         return xr.DataArray(met_data,attrs=attrs) if from_xarray else met_data
      else:
         return met_data

   @staticmethod
   def write_dataplane(met_in, tmp_filename):
      # Default is JSON for attributes and NUMPY serialization for 2D array
      if met_base_tools.use_netcdf_format():
         dataplane.write_dataplane_nc(met_in, tmp_filename)
      else:
         dataplane.write_dataplane_json_numpy(met_in, tmp_filename)

   @staticmethod
   def write_dataplane_json_numpy(met_in, tmp_filename):
      if met_base_tools.is_debug_enabled("dataplane"):
         met_base.log_message(f"Save to a temporary JSON file and a temporary numpy output (dataplane)")
      if hasattr(met_in.met_data, 'attrs') and met_in.met_data.attrs:
         attrs = met_in.met_data.attrs
      else:
         attrs = met_in.attrs
      with open(tmp_filename,'w') as json_fh:
          json.dump(attrs, json_fh)

      met_dp_data = met_base_tools.convert_to_ndarray(met_in.met_data)
      numpy_dump_name = met_base_tools.get_numpy_filename(tmp_filename)
      np.save(numpy_dump_name, met_dp_data)

   @staticmethod
   def write_dataplane_nc(met_in, netcdf_filename):
      import netCDF4 as nc

      if met_base_tools.is_debug_enabled("dataplane"):
         met_base.log_message(f"Save to a temporary NetCDF file (dataplane)")

      met_info = {'met_data': met_in.met_data}
      if hasattr(met_in.met_data, 'attrs') and met_in.met_data.attrs:
         attrs = met_in.met_data.attrs
      else:
         attrs = met_in.attrs
      met_info['attrs'] = attrs

      # write NetCDF file
      ds = nc.Dataset(netcdf_filename, 'w')

      # create dimensions and variable
      nx, ny = met_in.met_data.shape
      ds.createDimension('x', nx)
      ds.createDimension('y', ny)
      dp = ds.createVariable('met_data', met_in.met_data.dtype, ('x', 'y'),
                             fill_value=dataplane.MET_FILL_VALUE)
      dp[:] = met_in.met_data

      # append attributes
      for attr, attr_val in met_info['attrs'].items():
         if attr_val is None:
            continue

         if attr == 'name':
            setattr(ds, 'name_str', attr_val)
         elif attr == 'fill_value':
            setattr(dp, dataplane.ATTR_USER_FILL_VALUE, attr_val)
         elif type(attr_val) == dict:
            for key in attr_val:
               setattr(ds, attr + '.' + key, attr_val[key])
         else:
            setattr(ds, attr, attr_val)

      ds.close()



def main(argv):
   global attrs, met_data, met_info

   met_in = dataplane.call_python(sys.argv)

   user_fill_value = None
   try:
      met_info = met_in.met_info
      attrs = met_info['attrs']
      init_met_data = met_info['met_data']
   except:
      met_info = {}
      init_met_data = met_in.met_data
      try:     # numpy and attrs
         attrs = met_in.attrs
      except:  # xarray
         attrs = init_met_data.attrs
      met_info['attrs'] = attrs
      if hasattr(met_in, 'user_fill_value'):
         fill_value = met_in.user_fill_value

   fill_value = attrs.get('fill_value', None)
   met_base.log_message('validating the dataplane array...')
   met_data = dataplane.validate_met_data(init_met_data, fill_value)
   met_info['met_data'] = met_data

   if os.environ.get('MET_PYTHON_DEBUG', None) is not None:
      met_base.log_message('--- met_data after validating ---')
      met_base.log_message(met_data)

if __name__ == '__main__' or __name__ == sys.argv[0]:
   main(sys.argv)
   met_base.log_message(f'{__name__} complete')
