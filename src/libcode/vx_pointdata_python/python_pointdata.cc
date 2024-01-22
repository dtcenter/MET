// *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*
// ** Copyright UCAR (c) 1992 - 2023
// ** University Corporation for Atmospheric Research (UCAR)
// ** National Center for Atmospheric Research (NCAR)
// ** Research Applications Lab (RAL)
// ** P.O.Box 3000, Boulder, Colorado, 80307-3000, USA
// *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*

////////////////////////////////////////////////////////////////////////

#include <iostream>
#include <vector>

#include "observation.h"
#include "vx_python3_utils.h"
#include "python_pointdata.h"
#include "pointdata_from_array.h"
#include "vx_util.h"
#include "vx_python3_utils.h"

#include "global_python.h"
#include "wchar_argv.h"

////////////////////////////////////////////////////////////////////////

extern GlobalPython GP;   //  this needs external linkage

////////////////////////////////////////////////////////////////////////


static const char * user_ppath            = 0;

static const char write_tmp_nc         [] = "MET_BASE/python/pyembed/write_tmp_point_nc.py";

static const char read_tmp_nc          [] = "pyembed.read_tmp_point_nc";   //  NO ".py" suffix

////////////////////////////////////////////////////////////////////////


static bool tmp_nc_point_obs(const char * script_name, int user_script_argc,
                             char ** user_script_argv, MetPointDataPython &met_pd_out,
                             MaskFilters *filters);

static bool straight_python_point_data(const char * script_name,
                                      int script_argc, char ** script_argv,
                                      MetPointDataPython &met_pd_out,
                                      MaskFilters *filters);

bool process_point_data(PyObject *module_obj, MetPointDataPython &met_pd_out);
bool process_point_data_list(PyObject *python_obj, MetPointDataPython &met_pd_out,
                             MaskFilters *filters);

////////////////////////////////////////////////////////////////////////

void check_header_data(MetPointHeader *header_data, const char *caller) {

   if (header_data->typ_idx_array.n() == 0) {
      mlog << Error << "\n" << caller
           << "The hdr_typ is empty. Please check if python input is processed properly\n\n";
      exit (1);
   }
   if (header_data->sid_idx_array.n() == 0) {
      mlog << Error << "\n" << caller
           << "The hdr_sid is empty. Please check if python input is processed properly\n\n";
      exit (1);
   }
   if (header_data->vld_idx_array.n() == 0) {
      mlog << Error << "\n" << caller
           << "The hdr_vld is empty. Please check if python input is processed properly\n\n";
      exit (1);
   }
   if (header_data->lat_array.n() == 0) {
      mlog << Error << "\n" << caller
           << "The hdr_lat is empty. Please check if python input is processed properly\n\n";
      exit (1);
   }
   if (header_data->lon_array.n() == 0) {
      mlog << Error << "\n" << caller
           << "The hdr_lon is empty. Please check if python input is processed properly\n\n";
      exit (1);
   }
   if (header_data->elv_array.n() == 0) {
      mlog << Error << "\n" << caller
           << "The hdr_elv is empty. Please check if python input is processed properly\n\n";
      exit (1);
   }

   if (header_data->typ_array.n() == 0) {
      mlog << Error << "\n" << caller
           << "The hdr_typ_table is empty. Please check if python input is processed properly\n\n";
      exit (1);
   }
   if (header_data->sid_array.n() == 0) {
      mlog << Error << "\n" << caller
           << "The hdr_sid_table is empty. Please check if python input is processed properly\n\n";
      exit (1);
   }
   if (header_data->vld_array.n() == 0) {
      mlog << Error << "\n" << caller
           << "The hdr_vld_table is empty. Please check if python input is processed properly\n\n";
      exit (1);
   }
}

////////////////////////////////////////////////////////////////////////

void check_obs_data(MetPointObsData *obs_data, bool use_var_id, const char *caller) {

   if (obs_data->qty_names.n() == 0) {
      mlog << Error << "\n" << caller
           << "The obs_qty_table is empty. Please check if python input is processed properly\n\n";
      exit (1);
   }
   if (use_var_id && obs_data->var_names.n() == 0) {
      mlog << Error << "\n" << caller
           << "The obs_var_table is empty. Please check if python input is processed properly\n\n";
      exit (1);
   }

}

////////////////////////////////////////////////////////////////////////


PyObject *get_python_object(PyObject *module_obj, const char *python_var_name)
{

   //
   //   get the namespace for the module (as a dictionary)
   //

   PyObject *module_dict_obj = PyModule_GetDict (module_obj);

   //
   //  get handles to the objects of interest from the module_dict
   //

   PyObject *python_met_point_data = PyDict_GetItemString (module_dict_obj, python_var_name);

   return python_met_point_data;
}


////////////////////////////////////////////////////////////////////////


static void set_str_array_from_python(PyObject *python_data, const char *python_key, StringArray *out) {
   const char *method_name = "set_met_array_from_python(StringArray *) -> ";
   PyObject *str_array_obj = PyDict_GetItemString (python_data, python_key);
   if (str_array_obj) {
      pointdata_from_str_array(str_array_obj, out);
      mlog << Debug(7) << method_name
           << "get the point data for " << python_key << " from python object\n";
   }
   else {
      mlog << Error << "\n" << method_name
           << "error getting member (" << python_key << ") from python object\"\n\n";
      exit (1);
   }
}


////////////////////////////////////////////////////////////////////////


bool python_point_data(const char * script_name, int script_argc, char ** script_argv,
                       MetPointDataPython &met_pd_out, MaskFilters *filters)

{

bool status = false;

if ( user_ppath == 0 ) user_ppath = getenv(user_python_path_env);

if ( user_ppath != 0 ) {    //  do_tmp_nc = true;

   status = tmp_nc_point_obs(script_name, script_argc, script_argv,
                             met_pd_out, filters);
}
else {

   status = straight_python_point_data(script_name, script_argc, script_argv,
                                       met_pd_out, filters);
}

return ( status );

}

////////////////////////////////////////////////////////////////////////

bool process_point_data(PyObject *python_met_point_data,
                        MetPointDataPython &met_pd_out)

{

int int_value;
PyObject *python_value;
ConcatString cs, user_dir, user_base;
const char *method_name = "process_point_data -> ";
const char *method_name_s = "process_point_data()";

   //
   //  get handles to the objects of interest from the module_dict
   //

python_value = PyDict_GetItemString (python_met_point_data, python_use_var_id);

bool use_var_id = pyobject_as_bool(python_value);
met_pd_out.set_use_var_id(use_var_id);
mlog << Debug(9) << method_name << "use_var_id: \"" << use_var_id
     << "\" from python.  is_using_var_id(): " << met_pd_out.is_using_var_id() << "\n";

python_value = PyDict_GetItemString (python_met_point_data, python_key_nhdr);

int_value = pyobject_as_int(python_value);
if (int_value == 0) {
   mlog << Error << "\n" << method_name
        << "The header is empty. Please check if python input exists\n\n";
   exit (1);
}
met_pd_out.set_hdr_cnt(int_value);

python_value = PyDict_GetItemString (python_met_point_data, python_key_nobs);
int_value = pyobject_as_int(python_value);
if (int_value == 0) {
   mlog << Error << "\n" << method_name
        << "The point observation data is empty. Please check if python input is processed properly\n\n";
   exit (1);
}

met_pd_out.allocate(int_value);

MetPointObsData *obs_data = met_pd_out.get_point_obs_data();
MetPointHeader *header_data = met_pd_out.get_header_data();

   //  look up the data array variable name from the dictionary

   set_array_from_python(python_met_point_data, numpy_array_hdr_typ, &header_data->typ_idx_array);
   set_array_from_python(python_met_point_data, numpy_array_hdr_sid, &header_data->sid_idx_array);
   set_array_from_python(python_met_point_data, numpy_array_hdr_vld, &header_data->vld_idx_array);
   set_array_from_python(python_met_point_data, numpy_array_hdr_lat, &header_data->lat_array);
   set_array_from_python(python_met_point_data, numpy_array_hdr_lon, &header_data->lon_array);
   set_array_from_python(python_met_point_data, numpy_array_hdr_elv, &header_data->elv_array);

   set_str_array_from_python(python_met_point_data, numpy_array_hdr_typ_table, &header_data->typ_array);
   set_str_array_from_python(python_met_point_data, numpy_array_hdr_sid_table, &header_data->sid_array);
   set_str_array_from_python(python_met_point_data, numpy_array_hdr_vld_table, &header_data->vld_array);

   set_array_from_python(python_met_point_data, numpy_array_prpt_typ_table, &header_data->prpt_typ_array, false);
   set_array_from_python(python_met_point_data, numpy_array_irpt_typ_table, &header_data->irpt_typ_array, false);
   set_array_from_python(python_met_point_data, numpy_array_inst_typ_table, &header_data->inst_typ_array, false);

   check_header_data(header_data, method_name);

   set_array_from_python(python_met_point_data, numpy_array_obs_qty, obs_data->obs_qids);
   set_array_from_python(python_met_point_data, numpy_array_obs_hid, obs_data->obs_hids);
   set_array_from_python(python_met_point_data, numpy_array_obs_vid, obs_data->obs_ids);
   set_array_from_python(python_met_point_data, numpy_array_obs_lvl, obs_data->obs_lvls);
   set_array_from_python(python_met_point_data, numpy_array_obs_hgt, obs_data->obs_hgts);
   set_array_from_python(python_met_point_data, numpy_array_obs_val, obs_data->obs_vals);

   set_str_array_from_python(python_met_point_data, numpy_array_obs_qty_table, &obs_data->qty_names);
   set_str_array_from_python(python_met_point_data, numpy_array_obs_var_table, &obs_data->var_names);

   check_obs_data(obs_data, use_var_id, method_name);

   if(mlog.verbosity_level()>=point_data_debug_level) {
      print_met_data(met_pd_out.get_point_obs_data(),
                     met_pd_out.get_header_data(), method_name_s);
   }

   //
   //  done
   //

return ( true );

}


////////////////////////////////////////////////////////////////////////

bool process_point_data_list(PyObject *python_point_data, MetPointDataPython &met_pd_out,
                             MaskFilters *filters)
{

   bool use_var_id;
   Observation obs;
   time_t vld_time;
   int hid, vid, qid, sid, typ_idx, vld_idx;
   double prev_lat, prev_lon, prev_elv, prev_vld, prev_typ, prev_sid;
   Python3_List list(python_point_data);
   const char *method_name = "process_point_data_list -> ";
   const char *method_name_s = "process_point_data_list()";

   int obs_cnt = list.size();
   if (obs_cnt == 0) {
      mlog << Error << "\n" << method_name
           << "The point observation data is empty. Please check if python input is processed properly\n\n";
      exit (1);
   }

   //
   //  initialize use_var_id to false
   //

   use_var_id = false;
   hid = -1;   // starts from -1 to be 0 for the first header
   prev_lat = prev_lon = prev_elv = bad_data_double;
   prev_vld = prev_typ = prev_sid = bad_data_double;

   met_pd_out.allocate(obs_cnt);
   MetPointHeader *header_data = met_pd_out.get_header_data();
   MetPointObsData *obs_data = met_pd_out.get_point_obs_data();

   for (int j=0; j<obs_cnt; ++j)  {
      std::string str_data;
      PyObject *py_value = list[j];
      if ( ! PyList_Check(py_value) )  {
         mlog << Error << "\n" << method_name
              << "non-list object found in main list!\n\n";
         exit ( 1 );
      }

      obs.set(py_value);
      double lat = obs.getLatitude();
      double lon = obs.getLongitude();
      double elv = obs.getElevation();
      if(filters) {
         if (filters->is_filtered(lat, lon)) continue;
         if (filters->is_filtered_sid(obs.getStationId().c_str())) continue;
         if (filters->is_filtered_typ(obs.getHeaderType().c_str())) continue;
      }

      // get message type index
      str_data = obs.getHeaderType();
      if ( !header_data->typ_array.has(str_data, typ_idx) )  {
         header_data->typ_array.add(str_data);
         header_data->typ_array.has(str_data, typ_idx);
      }

      // get station ID index
      str_data = obs.getStationId();
      if ( !header_data->sid_array.has(str_data, sid) )  {
         header_data->sid_array.add(str_data);
         header_data->sid_array.has(str_data, sid);
      }

      // get valid time index
      vld_time = obs.getValidTime();
      if ( !header_data->vld_num_array.has(vld_time, vld_idx) )  {
         header_data->vld_num_array.add(vld_time);
         header_data->vld_num_array.has(vld_time, vld_idx);
      }

      if (!is_eq(prev_lat, lat) || !is_eq(prev_lon, lon) || !is_eq(prev_elv, elv)
          || !is_eq(prev_sid, sid) || !is_eq(prev_typ, typ_idx)
          || !is_eq(prev_vld, vld_idx)) {
         header_data->lat_array.add(lat);
         header_data->lon_array.add(lon);
         header_data->elv_array.add(elv);
         header_data->sid_idx_array.add(sid);
         header_data->typ_idx_array.add(typ_idx);
         header_data->vld_idx_array.add(vld_idx);
         header_data->vld_array.add(obs.getValidTimeString());

         prev_lat = lat;
         prev_lon = lon;
         prev_elv = elv;
         prev_sid = sid;
         prev_typ = typ_idx;
         prev_vld = vld_idx;
         hid++;
      }
      obs_data->obs_hids[j] = hid;

      // get the observation variable code
      str_data = obs.getVarName();
      if ( use_var_id || !is_number(str_data.c_str()) )  {
         use_var_id = true;
         // update the list of variable names
         if ( !obs_data->var_names.has(str_data, vid) )  {
            obs_data->var_names.add(str_data);
            obs_data->var_names.has(str_data, vid);
         }
      }
      else {
         vid = atoi(obs.getVarName().c_str());
      }
      obs_data->obs_ids[j] = vid;
      obs.setVarCode(vid);

      // get the quality flag index
      str_data = obs.getQualityFlag();
      if ( !obs_data->qty_names.has(str_data, qid) )  {
         obs_data->qty_names.add(str_data);
         obs_data->qty_names.has(str_data, qid);
      }
      obs_data->obs_qids[j] = qid;
      obs_data->obs_lvls[j] = obs.getPressureLevel();
      obs_data->obs_hgts[j] = obs.getHeight();
      obs_data->obs_vals[j] = obs.getValue();

   }   //  for j

   int h_cnt = hid + 1; // hid starts with -1
   if (h_cnt < 0) {
      mlog << Error << "\n" << method_name
           << "The header is empty. Please check the python script and input\n\n";
      exit (1);
   }
   met_pd_out.set_hdr_cnt(h_cnt);

   met_pd_out.set_use_var_id(use_var_id);
   mlog << Debug(9) << method_name << "use_var_id: \"" << use_var_id
        << "\" from python.  is_using_var_id(): " << met_pd_out.is_using_var_id() << "\n";

   check_obs_data(obs_data, use_var_id, method_name);
   check_header_data(header_data, method_name);

   if(mlog.verbosity_level()>=point_data_debug_level) {
      print_met_data(met_pd_out.get_point_obs_data(),
                     met_pd_out.get_header_data(), method_name_s);
   }

   //
   //  done
   //

   return ( true );

}


////////////////////////////////////////////////////////////////////////


bool straight_python_point_data(const char * script_name, int script_argc, char ** script_argv,
                                MetPointDataPython &met_pd_out, MaskFilters *filters)
{

int int_value;
PyObject *module_obj;
ConcatString cs, user_dir, user_base;
const char *method_name = "straight_python_point_data -> ";


cs        = script_name;
user_dir  = cs.dirname();
user_base = cs.basename();

Wchar_Argv wa;

wa.set(script_argc, script_argv);

   //
   //  if the global python object has already been initialized,
   //  we need to reload the module
   //

bool do_reload = GP.is_initialized;

GP.initialize();

   //
   //   start up the python interpreter
   //

if ( PyErr_Occurred() )  {

   PyErr_Print();

   mlog << Warning << "\n" << method_name
        << "an error occurred initializing python\n\n";

   return ( false );

}


mlog << Debug(3) << "Running MET compile time python instance ("
     << MET_PYTHON_BIN_EXE << ") to run user's python script ("
     << script_name << ").\n";

   //
   //  set the arguments
   //

run_python_string("import os");
run_python_string("import sys");

ConcatString command;

command << cs_erase
        << "sys.path.append(\""
        << user_dir
        << "\")";

run_python_string(command.text());

if ( script_argc > 0 )  {

   PySys_SetArgv (wa.wargc(), wa.wargv());

}

   //
   //  import the python script as a module
   //

module_obj = PyImport_ImportModule (user_base.c_str());

   //
   //  if needed, reload the module
   //

if ( do_reload )  {

   module_obj = PyImport_ReloadModule (module_obj);

}

if ( PyErr_Occurred() )  {

   PyErr_Print();

   mlog << Warning << "\n" << method_name
        << "an error occurred importing module \""
        << script_name << "\"\n\n";

   return false;

}

if ( ! module_obj )  {

   mlog << Warning << "\n" << method_name
        << "error running python script \""
        << script_name << "\"\n\n";

   return false;

}

bool result = false;
PyObject *met_point_data = get_python_object(module_obj, python_key_point_data);
if ( met_point_data && met_point_data != &_Py_NoneStruct) {
   result = process_point_data(met_point_data, met_pd_out);
}
else {
   PyObject *point_data = get_python_object(module_obj, python_key_point_data_list);
   if ( point_data && point_data != &_Py_NoneStruct)
      result = process_point_data_list(point_data, met_pd_out, filters);
   else {
      mlog << Warning << "\n" << method_name
           << "no \"" << python_key_point_data << "\" and \""
           << python_key_point_data_list << "\" from "
           << script_name << "\"\n\n";
   }
}

return result;
}


////////////////////////////////////////////////////////////////////////


bool tmp_nc_point_obs(const char * user_script_name, int user_script_argc,
                      char ** user_script_argv, MetPointDataPython &met_pd_out,
                      MaskFilters *filters)

{

int j;
int status;
ConcatString command;
ConcatString path;
ConcatString tmp_nc_path;
const char * tmp_dir = nullptr;
Wchar_Argv wa;
const char *method_name = "tmp_nc_point_obs() -> ";

   //
   //  if the global python object has already been initialized,
   //  we need to reload the module
   //

bool do_reload = GP.is_initialized;

GP.initialize();

   //
   //   start up the python interpreter
   //

if ( PyErr_Occurred() )  {

   PyErr_Print();

   mlog << Warning << "\n" << method_name
        << "an error occurred initializing python\n\n";

   return ( false );

}

run_python_string("import sys");
command << cs_erase
        << "sys.path.append(\""
        << replace_path(python_dir)
        << "\")";
run_python_string(command.text());
mlog << Debug(3) << method_name << "added python path ("
     << replace_path(python_dir) << ") to python interpreter\n";

//setenv(env_PYTHONPATH, python_dir.c_str(),1);

mlog << Debug(3) << "Running user-specified python instance (MET_PYTHON_EXE=" << user_ppath
     << ") to run user's python script (" << user_script_name << ").\n";


tmp_dir = getenv ("MET_TMP_DIR");

if ( ! tmp_dir )  tmp_dir = default_tmp_dir;

path << cs_erase
     << tmp_dir << '/'
     << tmp_nc_base_name;

tmp_nc_path = make_temp_file_name(path.text(), 0);

command << cs_erase
        << user_ppath                    << ' '    //  user's path to python
        << replace_path(write_tmp_nc)    << ' '    //  write_tmp_nc.py
        << tmp_nc_path                   << ' '    //  tmp_nc output filename
        << user_script_name;                       //  user's script name

for (j=1; j<user_script_argc; ++j)  {   //  j starts at one, here

   command << ' ' << user_script_argv[j];

}

mlog << Debug(4) << "Writing temporary Python point data file:\n\t"
     << command << "\n";

status = system(command.text());

if ( status )  {

   mlog << Error << "\n" << method_name
        << "command \"" << command.text() << "\" failed ... status = "
        << status << "\n\n";

   exit ( 1 );

}

   //
   //  set the arguments
   //

StringArray a;

a.add(read_tmp_nc);

a.add(tmp_nc_path);

wa.set(a);

PySys_SetArgv (wa.wargc(), wa.wargv());

mlog << Debug(4) << "Reading temporary Python point data file: "
     << tmp_nc_path << "\n";

   //
   //  import the python wrapper script as a module
   //

path = get_short_name(read_tmp_nc);

PyObject * module_obj = PyImport_ImportModule (path.text());

   //
   //  if needed, reload the module
   //

if ( do_reload )  {

   module_obj = PyImport_ReloadModule (module_obj);

}

if ( PyErr_Occurred() )  {

   PyErr_Print();

   mlog << Warning << "\n" << method_name
        << "an error occurred importing module "
        << '\"' << path << "\"\n\n";

   return ( false );

}

if ( ! module_obj )  {

   mlog << Warning << "\n" << method_name
        << "error running python script\n\n";

   return ( false );

}

   //
   //  read the tmp_nc file
   //

   //
   //   get the namespace for the module (as a dictionary)
   //


PyObject *met_point_data = get_python_object(module_obj, python_key_point_data);
if ( met_point_data ) {
   process_point_data(met_point_data, met_pd_out);
}
else {
   PyObject *point_data = get_python_object(module_obj, python_key_point_data_list);
   process_point_data_list(point_data, met_pd_out, filters);
}

   //
   //  cleanup
   //

remove_temp_file(tmp_nc_path);

   //
   //  done
   //

return ( true );

}


////////////////////////////////////////////////////////////////////////

void print_met_data(MetPointObsData *obs_data, MetPointHeader *header_data,
                    const char *caller, int debug_level) {
   int log_count, count;
   const int min_count = 20;
   const char *method_name = "print_met_data() ";

   mlog << Debug(debug_level) << "\n" << method_name << "by " << caller << "\n"
        << "       obs_data.obs_cnt    = " << obs_data->obs_cnt << "\n"
        << "    header_data.hdr_count  = " << header_data->hdr_count << " type="
        << header_data->typ_idx_array.n()  << ", sid="
        << header_data->sid_idx_array.n()  << ", valid="
        << header_data->vld_idx_array.n()  << ", lat="
        << header_data->lat_array.n()      << ", lon="
        << header_data->lon_array.n()      << ", elv="
        << header_data->elv_array.n()      << ",  message_type="
        << header_data->typ_array.n()      << ", station_id="
        << header_data->sid_array.n()      << ", valid_time="
        << header_data->vld_array.n()      << ", prpt="
        << header_data->prpt_typ_array.n() << ", irpt="
        << header_data->irpt_typ_array.n() << ", inst="
        << header_data->inst_typ_array.n() << " &header_data=" <<  header_data
        << "\n";

   log_count = (header_data->hdr_count > min_count) ? min_count : header_data->hdr_count;
   mlog << Debug(debug_level) << method_name
        << "header_data: message_type,station_id,time_time,lat,lon.elv\n";
   for (int idx=0; idx<log_count; idx++) {
      mlog << Debug(debug_level)
           << "  header_data[" << idx << "] = "
           << header_data->typ_idx_array[idx] << ", "
           << header_data->sid_idx_array[idx] << ", "
           << header_data->vld_idx_array[idx] << ", "
           << header_data->lat_array[idx] << ", "
           << header_data->lon_array[idx] << ", "
           << header_data->elv_array[idx] << "\n";
   }
   if (header_data->hdr_count > log_count) {
      log_count = header_data->hdr_count - min_count;
      if (log_count < min_count) log_count = min_count;
      else mlog << Debug(debug_level)
                << "  header_data[...] = ...\n";
      for (int idx=log_count; idx<header_data->hdr_count; idx++) {
         mlog << Debug(debug_level)
              << "  header_data[" << idx << "] = "
              << header_data->typ_idx_array[idx] << ", "
              << header_data->sid_idx_array[idx] << ", "
              << header_data->vld_idx_array[idx] << ", "
              << header_data->lat_array[idx] << ", "
              << header_data->lon_array[idx] << ", "
              << header_data->elv_array[idx] << "\n";
      }
   }
   if (header_data->typ_array.n() > 0) {
      mlog << Debug(debug_level) << "\n";
      count = header_data->typ_array.n();
      for (int idx=0; idx<count; idx++)
         mlog << Debug(debug_level)
              << "  message_type[" << idx << "] = " << header_data->typ_array[idx] << "\n";
   }
   else mlog << Debug(debug_level)
              << "     typ_array is empty!!!\n";
   if (header_data->sid_array.n() > 0) {
      mlog << Debug(debug_level) << "\n";
      count = header_data->sid_array.n();
      log_count = (count > min_count) ? min_count : count;
      for (int idx=0; idx<log_count; idx++)
         mlog << Debug(debug_level)
              << "    station_id[" << idx << "] = " << header_data->sid_array[idx] << "\n";
      if (count > log_count) {
         log_count = count - min_count;
         if (log_count < min_count) log_count = min_count;
         else mlog << Debug(debug_level)
                   << "    station_id[...] = ...\n";
         for (int idx=log_count; idx<count; idx++)
            mlog << Debug(debug_level)
                 << "    station_id[" << idx << "] = " << header_data->sid_array[idx] << "\n";
      }
   }
   else mlog << Debug(debug_level)
              << "     s_array is empty!!!\n";
   if (header_data->vld_array.n() > 0) {
      mlog << Debug(debug_level) << "\n";
      count = header_data->vld_array.n();
      log_count = (count > min_count) ? min_count : count;
      for (int idx=0; idx<log_count; idx++)
         mlog << Debug(debug_level)
              << "    valid time[" << idx << "] = " << header_data->vld_array[idx] << "\n";
      if (count > log_count) {
         log_count = count - min_count;
         if (log_count < min_count) log_count = min_count;
         else mlog << Debug(debug_level)
                  << "    valid time[...] = ...\n";
         for (int idx=log_count; idx<count; idx++)
            mlog << Debug(debug_level)
                 << "    valid time[" << idx << "] = " << header_data->vld_array[idx] << "\n";
      }
   }
   else mlog << Debug(debug_level)
              << "     typ_array is empty!!!\n";
   if (header_data->prpt_typ_array.n() > 0) {
      mlog << Debug(debug_level) << "\n";
      for (int idx=0; idx<header_data->prpt_typ_array.n(); idx++)
         mlog << Debug(debug_level)
              << "     prpt_type[" << idx << "] = " << header_data->prpt_typ_array[idx] << "\n";
   }
   if (header_data->irpt_typ_array.n() > 0) {
      mlog << Debug(debug_level) << "\n";
      for (int idx=0; idx<header_data->irpt_typ_array.n(); idx++)
         mlog << Debug(debug_level)
              << "     irpt_type[" << idx << "] = " << header_data->irpt_typ_array[idx] << "\n";
   }
   if (header_data->inst_typ_array.n() > 0) {
      mlog << Debug(debug_level) << "\n";
      for (int idx=0; idx<header_data->inst_typ_array.n(); idx++)
         mlog << Debug(debug_level)
              << "     inst_type[" << idx << "] = " << header_data->inst_typ_array[idx] << "\n";
   }

   log_count = (obs_data->obs_cnt > min_count) ? min_count : obs_data->obs_cnt;
   mlog << Debug(debug_level) << "\n" << method_name
        << "obs_data: hid,vid.level,height,value,qty\n";
   for (int idx=0; idx<log_count; idx++) {
      mlog << Debug(debug_level)
           << "     obs_data[" << idx << "] = "
           << obs_data->obs_hids[idx] << ", "
           << obs_data->obs_ids[idx]  << ", "
           << obs_data->obs_lvls[idx] << ", "
           << obs_data->obs_hgts[idx] << ", "
           << obs_data->obs_vals[idx] << ", "
           << obs_data->obs_qids[idx] << "\n";
   }
   if (obs_data->obs_cnt > log_count) {
      log_count = obs_data->obs_cnt - min_count;
      if (log_count < min_count) log_count = min_count;
      else mlog << Debug(debug_level)
                << "     obs_data[...] = ...\n";
      for (int idx=log_count; idx<obs_data->obs_cnt; idx++) {
         mlog << Debug(debug_level)
              << "     obs_data[" << idx << "] = "
              << obs_data->obs_hids[idx] << ", "
              << obs_data->obs_ids[idx]  << ", "
              << obs_data->obs_lvls[idx] << ", "
              << obs_data->obs_hgts[idx] << ", "
              << obs_data->obs_vals[idx] << ", "
              << obs_data->obs_qids[idx] << "\n";
      }
   }

   if (obs_data->var_names.n() > 0) {
      mlog << Debug(debug_level) << "\n";
      for (int idx=0; idx<obs_data->var_names.n(); idx++)
         mlog << Debug(debug_level)
              << "     var_names[" << idx << "] = " << obs_data->var_names[idx] << "\n";
   }
   else mlog << Debug(debug_level)
              << "     var_names is empty!!!\n";
   if (obs_data->qty_names.n() > 0) {
      mlog << Debug(debug_level) << "\n";
      for (int idx=0; idx<obs_data->qty_names.n(); idx++)
         mlog << Debug(debug_level)
              << "     qty_names[" << idx << "] = " << obs_data->qty_names[idx] << "\n";
   }
   else mlog << Debug(debug_level)
              << "     qty_names is empty!!!\n";

   mlog << Debug(debug_level) << "Done " << method_name << "by " << caller << "\n\n";

}

////////////////////////////////////////////////////////////////////////
