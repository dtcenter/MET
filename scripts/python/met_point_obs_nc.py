#!/usr/bin/env python3

'''
Separated from read_met_point_obs on Feb 09, 2023

@author: hsoh

This script reads the MET point observation NetCDF file like MET tools do.
'''

import os
import sys
from datetime import datetime
import numpy as np
import netCDF4 as nc

from met_point_obs import met_point_obs, base_met_point_obs, get_prompt

DO_PRINT_DATA = False
ARG_PRINT_DATA = 'show_data'

# Note: caller should import netCDF4
# the argements nc_group(dataset) and nc_var should not be None
class nc_tools():

    met_missing = -99999999.

    @staticmethod
    def get_num_array(nc_group, var_name):
        nc_var = nc_group.variables.get(var_name, None)
        return [] if nc_var is None else nc_var[:]

    @staticmethod
    def get_ncbyte_array_to_str(nc_var):
        nc_str_data = nc_var[:]
        if nc_var.datatype.name == 'bytes8':
            nc_str_data = [ str(s.compressed(),"utf-8") for s in nc_var[:] ]
        return nc_str_data

    @staticmethod
    def get_string_array(nc_group, var_name):
        nc_var = nc_group.variables.get(var_name, None)
        return [] if nc_var is None else nc_tools.get_ncbyte_array_to_str(nc_var)


class nc_point_obs(met_point_obs):

    # args should be string, list, or dictionary
    def get_nc_filename(self, args):
        nc_filename = None
        if isinstance(args, dict):
            nc_filename = args.get('nc_name',None)
        elif isinstance(args, list):
            nc_filename = args[0]
        elif args != ARG_PRINT_DATA:
            nc_filename = args
        
        return nc_filename

    def read_data(self, nc_filename):
        if nc_filename is None:
            self.log_error_msg("The input NetCDF filename is missing")
        elif not os.path.exists(nc_filename):
            self.log_error_msg(f"input NetCDF file ({nc_filename}) does not exist")
        else:
            dataset = nc.Dataset(nc_filename, 'r')

            attr_name = 'use_var_id'
            use_var_id_str = dataset.getncattr(attr_name) if attr_name in dataset.ncattrs() else "false"
            self.use_var_id = use_var_id_str.lower() == 'true'

            # Header
            self.hdr_typ = dataset['hdr_typ'][:]
            self.hdr_sid = dataset['hdr_sid'][:]
            self.hdr_vld = dataset['hdr_vld'][:]
            self.hdr_lat = dataset['hdr_lat'][:]
            self.hdr_lon = dataset['hdr_lon'][:]
            self.hdr_elv = dataset['hdr_elv'][:]
            self.hdr_typ_table = nc_tools.get_string_array(dataset, 'hdr_typ_table')
            self.hdr_sid_table = nc_tools.get_string_array(dataset, 'hdr_sid_table')
            self.hdr_vld_table = nc_tools.get_string_array(dataset, 'hdr_vld_table')

            nc_var = dataset.variables.get('obs_unit', None)
            if nc_var:
                self.obs_var_unit  = nc_var[:]
            nc_var = dataset.variables.get('obs_desc', None)
            if nc_var:
                self.obs_var_desc  = nc_var[:]

            nc_var = dataset.variables.get('hdr_prpt_typ', None)
            if nc_var:
                self.hdr_prpt_typ  = nc_var[:]
            nc_var = dataset.variables.get('hdr_irpt_typ', None)
            if nc_var:
                self.hdr_irpt_typ  = nc_var[:]
            nc_var = dataset.variables.get('hdr_inst_typ', None)
            if nc_var:
                self.hdr_inst_typ  =nc_var[:]

            #Observation data        
            self.hdr_sid = dataset['hdr_sid'][:]
            self.obs_qty = np.array(dataset['obs_qty'][:])
            self.obs_hid = np.array(dataset['obs_hid'][:])
            self.obs_lvl = np.array(dataset['obs_lvl'][:])
            self.obs_hgt = np.array(dataset['obs_hgt'][:])
            self.obs_val = np.array(dataset['obs_val'][:])
            nc_var = dataset.variables.get('obs_vid', None)
            if nc_var is None:
                self.use_var_id = False
                nc_var = dataset.variables.get('obs_gc', None)
            else:
                self.obs_var_table = nc_tools.get_string_array(dataset, 'obs_var')
            if nc_var:
                self.obs_vid = np.array(nc_var[:])
            
            self.obs_qty_table = nc_tools.get_string_array(dataset, 'obs_qty_table')

    def save_ncfile(self, nc_filename):
        met_data = self.get_point_data()
        with nc.Dataset(nc_filename, 'w') as nc_dataset:
            self.set_nc_data(nc_dataset)
        return met_data

    def set_nc_data(self, nc_dataset):
        return nc_point_obs.write_nc_data(nc_dataset, self)

    @staticmethod
    def write_nc_file(nc_filename, point_obs):
        with nc.Dataset(nc_filename, 'w') as nc_dataset:
            nc_point_obs.set_nc_data(nc_dataset, point_obs)

    @staticmethod
    def write_nc_data(nc_dataset, point_obs):
        do_nothing = False
        if 0 == point_obs.nhdr:
            do_nothing = True
            base_met_point_obs.info_msg("the header is empty")
        if 0 == point_obs.nobs:
            do_nothing = True
            base_met_point_obs.info_msg("the observation data is empty")
        if do_nothing:
            print()
            return 

        # Set global attributes
        nc_dataset.MET_Obs_version = "1.02" ;
        nc_dataset.use_var_id = "true" if point_obs.use_var_id else "false"

        # Create dimensions
        nc_dataset.createDimension('mxstr', 16)
        nc_dataset.createDimension('mxstr2', 40)
        nc_dataset.createDimension('mxstr3', 80)
        nc_dataset.createDimension('nhdr', point_obs.nhdr)
        nc_dataset.createDimension('nobs', point_obs.nobs)
        #npbhdr = len(point_obs.hdr_prpt_typ)
        if 0 < point_obs.npbhdr:
            nc_dataset.createDimension('npbhdr', point_obs.npbhdr)
        nc_dataset.createDimension('nhdr_typ', point_obs.nhdr_typ)
        nc_dataset.createDimension('nhdr_sid', point_obs.nhdr_sid)
        nc_dataset.createDimension('nhdr_vld', point_obs.nhdr_vld)
        nc_dataset.createDimension('nobs_qty', point_obs.nobs_qty)
        nc_dataset.createDimension('obs_var_num', point_obs.nobs_var)

        type_for_string = 'S1'    # np.byte
        dims_hdr = ('nhdr',)
        dims_obs = ('nobs',)

        # Create header and observation variables
        var_hdr_typ = nc_dataset.createVariable('hdr_typ', np.int32, dims_hdr, fill_value=-9999)
        var_hdr_sid = nc_dataset.createVariable('hdr_sid', np.int32, dims_hdr, fill_value=-9999)
        var_hdr_vld = nc_dataset.createVariable('hdr_vld', np.int32, dims_hdr, fill_value=-9999)
        var_hdr_lat = nc_dataset.createVariable('hdr_lat', np.float32, dims_hdr, fill_value=-9999.)
        var_hdr_lon = nc_dataset.createVariable('hdr_lon', np.float32, dims_hdr, fill_value=-9999.)
        var_hdr_elv = nc_dataset.createVariable('hdr_elv', np.float32, dims_hdr, fill_value=-9999.)

        var_obs_qty = nc_dataset.createVariable('obs_qty', np.int32, dims_obs, fill_value=-9999)
        var_obs_hid = nc_dataset.createVariable('obs_hid', np.int32, dims_obs, fill_value=-9999)
        var_obs_vid = nc_dataset.createVariable('obs_vid', np.int32, dims_obs, fill_value=-9999)
        var_obs_lvl = nc_dataset.createVariable('obs_lvl', np.float32, dims_obs, fill_value=-9999.)
        var_obs_hgt = nc_dataset.createVariable('obs_hgt', np.float32, dims_obs, fill_value=-9999.)
        var_obs_val = nc_dataset.createVariable('obs_val', np.float32, dims_obs, fill_value=-9999.)

        if 0 == point_obs.npbhdr:
            var_hdr_prpt_typ = None
            var_hdr_irpt_typ = None
            var_hdr_inst_typ = None
        else:
            dims_npbhdr = ('npbhdr',)
            var_hdr_prpt_typ = nc_dataset.createVariable('hdr_prpt_typ', np.int32, dims_npbhdr, fill_value=-9999.)
            var_hdr_irpt_typ = nc_dataset.createVariable('hdr_irpt_typ', np.int32, dims_npbhdr, fill_value=-9999.)
            var_hdr_inst_typ = nc_dataset.createVariable('hdr_inst_typ', np.int32, dims_npbhdr, fill_value=-9999.)

        var_hdr_typ_table = nc_dataset.createVariable('hdr_typ_table', type_for_string, ('nhdr_typ','mxstr2'))
        var_hdr_sid_table = nc_dataset.createVariable('hdr_sid_table', type_for_string, ('nhdr_sid','mxstr2'))
        var_hdr_vld_table = nc_dataset.createVariable('hdr_vld_table', type_for_string, ('nhdr_vld','mxstr'))
        var_obs_qty_table = nc_dataset.createVariable('obs_qty_table', type_for_string, ('nobs_qty','mxstr'))
        var_obs_var_table = nc_dataset.createVariable('obs_var', type_for_string, ('obs_var_num','mxstr2'))
        var_obs_var_unit  = nc_dataset.createVariable('obs_unit', type_for_string, ('obs_var_num','mxstr2'))
        var_obs_var_desc  = nc_dataset.createVariable('obs_desc', type_for_string, ('obs_var_num','mxstr3'))

        # Set variables
        var_hdr_typ[:] = point_obs.hdr_typ[:]
        var_hdr_sid[:] = point_obs.hdr_sid[:]
        var_hdr_vld[:] = point_obs.hdr_vld[:]
        var_hdr_lat[:] = point_obs.hdr_lat[:]
        var_hdr_lon[:] = point_obs.hdr_lon[:]
        var_hdr_elv[:] = point_obs.hdr_elv[:]
        for i in range(0, point_obs.nhdr_typ):
            for j in range(0, len(point_obs.hdr_typ_table[i])):
                var_hdr_typ_table[i,j] = point_obs.hdr_typ_table[i][j]
        for i in range(0, point_obs.nhdr_sid):
            for j in range(0, len(point_obs.hdr_sid_table[i])):
                var_hdr_sid_table[i,j] = point_obs.hdr_sid_table[i][j]
        for i in range(0, point_obs.nhdr_vld): 
            for j in range(0, len(point_obs.hdr_vld_table[i])):
                var_hdr_vld_table[i,j] = point_obs.hdr_vld_table[i][j]
        if 0 < point_obs.npbhdr:
            var_hdr_prpt_typ[:] = point_obs.hdr_prpt_typ[:]
            var_hdr_irpt_typ[:] = point_obs.hdr_irpt_typ[:]
            var_hdr_inst_typ[:] = point_obs.hdr_inst_typ[:]

        var_obs_qty[:] = point_obs.obs_qty[:]
        var_obs_hid[:] = point_obs.obs_hid[:]
        var_obs_vid[:] = point_obs.obs_vid[:]
        var_obs_lvl[:] = point_obs.obs_lvl[:]
        var_obs_hgt[:] = point_obs.obs_hgt[:]
        var_obs_val[:] = point_obs.obs_val[:]
        for i in range(0, point_obs.nobs_var): 
            for j in range(0, len(point_obs.obs_var_table[i])):
                var_obs_var_table[i,j] = point_obs.obs_var_table[i][j]
            var_obs_var_unit[i] = "" if i >= len(point_obs.obs_var_unit) else point_obs.obs_var_unit[i] 
            var_obs_var_desc[i] = "" if i >= len(point_obs.obs_var_desc) else point_obs.obs_var_desc[i]
        for i in range(0, point_obs.nobs_qty): 
            for j in range(0, len(point_obs.obs_qty_table[i])):
                var_obs_qty_table[i,j] = point_obs.obs_qty_table[i][j]

        # Set variable attributes
        var_hdr_typ.long_name = "index of message type"
        var_hdr_sid.long_name = "index of station identification"
        var_hdr_vld.long_name = "index of valid time"
        var_hdr_lat.long_name = "latitude"
        var_hdr_lat.units = "degrees_north"
        var_hdr_lon.long_name = "longitude"
        var_hdr_lon.units = "degrees_east"
        var_hdr_elv.long_name = "elevation"
        var_hdr_elv.units = "meters above sea level (msl)"

        var_obs_qty.long_name = "index of quality flag"
        var_obs_hid.long_name = "index of matching header data"
        var_obs_vid.long_name = "index of BUFR variable corresponding to the observation type"
        var_obs_lvl.long_name = "pressure level (hPa) or accumulation interval (sec)"
        var_obs_hgt.long_name = "height in meters above sea level (msl)"
        var_obs_val.long_name = "observation value"
        var_hdr_typ_table.long_name = "message type"
        var_hdr_sid_table.long_name = "station identification"
        var_hdr_vld_table.long_name = "valid time"
        var_hdr_vld_table.units = "YYYYMMDD_HHMMSS UTC"
        var_obs_qty_table.long_name = "quality flag"
        var_obs_var_table.long_name = "variable names"
        var_obs_var_unit.long_name = "variable units"
        var_obs_var_desc.long_name = "variable descriptions"


def main(argv):
    if len(argv) != 1 and argv[1] != ARG_PRINT_DATA:
        netcdf_filename = argv[1]
        tmp_nc_name = 'tmp_met_point.nc'
        point_obs_data = nc_point_obs()
        point_obs_data.read_data(point_obs_data.get_nc_filename(netcdf_filename))
        met_point_data = point_obs_data.save_ncfile(tmp_nc_name)
        print(f'{get_prompt()} saved met_point_data to {tmp_nc_name}')
        met_point_data['met_point_data'] = point_obs_data
    
        if DO_PRINT_DATA or ARG_PRINT_DATA == argv[-1]:
            met_point_obs.print_point_data(met_point_data)

if __name__ == '__main__':
    start_time = datetime.now()
    main(sys.argv)
    run_time = datetime.now() - start_time
    print(f'{get_prompt()} Done python script {sys.argv[0]} took {run_time}')
