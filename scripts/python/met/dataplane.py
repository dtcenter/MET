import os
import numpy as np
import netCDF4 as nc

###########################################

class dataplane():

    ##
    ##  create the metadata dictionary
    ##

    #@staticmethod
    def get_grid_metadata(data_name):
        attrs = {

            'valid': '20050807_120000',
            'init':  '20050807_000000',
            'lead':  '120000',
            'accum': '120000',

            'name':      data_name,
            'long_name': data_name + '_word',
            'level':     'Surface',
            'units':     'None',

            'grid': {
                'type': 'Lambert Conformal',
                'hemisphere': 'N',

                'name': 'FooGrid',

                'scale_lat_1': 25.0,
                'scale_lat_2': 25.0,

                'lat_pin': 12.19,
                'lon_pin': -135.459,

                'x_pin': 0.0,
                'y_pin': 0.0,

                'lon_orient': -95.0,

                'd_km': 40.635,
                'r_km': 6371.2,

                'nx': 185,
                'ny': 129,
            }

        }
        return attrs

    ##
    ##  create the metadata dictionary from the environment variable PYTHON_GRID
    ##

    #@staticmethod
    def get_grid_metadata_from_env(data_name, grid_env_name='PYTHON_GRID'):
        attrs = {

            'valid': '20050807_120000',
            'init':  '20050807_000000',
            'lead':  '120000',
            'accum': '120000',

            'name':      data_name,
            'long_name': data_name + '_word',
            'level':     'Surface',
            'units':     'None',
            'grid':      os.path.expandvars(os.getenv(grid_env_name))
        }
        return attrs

    #@staticmethod
    def load_txt(input_file, data_name):
        try:
            print("Input File:\t" + repr(input_file))
            print("Data Name:\t" + repr(data_name))
            met_data = np.loadtxt(input_file)
            print("Data Shape:\t" + repr(met_data.shape))
            print("Data Type:\t" + repr(met_data.dtype))
        except NameError:
            met_data = None
            print("Can't find the input file")
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

