import os
import numpy as np
import netCDF4 as nc

###########################################

class dataplane():

   ##
   ##  create the metadata dictionary
   ##

   #@staticmethod
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

   #@staticmethod
   def read_2d_text_input(input_file):
      if os.path.exists(input_file):
          met_data = np.loadtxt(input_file)
      else:
          met_data = None
      return met_data

   #@staticmethod
   def read_dataplane(netcdf_filename):
      # read NetCDF file
      ds = nc.Dataset(netcdf_filename, 'r')
      met_data = ds['met_data'][:]
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
      met_info['attrs'] = met_attrs
      return met_info

   #@staticmethod
   def write_dataplane(met_in, netcdf_filename):
      met_info = {'met_data': met_in.met_data}
      if hasattr(met_in.met_data, 'attrs') and met_in.met_data.attrs:
         attrs = met_in.met_data.attrs
      else:
         attrs = met_in.attrs
      met_info['attrs'] = attrs

      # determine fill value
      try:
         fill = met_in.met_data.get_fill_value()
      except:
         fill = -9999.

      # write NetCDF file
      ds = nc.Dataset(netcdf_filename, 'w')

      # create dimensions and variable
      nx, ny = met_in.met_data.shape
      ds.createDimension('x', nx)
      ds.createDimension('y', ny)
      dp = ds.createVariable('met_data', met_in.met_data.dtype, ('x', 'y'), fill_value=fill)
      dp[:] = met_in.met_data

      # append attributes
      for attr, attr_val in met_info['attrs'].items():
         if attr == 'name':
            setattr(ds, 'name_str', attr_val)
         elif type(attr_val) == dict:
            for key in attr_val:
               setattr(ds, attr + '.' + key, attr_val[key])
         else:
            setattr(ds, attr, attr_val)

      ds.close()

