'''
Created on Nov 10, 2021

@author: hsoh
'''

import os
import sys
import time
import numpy as np
import netCDF4 as nc

from met_point_obs import met_point_obs, get_sample_point_obs, MET_PYTHON_OBS_ARGS

DO_PRINT_DATA = False
ARG_PRINT_DATA = 'show_data'

class nc_point_obs(met_point_obs):

    @staticmethod
    def get_num_array(dataset, var_name):
        nc_var = dataset.variables.get(var_name, None)
        return nc_var[:].filled(nc_var._FillValue if '_FillValue' in nc_var.ncattrs() else -9999) if nc_var else []
    
    @staticmethod
    def get_ncbyte_array_to_str(nc_var):
        nc_str_data = nc_var[:]
        if nc_var.datatype.name == 'bytes8':
            nc_str_data = [ str(s.compressed(),"utf-8") for s in nc_var[:]]
        return nc_str_data

    @staticmethod
    def get_string_array(dataset, var_name):
        nc_var = dataset.variables.get(var_name, None)
        return nc_point_obs.get_ncbyte_array_to_str(nc_var) if nc_var else []
    
    #def convert(self):
    #    pass
    
    def read_nectdf(self, nc_filename):
        if os.path.exists(nc_filename):
            dataset = nc.Dataset(nc_filename, 'r')
            # Header
            self.hdr_typ = dataset['hdr_typ'][:]
            self.hdr_sid = dataset['hdr_sid'][:]
            self.hdr_vld = dataset['hdr_vld'][:]
            self.hdr_lat = dataset['hdr_lat'][:]
            self.hdr_lon = dataset['hdr_lon'][:]
            self.hdr_elv = dataset['hdr_elv'][:]
            self.hdr_typ_table = self.get_string_array(dataset, 'hdr_typ_table')
            self.hdr_sid_table = self.get_string_array(dataset, 'hdr_sid_table')
            self.hdr_vld_table = self.get_string_array(dataset, 'hdr_vld_table')
            
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
            self.nobs = 0
            self.hdr_sid = dataset['hdr_sid'][:]
            #self.obs_qty = self.get_num_array(dataset, 'obs_qty')
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
                self.obs_var_table = self.get_string_array(dataset, 'obs_var')
            if nc_var:
                self.obs_vid = np.array(nc_var[:])
            
            self.obs_qty_rwo = dataset['obs_qty'][:]

            self.obs_qty_table = self.get_string_array(dataset, 'obs_qty_table')
        else:
            print("==ERROR== netcdf file ({f}) does not exist".format(f=nc_filename))
    


perf_start_time = time.time()
perf_start_counter = time.perf_counter_ns()

point_obs_data = None
if len(sys.argv) == 1:
    point_obs_data = get_sample_point_obs()
else:
    netcdf_filename = sys.argv[1]
    if os.path.exists(netcdf_filename):
        point_obs_data = nc_point_obs()
        point_obs_data.read_nectdf(netcdf_filename)
    elif met_point_obs.is_python_script(netcdf_filename):
        python_script = met_point_obs.get_python_script(netcdf_filename)
        python_args = []
        if len(sys.argv) > 2:
            if ARG_PRINT_DATA == sys.argv[-1]:
                python_args = sys.argv[2:-1]
            else:
                python_args = sys.argv[2:]
        python_args_string = " ".join(python_args) if 0 < len(python_args) else ''
        os.environ[MET_PYTHON_OBS_ARGS] = python_args_string
        #os.system(python_command)
        exec(open(python_script).read())
        print('------')

        if point_obs_data is None:
            print("PYTHON: Fail to get point_obs_data by calling {s} from {m}".format(
                    s=python_script, m=sys.argv[0]))
    if ARG_PRINT_DATA == sys.argv[-1]:
        DO_PRINT_DATA = True
    
    point_obs_data = nc_point_obs()
    point_obs_data.read_nectdf(netcdf_filename)

if point_obs_data is not None:
    met_point_data = point_obs_data.get_point_data() 
    met_point_data['met_point_data'] = point_obs_data

    if DO_PRINT_DATA:
        met_point_obs.print_point_data(met_point_data)

perf_end_time = time.time()
perf_end_counter = time.perf_counter_ns()
perf_duration = perf_end_time - perf_start_time
perf_duration_counter = (perf_end_counter - perf_start_counter) / 1000000000

print('Done python script {s} Took walltime: {t1} & perf: {t2} seconds'.format(s=sys.argv[0], t1=perf_duration, t2=perf_duration_counter))
