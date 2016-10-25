// *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*
// ** Copyright UCAR (c) 1992 - 2016
// ** University Corporation for Atmospheric Research (UCAR)
// ** National Center for Atmospheric Research (NCAR)
// ** Research Applications Lab (RAL)
// ** P.O.Box 3000, Boulder, Colorado, 80307-3000, USA
// *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*


////////////////////////////////////////////////////////////////////////

using namespace std;

#include <string.h>

#include <netcdf.hh>

#include "vx_log.h"
#include "nc_utils.h"
#include "util_constants.h"
#include "vx_cal.h"

////////////////////////////////////////////////////////////////////////

static const char  level_att_name         [] = "level";
static const char  units_att_name         [] = "units";
static const char  missing_value_att_name [] = "missing_value";
static const char  fill_value_att_name    [] = "_FillValue";

////////////////////////////////////////////////////////////////////////

bool get_var_att(const NcVar *var, const ConcatString &att_name,
                 ConcatString &att_val) {
   int i, n;
   NcAtt *att = (NcAtt *) 0;
   bool status = false;

   // Initialize
   att_val.clear();

   n = var->num_atts();

   // Loop through the attributes looking for a match
   for(i=0; i<n; i++) {

      att = var->get_att(i);

      // Look for a match
      if(strcmp(att_name, att->name()) == 0) {
         att_val << att->as_string(0);
         status = true;
         break;
      }
   }

   return(status);
}


////////////////////////////////////////////////////////////////////////
bool get_var_att(const NcVar *var, const ConcatString &att_name,
                 int &att_val, bool exit_on_error) {
   NcAtt *att = (NcAtt *) 0;
   bool status = true;

   // Initialize
   att_val = bad_data_int;
   
   //
   // Retrieve the NetCDF variable attribute.
   //
   if(!(att = var->get_att(att_name)) || !att->is_valid()) {
      status = false;
      mlog << Error << "\nget_var_att(float) -> "
           << "can't read attribute \"" << att_name
           << "\" from \"" << var->name() << "\" variable.\n\n";
      if (exit_on_error) exit(1);
   }
   att_val = att->as_int(0);

   return(status);
}


////////////////////////////////////////////////////////////////////////
bool get_var_att(const NcVar *var, const ConcatString &att_name,
                 float &att_val, bool exit_on_error) {
   NcAtt *att = (NcAtt *) 0;
   bool status = true;

   // Initialize
   att_val = bad_data_float;
   
   //
   // Retrieve the NetCDF variable attribute.
   //
   if(!(att = var->get_att(att_name)) || !att->is_valid()) {
      status = false;
      mlog << Error << "\nget_var_att(float) -> "
           << "can't read attribute \"" << att_name
           << "\" from \"" << var->name() << "\" variable.\n\n";
      if (exit_on_error) exit(1);
   }
   att_val = att->as_float(0);

   return(status);
}


////////////////////////////////////////////////////////////////////////

bool get_var_att_double(const NcVar *var, const ConcatString &att_name,
                        double &att_val) {
   int i, n;
   NcAtt *att = (NcAtt *) 0;
   bool status = false;

   // Initialize
   att_val = bad_data_double;

   n = var->num_atts();

   // Loop through the attributes looking for a match
   for(i=0; i<n; i++) {

      att = var->get_att(i);

      // Look for a match
      if(strcmp(att_name, att->name()) == 0) {
         att_val = att->as_double(0);
         status = true;
         break;
      }
   }

   return(status);
}

////////////////////////////////////////////////////////////////////////

bool get_var_units(const NcVar *var, ConcatString &att_val) {

   return(get_var_att(var, units_att_name, att_val));
}

////////////////////////////////////////////////////////////////////////

bool get_var_level(const NcVar *var, ConcatString &att_val) {

   return(get_var_att(var, level_att_name, att_val));
}

////////////////////////////////////////////////////////////////////////

double get_var_missing_value(const NcVar *var) {
   double v;

   if(!get_var_att_double(var, missing_value_att_name, v)) {
      v = bad_data_double;
   }

   return(v);
}

////////////////////////////////////////////////////////////////////////

double get_var_fill_value(const NcVar *var) {
   double v;

   if(!get_var_att_double(var, fill_value_att_name, v)) {
      v = bad_data_double;
   }

   return(v);
}

////////////////////////////////////////////////////////////////////////

char get_char_val(NcFile * nc, const char * var_name, const int index) {
   NcVar * var = (NcVar *) 0;

   var = nc->get_var(var_name);
   return (get_char_val(var, index));
}

////////////////////////////////////////////////////////////////////////

char get_char_val(NcVar *var, const int index) {
   char k;

   //
   // Retrieve the character array value from the NetCDF variable.
   //
   if(!var->set_cur(index) || !var->get(&k, 1)) {
      mlog << Error << "\n\tget_char_val() -> "
           << "can't read data from \"" << var->name()
           << "\" variable.\n\n";
      exit(1);
   }

   return (k);
}

////////////////////////////////////////////////////////////////////////

ConcatString* get_string_val(NcFile * nc, const char * var_name, const int index,
                    const int len, ConcatString &tmp_cs) {
   NcVar * var = (NcVar *) 0;

   var = nc->get_var(var_name);
   return (get_string_val(var, index, len, tmp_cs));
}

////////////////////////////////////////////////////////////////////////

ConcatString* get_string_val(NcVar *var, const int index,
                    const int len, ConcatString &tmp_cs) {
   char tmp_str[len];

   //
   // Retrieve the character array value from the NetCDF variable.
   //
   if(!var->set_cur(index) || !var->get(&tmp_str[0], 1, len)) {
      mlog << Error << "\n\tget_string_val(ConcatString) -> "
           << "can't read data from \"" << var->name()
           << "\" variable.\n\n";
      exit(1);
   }

   //
   // Store the character array as a ConcatString
   //
   tmp_cs = tmp_str;

   return (&tmp_cs);
}

////////////////////////////////////////////////////////////////////////

int get_int_var(NcFile * nc, const char * var_name, const int index) {
   NcVar * var = (NcVar *) 0;

   var = nc->get_var(var_name);
   return(get_int_var(var, index));
}

////////////////////////////////////////////////////////////////////////

int get_int_var(NcVar * var, const int index) {
   int k;

   k = bad_data_int;
   if (var) {
      if(!var->set_cur(index) || !var->get(&k, 1)) {
         mlog << Error << "\n\tget_int_var() -> "
              << "can't read data from \"" << var->name()
              << "\" variable.\n\n";
      }
   }

   return(k);
}

////////////////////////////////////////////////////////////////////////

double get_double_var(NcFile * nc, const char * var_name, const int index) {
   NcVar * var = (NcVar *) 0;

   var = nc->get_var(var_name);
   return(get_double_var(var, index));
}

////////////////////////////////////////////////////////////////////////

double get_double_var(NcVar * var, const int index) {
   double k;

   k = bad_data_double;
   if (var) {
      if(!var->set_cur(index) || !var->get(&k, 1)) {
         mlog << Error << "\n\tget_double_var() -> "
              << "can't read data from \"" << var->name()
              << "\" variable.\n\n";
      }
   }

   return(k);
}

////////////////////////////////////////////////////////////////////////

float get_float_var(NcFile * nc, const char * var_name, const int index) {
   NcVar * var = (NcVar *) 0;

   var = nc->get_var(var_name);
   return(get_float_var(var, index));
}

////////////////////////////////////////////////////////////////////////

float get_float_var(NcVar * var, const int index) {
   float k;

   k = bad_data_float;
   if (var) {
      if(!var->set_cur(index) || !var->get(&k, 1)) {
         mlog << Error << "\n\tget_float_var() -> "
              << "can't read data from \"" << var->name()
              << "\" variable.\n\n";
      }
   }

   return(k);
}

////////////////////////////////////////////////////////////////////////

bool get_nc_data(NcFile *nc, const char *var_name,
                 const long *cur, const long *dim, int *data) {
   
   //
   // Retrieve the input variables
   //
   NcVar *var    = get_nc_var(nc, var_name);
   return get_nc_data(var, cur, dim, data);
}

////////////////////////////////////////////////////////////////////////

bool get_nc_data(NcVar *var, const long *cur, const long *dim, int *data) {
   bool return_status = false;
   //mlog << Debug(3) << "get_nc_data(int) is called\n";

   if (var) {
      for (int idx1=0; idx1<dim[0]; idx1++) {
         if (cur[1] > 0) {
           for (int idx2=0; idx2<dim[1]; idx2++) {
              data[(idx1*dim[0])+idx2] = bad_data_float;
           }
         }
         else{
            data[idx1] = bad_data_int;
         }
      }
      
      //
      // Retrieve the float value from the NetCDF variable.
      // Note: missing data was checked here
      //
      if(!var->set_cur((long *) cur) || !var->get(data, (long *) dim)) {
         mlog << Error << "\nget_nc_data(int *) -> "
              << "can't read data from \"" << var->name()
              << "\" variable.\n\n";
         exit(1);
      }
      return_status = true;
   }
   return(return_status);
}

////////////////////////////////////////////////////////////////////////

bool get_nc_data(NcFile *nc, const char *var_name,
                 const long *cur, const long *dim, float *data) {
   
   //
   // Retrieve the input variables
   //
   NcVar *var    = get_nc_var(nc, var_name);
   return get_nc_data(var, cur, dim, data);
}

////////////////////////////////////////////////////////////////////////

bool get_nc_data(NcVar *var, const long *cur, const long *dim, float *data) {
   bool return_status = false;
   //mlog << Debug(3) << "get_nc_data(float) is called\n";

   if (var) {
      for (int idx1=0; idx1<dim[0]; idx1++) {
         if (cur[1] > 0) {
           for (int idx2=0; idx2<dim[1]; idx2++) {
              data[(idx1*dim[0])+idx2] = bad_data_float;
           }
         }
         else{
            data[idx1] = bad_data_double;
         }
      }
      
      //
      // Retrieve the float value from the NetCDF variable.
      // Note: missing data was checked here
      //
      if(!var->set_cur((long *) cur) || !var->get(data, (long *) dim)) {
         mlog << Error << "\nget_nc_data(float *) -> "
              << "can't read data from \"" << var->name()
              << "\" variable.\n\n";
         exit(1);
      }
      return_status = true;
   }
   return(return_status);
}

////////////////////////////////////////////////////////////////////////

bool get_nc_data(NcFile *nc, const char *var_name,
                 const long *cur, const long *dim, double *data) {
   
   //
   // Retrieve the input variables
   //
   NcVar *var    = get_nc_var(nc, var_name);
   return get_nc_data(var, cur, dim, data);
}

////////////////////////////////////////////////////////////////////////

bool get_nc_data(NcVar *var, const long *cur, const long *dim, double *data) {
   bool return_status = false;

   if (var) {
      for (int idx1=0; idx1<dim[0]; idx1++) {
         if (cur[1] > 0) {
           for (int idx2=0; idx2<dim[1]; idx2++) {
              data[(idx1*dim[0])+idx2] = bad_data_double;
           }
         }
         else{
            data[idx1] = bad_data_double;
         }
      }
      
      //
      // Retrieve the float value from the NetCDF variable.
      // Note: missing data was checked here
      //
      if(!var->set_cur((long *) cur) || !var->get(data, (long *) dim)) {
         mlog << Error << "\nget_nc_data(float *) -> "
              << "can't read data from \"" << var->name()
              << "\" variable.\n\n";
         exit(1);
      }
      return_status = true;
   }
   return(return_status);
}

////////////////////////////////////////////////////////////////////////

bool get_nc_data(NcFile *nc, const char *var_name,
                 const long *cur, const long *dim, char *data) {
   
   //
   // Retrieve the input variables
   //
   NcVar *var    = get_nc_var(nc, var_name);
   return get_nc_data(var, cur, dim, data);
}

////////////////////////////////////////////////////////////////////////

bool get_nc_data(NcVar *var, const long *cur, const long *dim, char *data) {
   bool return_status = false;

   if (var) {
      for (int idx1=0; idx1<dim[0]; idx1++) {
         if (cur[1] > 0) {
           for (int idx2=0; idx2<dim[1]; idx2++) {
              data[(idx1*dim[0])+idx2] = bad_data_char;
           }
         }
         else{
            data[idx1] = bad_data_char;
         }
      }
      
      //
      // Retrieve the char value from the NetCDF variable.
      //
      if(!var->set_cur((long *) cur) || !var->get(data, (long *) dim)) {
         mlog << Error << "\nget_nc_data(char **) -> "
              << "can't read data from \"" << var->name()
              << "\" variable.\n\n";
         exit(1);
      }
      return_status = true;
   }
   return(return_status);
}

////////////////////////////////////////////////////////////////////////

bool get_nc_data(NcFile *nc, const char *var_name,
                 const long *cur, const long *dim, ncbyte *data) {
   
   //
   // Retrieve the input variables
   //
   NcVar *var    = get_nc_var(nc, var_name);
   return get_nc_data(var, cur, dim, data);
}

////////////////////////////////////////////////////////////////////////

bool get_nc_data(NcVar *var, const long *cur, const long *dim, ncbyte *data) {
   bool return_status = false;

   if (var) {
      for (int idx1=0; idx1<dim[0]; idx1++) {
         if (cur[1] > 0) {
           for (int idx2=0; idx2<dim[1]; idx2++) {
              data[(idx1*dim[0])+idx2] = bad_data_char;
           }
         }
         else{
            data[idx1] = bad_data_char;
         }
      }
      
      //
      // Retrieve the char value from the NetCDF variable.
      //
      if(!var->set_cur((long *) cur) || !var->get(data, (long *) dim)) {
         mlog << Error << "\nget_nc_data(char **) -> "
              << "can't read data from \"" << var->name()
              << "\" variable.\n\n";
         exit(1);
      }
      return_status = true;
   }
   return(return_status);
}

////////////////////////////////////////////////////////////////////////

bool args_ok(const LongArray & a) {
   int j, k;

   for (j=0; j<(a.n_elements()); ++j)  {

      k = a[j];

      if ( (k < 0) && (k != vx_data2d_star) ) return (false);
   }

   return(true);
}

////////////////////////////////////////////////////////////////////////

NcVar* get_nc_var(NcFile *nc, const char *var_name) {
   NcVar *var = (NcVar *) 0;

   //
   // Retrieve the variable from the NetCDF file.
   //
   if(!(var = nc->get_var(var_name))) {
      mlog << Error << "\nget_nc_var() -> "
           << "can't read \"" << var_name << "\" variable.\n\n";
      exit(1);
   }

   return(var);
}


////////////////////////////////////////////////////////////////////////

NcVar* has_var(NcFile *nc, const char * var_name) {
   for(int i=0; i < nc->num_vars(); i++){
      NcVar* v = nc->get_var(i);
      if( !strcmp(v->name(), var_name) ) return v;
   }
   return NULL;
}

////////////////////////////////////////////////////////////////////////

NcDim* has_dim(NcFile *nc, const char * dim_name) {
   for(int i=0; i < nc->num_dims(); i++){
      NcDim* d = nc->get_dim(i);
      if( !strcmp(d->name(), dim_name) ) return d;
   }
   return NULL;
}

////////////////////////////////////////////////////////////////////////

bool get_global_att(const NcFile *nc, const ConcatString &att_name,
                    ConcatString &att_val, bool error_out) {
   int i, n;
   NcAtt *att = (NcAtt *) 0;
   bool status = false;

   // Initialize
   att_val.clear();

   n = nc->num_atts();

   // Loop through the attributes looking for a match
   for(i=0; i<n; i++) {

      att = nc->get_att(i);

      // Look for a match
      if(strcmp(att_name, att->name()) == 0) {
         att_val << att->as_string(0);
         status = true;
         break;
      }
   }

   // Check error_out status
   if(error_out && !status) {
      mlog << Error << "\nget_global_att() -> "
           << "can't find global NetCDF attribute \"" << att_name
           << "\".\n\n";
      exit(1);
   }

   return(status);
}

////////////////////////////////////////////////////////////////////////

bool get_global_att_double(const NcFile *nc, const ConcatString &att_name,
                           double &att_val, bool error_out) {
   int i, n;
   NcAtt *att = (NcAtt *) 0;
   bool status = false;

   // Initialize
   att_val = bad_data_double;

   n = nc->num_atts();

   // Loop through the attributes looking for a match
   for(i=0; i<n; i++) {

      att = nc->get_att(i);

      // Look for a match
      if(strcmp(att_name, att->name()) == 0) {
         att_val = att->as_double(0);
         status = true;
         break;
      }
   }

   // Check error_out status
   if(error_out && !status) {
      mlog << Error << "\nget_global_att_double() -> "
           << "can't find global NetCDF attribute \"" << att_name
           << "\".\n\n";
      exit(1);
   }

   return(status);
}

////////////////////////////////////////////////////////////////////////

bool get_dim(const NcFile *nc, const ConcatString &dim_name,
             int &dim_val, bool error_out) {
   int i, n;
   NcDim *dim = (NcDim *) 0;
   bool status = false;

   // Initialize
   dim_val = bad_data_int;

   n = nc->num_dims();

   // Loop through the dimensions looking for a match
   for(i=0; i<n; i++) {

      dim = nc->get_dim(i);

      // Look for a match
      if(strcmp(dim_name, dim->name()) == 0) {
         dim_val = (int) (dim->size());
         status = true;
         break;
      }
   }

   // Check error_out status
   if(error_out && !status) {
      mlog << Error << "\nget_dim() -> "
           << "can't find NetCDF dimension \"" << dim_name << "\".\n\n";
      exit(1);
   }

   return(status);
}

NcFile* open_ncfile(const char * nc_name, NcFile::FileMode file_mode) {
   //return new NcFile(nc_name, file_mode);
   cout << " Opening " << nc_name << "\n";
   return new NcFile(nc_name, file_mode, NULL, 0, NcFile::Netcdf4);
}

////////////////////////////////////////////////////////////////////////
