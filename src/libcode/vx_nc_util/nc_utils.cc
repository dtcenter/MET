// *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*
// ** Copyright UCAR (c) 1992 - 2022
// ** University Corporation for Atmospheric Research (UCAR)
// ** National Center for Atmospheric Research (NCAR)
// ** Research Applications Lab (RAL)
// ** P.O.Box 3000, Boulder, Colorado, 80307-3000, USA
// *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*

////////////////////////////////////////////////////////////////////////

using namespace std;

#include <string.h>
#include <cstring>
#include <sys/stat.h>

#include <netcdf>
using namespace netCDF;
using namespace netCDF::exceptions;

#include "vx_log.h"
#include "nc_utils.h"
#include "util_constants.h"
#include "vx_cal.h"

////////////////////////////////////////////////////////////////////////

void patch_nc_name(string *var_name) {
   size_t offset;

   // Replace commas with underscores
   offset = var_name->find(',');
   while (offset != string::npos) {
      var_name->replace(offset, 1, "_");
      offset = var_name->find(',', offset);
   }

   // Replaces stars with the word all
   offset = var_name->find('*');
   while (offset != string::npos) {
      var_name->replace(offset, 1, "all");
      offset = var_name->find('*', offset);
   }
}

////////////////////////////////////////////////////////////////////////

bool get_att_value(const NcAtt *att, ConcatString &value) {
   bool status = false;
   if (IS_VALID_NC_P(att)) {
      att->getValues(&value);
      status = true;
   }
   return status;
}

////////////////////////////////////////////////////////////////////////

bool get_att_value(const NcAtt *att, ncbyte &att_val) {
   bool status = get_att_num_value_(att, att_val, NC_BYTE);
   return(status);
}

////////////////////////////////////////////////////////////////////////

bool get_att_value(const NcAtt *att, short &att_val) {
   bool status = get_att_num_value_(att, att_val, NC_SHORT);
   return(status);
}

////////////////////////////////////////////////////////////////////////

bool get_att_value(const NcAtt *att, int &att_val) {
   bool status = get_att_num_value_(att, att_val, NC_INT);
   return(status);
}

////////////////////////////////////////////////////////////////////////

bool get_att_value(const NcAtt *att, unsigned int &att_val) {
   bool status = get_att_num_value_(att, att_val, NC_UINT);
   return(status);
}

////////////////////////////////////////////////////////////////////////

bool get_att_value(const NcAtt *att, float &att_val) {
   bool status = get_att_num_value_(att, att_val, NC_FLOAT);
   return(status);
}

////////////////////////////////////////////////////////////////////////

bool get_att_value(const NcAtt *att, double &att_val) {
   bool status = get_att_num_value_(att, att_val, NC_DOUBLE);
   return(status);
}

////////////////////////////////////////////////////////////////////////

int get_att_value_int(const NcAtt *att) {
   int value = bad_data_int;
   static const char *method_name = "get_att_value_int(NcAtt) -> ";

   if (IS_INVALID_NC_P(att)) return value;

   switch (att->getType().getId()) {
      case NC_BYTE:
         ncbyte b_value;
         att->getValues(&b_value);
         value = (int)b_value;
         break;
      case NC_SHORT:
         short s_value;
         att->getValues(&s_value);
         value = (int)s_value;
         break;
      case NC_INT:
         att->getValues(&value);
         break;
      case NC_INT64:
         long long l_value;
         att->getValues(&l_value);
         value = (int)l_value;
         if ((long long)value != l_value) {
            mlog << Warning << "\n" << method_name
                 << "loosing precision during type conversion. "
                 << value << " from int64 \"" << l_value
                 << "\" for attribute \"" << GET_SAFE_NC_NAME_P(att) << "\".\n\n";
         }
         break;
      default:
         mlog << Warning << "\n" << method_name
              << "data type mismatch (int vs. \"" << GET_NC_TYPE_NAME_P(att)
              << "\" for attribute \"" << GET_SAFE_NC_NAME_P(att) << "\".\n\n";
         break;
   }
   return value;
}

////////////////////////////////////////////////////////////////////////

char get_att_value_char(const NcAtt *att) {
   char att_val = bad_data_char;
   static const char *method_name = "get_att_value_char(NcAtt) -> ";
   if (IS_VALID_NC_P(att)) {
      nc_type attType = GET_NC_TYPE_ID_P(att);
      if (attType == NC_CHAR) {
         att->getValues(&att_val);
      }
      else {
         mlog << Error << "\n" << method_name
              << "Please convert data type of \"" << GET_NC_NAME_P(att)
              << "\" to NC_CHAR type.\n\n";
         exit(1);
      }
   }
   return att_val;
}

////////////////////////////////////////////////////////////////////////

bool get_att_value_chars(const NcAtt *att, ConcatString &value) {
   bool status = false;
   static const char *method_name = "get_att_value_chars(NcAtt) -> ";
   if (IS_VALID_NC_P(att)) {
      nc_type attType = GET_NC_TYPE_ID_P(att);
      if (attType == NC_CHAR || attType == NC_STRING) {
         try {
            string att_value;
            att->getValues(att_value);
            value = att_value;
         }
         catch (exceptions::NcChar ex) {
            value = "";
            // Handle netCDF::exceptions::NcChar:  NetCDF: Attempt to convert between text & numbers
            mlog << Warning << "\n" << method_name
                 << "Exception: " << ex.what() << "\n"
                 << "Fail to read " << GET_NC_NAME_P(att) << " attribute ("
                 << GET_NC_TYPE_NAME_P(att) << " type).\n"
                 << "Please check the encoding of the "<< GET_NC_NAME_P(att) << " attribute.\n\n";
         }
      }
      else { // MET-788: to handle a custom modified NetCDF
         mlog << Error << "\n" << method_name
              << "Please convert data type of \"" << GET_NC_NAME_P(att)
              << "\" " << GET_NC_TYPE_NAME_P(att) << " to NC_CHAR type.\n\n";
         exit(1);
      }
      status = true;
   }
   return status;
}

////////////////////////////////////////////////////////////////////////

long long get_att_value_llong(const NcAtt *att) {
   long long value = bad_data_int;
   if (IS_VALID_NC_P(att)) att->getValues(&value);
   return value;
}

////////////////////////////////////////////////////////////////////////

double get_att_value_double(const NcAtt *att) {
   double value = bad_data_double;
   if (IS_VALID_NC_P(att)) att->getValues(&value);
   return value;
}

////////////////////////////////////////////////////////////////////////

void get_att_value_doubles(const NcAtt *att, NumArray &value) {
   value.erase();
   double *values = new double[att->getAttLength()];
   att->getValues(values);
   for(unsigned int i=0; i<=att->getAttLength(); i++) value.add(values[i]);
   if(values) { delete [] values; values = 0; }
   return;
}

double *get_att_value_doubles(const NcAtt *att) {
   double *values = new double[att->getAttLength()];
   att->getValues(values);
   return values;
}

////////////////////////////////////////////////////////////////////////

float get_att_value_float(const NcAtt *att) {
   float value = bad_data_float;
   att->getValues(&value);
   return value;
}

////////////////////////////////////////////////////////////////////////

short get_att_value_short(const NcAtt *att) {
   short value = bad_data_int;
   att->getValues(&value);
   return value;
}

unsigned short get_att_value_ushort(const NcAtt *att) {
   unsigned short value = bad_data_int;
   att->getValues(&value);
   return value;
}

////////////////////////////////////////////////////////////////////////

bool get_att_value_string(const NcVar *var, const ConcatString &att_name, ConcatString &value) {
   NcVarAtt *att = get_nc_att(var, att_name);
   bool status =  get_att_value_chars(att, value);
   if (att) delete att;
   return status;
}

////////////////////////////////////////////////////////////////////////

int  get_att_value_int   (const NcVar *var, const ConcatString &att_name) {
   NcVarAtt *att = get_nc_att(var, att_name);
   int att_val = get_att_value_int(att);
   if (att) delete att;
   return att_val;
}

////////////////////////////////////////////////////////////////////////

long long  get_att_value_llong (const NcVar *var, const ConcatString &att_name) {
   NcVarAtt *att = get_nc_att(var, att_name);
   long long att_val = get_att_value_llong(att);
   if (att) delete att;
   return att_val;
}

////////////////////////////////////////////////////////////////////////

double get_att_value_double(const NcVar *var, const ConcatString &att_name) {
   NcVarAtt *att = get_nc_att(var, att_name);
   double att_val = get_att_value_double(att);
   if (att) delete att;
   return att_val;
}

////////////////////////////////////////////////////////////////////////

bool get_att_value_string(const NcFile *nc, const ConcatString &att_name, ConcatString &value) {
   NcGroupAtt *att = get_nc_att(nc, att_name);
   bool status = get_att_value_chars(att, value);
   if (att) delete att;
   return status;
}

////////////////////////////////////////////////////////////////////////

int  get_att_value_int   (const NcFile *nc, const ConcatString &att_name) {
   NcGroupAtt *att = get_nc_att(nc, att_name);
   int att_val = get_att_value_int(att);
   if (att) delete att;
   return att_val;
}

////////////////////////////////////////////////////////////////////////

long long  get_att_value_llong (const NcFile *nc, const ConcatString &att_name) {
   NcGroupAtt *att = get_nc_att(nc, att_name);
   long long att_val = get_att_value_llong(att);
   if (att) delete att;
   return att_val;
}

////////////////////////////////////////////////////////////////////////

double  get_att_value_double(const NcFile *nc, const ConcatString &att_name) {
   NcGroupAtt *att = get_nc_att(nc, att_name);
   double att_val = get_att_value_double(att);
   if (att) delete att;
   return att_val;
}

////////////////////////////////////////////////////////////////////////

bool    get_att_no_leap_year(const NcVar *var) {
   bool no_leap_year = false;
   NcVarAtt *calendar_att = get_nc_att(var, string("calendar"), false);
   if (IS_VALID_NC_P(calendar_att)) {
      ConcatString calendar_value;
      if (get_att_value_chars(calendar_att, calendar_value)) {
         no_leap_year = ( "noleap" == calendar_value
                        || "365_day" == calendar_value
                        || "365 days" == calendar_value);
      }
   }
   if (calendar_att) delete calendar_att;
   return no_leap_year;
}

////////////////////////////////////////////////////////////////////////

ConcatString get_log_msg_for_att(const NcVarAtt *att) {
   ConcatString log_msg("can't read attribute");
   if(IS_INVALID_NC_P(att)) {
      log_msg << " because attribute does not exist";
   }
   else {
      log_msg << " \"" << GET_NC_NAME_P(att) << "\" from \""
              << GET_SAFE_NC_NAME(att->getParentVar()) << "\" variable.\n\n";
   }
   log_msg << ".\n\n";
   return(log_msg);
}

////////////////////////////////////////////////////////////////////////

ConcatString get_log_msg_for_att(const NcVarAtt *att, string var_name,
                             const ConcatString att_name) {
   ConcatString log_msg;
   log_msg << "can't read attribute" << " \""
           << ((att_name.length() > 0) ? att_name.c_str() : GET_SAFE_NC_NAME_P(att))
           << "\" because attribute does not exist";
   if (0 != var_name.compare(C_unknown_str)) {
      log_msg << " from \"" << var_name << "\" variable";
   }
   else {
      if(IS_VALID_NC_P(att)) {
         log_msg << " from \"" << GET_SAFE_NC_NAME(att->getParentVar()) << "\" variable";
      }
   }
   log_msg << ".\n\n";
   return(log_msg);
}

////////////////////////////////////////////////////////////////////////

NcVarAtt *get_nc_att(const NcVar * var, const ConcatString &att_name, bool exit_on_error) {
   NcVarAtt *att = (NcVarAtt *)0;
   static const char *method_name = "get_nc_att(NcVar) -> ";

   //
   // Retrieve the NetCDF variable attribute.
   //
   if(IS_INVALID_NC_P(var)) {
      mlog << Error << "\n" << method_name
           << "can't read attribute \"" << att_name
           << "\" from because variable is invalid.\n\n";
   }
   else {
      multimap<string,NcVarAtt>::iterator itAtt;
      map<string,NcVarAtt> mapAttrs = var->getAtts();
      for (itAtt = mapAttrs.begin(); itAtt != mapAttrs.end(); ++itAtt) {
         if ( att_name == (*itAtt).first) {
            att = new NcVarAtt();
            *att = (*itAtt).second;
            break;
         }
      }

      if(IS_INVALID_NC_P(att) && exit_on_error) {
         mlog << Error << "\n" << method_name
              << "can't read attribute \"" << att_name
              << "\" from \"" << var->getName() << "\" variable.\n\n";
         exit(1);
      }
   }
   return(att);
}

////////////////////////////////////////////////////////////////////////

NcGroupAtt *get_nc_att(const NcFile * nc, const ConcatString &att_name, bool exit_on_error) {
   NcGroupAtt *att = (NcGroupAtt *)0;
   static const char *method_name = "get_nc_att(NcFile) -> ";

   //
   // Retrieve the NetCDF variable attribute.
   //
   if(IS_INVALID_NC_P(nc)) {
      mlog << Error << "\n" << method_name
           << "can't read attribute \"" << att_name
           << "\" from because NC is invalid.\n\n";
      if (exit_on_error) exit(1);
   }
   else {
      multimap<string,NcGroupAtt>::iterator itAtt;
      multimap<string,NcGroupAtt> mapAttrs = nc->getAtts();
      for (itAtt = mapAttrs.begin(); itAtt != mapAttrs.end(); ++itAtt) {
         if ( att_name == (*itAtt).first ) {
            att = new NcGroupAtt();
            *att = (*itAtt).second;
            break;
         }
      }

      if(IS_INVALID_NC_P(att) && exit_on_error) {
         mlog << Error << "\n" << method_name
              << "can't read attribute \"" << att_name
              << "\" from \"" << nc->getName() << "\".\n\n";
         exit(1);
      }
   }
   return(att);
}

////////////////////////////////////////////////////////////////////////

bool get_nc_att_value(const NcVar *var, const ConcatString &att_name,
                      ConcatString &att_val, bool exit_on_error) {
   bool status = false;
   NcVarAtt *att = (NcVarAtt *) 0;

   // Initialize
   att_val.clear();

   att = get_nc_att(var, att_name);

   // Look for a match
   status = get_att_value_chars(att, att_val);
   if (att) delete att;

   return(status);
}

////////////////////////////////////////////////////////////////////////

bool get_nc_att_value(const NcVar *var, const ConcatString &att_name,
                      int &att_val, bool exit_on_error) {
   static const char *method_name = "get_nc_att_value(NcVar,int) -> ";
   bool status = get_nc_att_value_(var, att_name, att_val, exit_on_error,
                                   bad_data_int, method_name);
   return(status);
}

////////////////////////////////////////////////////////////////////////

bool get_nc_att_value(const NcVar *var, const ConcatString &att_name,
                      double &att_val, bool exit_on_error) {
   static const char *method_name = "get_nc_att_value(NcVar,double) -> ";
   bool status = get_nc_att_value_(var, att_name, att_val, exit_on_error,
                                   bad_data_double, method_name);
   return(status);
}

////////////////////////////////////////////////////////////////////////

bool get_nc_att_value(const NcVar *var, const ConcatString &att_name,
                      float &att_val, bool exit_on_error) {
   static const char *method_name = "get_nc_att_value(NcVar,float) -> ";
   bool status = get_nc_att_value_(var, att_name, att_val, exit_on_error,
                                   bad_data_float, method_name);
   return(status);
}

////////////////////////////////////////////////////////////////////////

bool get_nc_att_value(const NcVarAtt *att, ConcatString &att_val) {
   bool status = false;

   // Initialize
   att_val.clear();

   // Look for a match
   if(IS_VALID_NC_P(att)) {
      string attr_value;
      att->getValues(attr_value);
      att_val = attr_value.c_str();
      status = true;
   }

   return(status);
}

////////////////////////////////////////////////////////////////////////

bool get_nc_att_value(const NcVarAtt *att, int &att_val, bool exit_on_error) {
   static const char *method_name = "get_nc_att_value(NcVarAtt,int) -> ";
   bool status = get_nc_att_value_(att, att_val, exit_on_error, bad_data_int, method_name);
   return(status);
}

////////////////////////////////////////////////////////////////////////

bool get_nc_att_value(const NcVarAtt *att, float &att_val, bool exit_on_error) {
   static const char *method_name = "get_nc_att_value(NcVarAtt,float) -> ";
   bool status = get_nc_att_value_(att, att_val, exit_on_error, bad_data_float, method_name);
   return(status);
}

////////////////////////////////////////////////////////////////////////

bool get_nc_att_value(const NcVarAtt *att, double &att_val, bool exit_on_error) {
   static const char *method_name = "get_nc_att_value(NcVarAtt,double) -> ";
   bool status = get_nc_att_value_(att, att_val, exit_on_error, bad_data_double, method_name);
   return(status);
}

///////////////////////////////////////////////////////////////////////////////

bool has_att(NcFile *ncfile, const ConcatString att_name, bool do_log) {
   bool status = false;
   NcGroupAtt *att;

   att = get_nc_att(ncfile, att_name);
   if (IS_VALID_NC_P(att)) {
      status = true;
   }
   else if (do_log) {
      mlog << Warning << "\nhas_att() -> "
           << "can't find global NetCDF attribute " << att_name
           << ".\n\n";
   }
   if (att) delete att;
   return status;
}

///////////////////////////////////////////////////////////////////////////////

bool has_att(NcVar *var, const ConcatString att_name, bool do_log) {
   bool status = false;

   NcVarAtt *att = get_nc_att(var, att_name);
   if (IS_VALID_NC_P(att)) {
      status = true;
   }
   else if (do_log) {
      mlog << Warning << "\nhas_att() -> "
           << "can't find NetCDF variable attribute " << att_name
           << ".\n\n";
   }
   if (att) delete att;
   return status;
}

////////////////////////////////////////////////////////////////////////

bool has_add_offset_attr(NcVar *var) {
   return has_att(var, add_offset_att_name);
}

////////////////////////////////////////////////////////////////////////

bool has_scale_factor_attr(NcVar *var) {
   return has_att(var, scale_factor_att_name);
}

////////////////////////////////////////////////////////////////////////

bool has_unsigned_attribute(NcVar *var) {
   bool is_unsigned = false;
   static const char *method_name = "has_unsigned_attribute() -> ";
   NcVarAtt *att_unsigned = get_nc_att(var, string("_Unsigned"));
   if (IS_VALID_NC_P(att_unsigned)) {
      ConcatString att_value;
      get_att_value_chars(att_unsigned, att_value);
      is_unsigned = ( att_value == "true" );
   }
   if(att_unsigned) delete att_unsigned;
   mlog << Debug(6) << method_name
        << GET_NC_NAME_P(var) << " " << (is_unsigned ? "has " : "does not have" )
        << " _Unsigned attribute.\n";

   return is_unsigned;
}

////////////////////////////////////////////////////////////////////////

bool get_global_att(const NcGroupAtt *att, ConcatString &att_val) {
   bool status = false;

   // Initialize
   att_val.clear();

   // Look for a match
   if(IS_VALID_NC_P(att)) {
      string attr_value;
      att->getValues(attr_value);
      att_val = attr_value.c_str();
      status = true;
   }

   return(status);
}

////////////////////////////////////////////////////////////////////////

bool get_global_att(const char *nc_name, const ConcatString &att_name,
                    ConcatString &att_val) {
   bool status = false;

   // Initialize
   att_val.clear();

   NcFile *nc = open_ncfile(nc_name);
   if (0 != nc && IS_VALID_NC_P(nc)) {
      status = get_global_att(nc, att_name, att_val, false);
   }

   if(nc) delete nc;

   return(status);
}

////////////////////////////////////////////////////////////////////////

bool get_global_att(const char *nc_name, const ConcatString &att_name,
                    bool &att_val) {
   bool status = false;

   // Initialize
   NcFile *nc = open_ncfile(nc_name);
   if (0 != nc && IS_VALID_NC_P(nc)) {
      status = get_global_att(nc, att_name, att_val, false);
   }

   if(nc) delete nc;

   return(status);
}

////////////////////////////////////////////////////////////////////////

bool get_global_att(const NcFile *nc, const ConcatString &att_name,
                    ConcatString &att_val, bool error_out) {
   bool status = false;
   NcGroupAtt *att;

   // Initialize
   att_val.clear();

   att = get_nc_att(nc, att_name);
   if(IS_VALID_NC_P(att)) {
      string attr_val;
      att->getValues(attr_val);
      att_val = attr_val.c_str();
      status = true;
   }
   if (att) delete att;

   // Check error_out status
   if(error_out && !status) {
      mlog << Error << "\nget_global_att(ConcatString) -> "
           << "can't find global NetCDF attribute \"" << att_name
           << "\".\n\n";
      exit(1);
   }

   return(status);
}

////////////////////////////////////////////////////////////////////////


bool get_global_att(const NcFile *nc, const ConcatString& att_name,
                    int &att_val, bool error_out) {
   static const char *method_name = "\nget_global_att(int) -> ";
   bool status = get_global_att_value_(nc, att_name, att_val, bad_data_int,
                                       false, method_name);
   if (!status) {
      short tmp_att_val;
      status = get_global_att_value_(nc, att_name, tmp_att_val, (short)bad_data_int,
                                     false, method_name);
      if (status) att_val = tmp_att_val;
      else {
         ncbyte tmp_val2;
         status = get_global_att_value_(nc, att_name, tmp_val2, (ncbyte)bad_data_int,
                                        error_out, method_name);
         if (status) att_val = tmp_val2;
      }
   }

   return(status);
}

////////////////////////////////////////////////////////////////////////

bool get_global_att(const NcFile *nc, const ConcatString& att_name,
                    bool &att_val, bool error_out) {
   bool status;
   ConcatString att_value;
   static const char *method_name = "\nget_global_att(bool) -> ";

   // Initialize
   att_val = false;
   status = get_global_att(nc, att_name, att_value, error_out);
   if(status) {
      att_val = (0 == strcmp("true", att_value.text()))
             || (0 == strcmp("yes", att_value.text()));
   }
   else if(error_out) {
      // Check error_out status
      mlog << Error << method_name
           << "can't find global NetCDF attribute \"" << att_name << "\".\n\n";
      exit(1);
   }

   return(status);
}

////////////////////////////////////////////////////////////////////////

bool get_global_att(const NcFile *nc, const ConcatString& att_name,
                    float &att_val, bool error_out) {
   static const char *method_name = "\nget_global_att(float) -> ";
   bool status = get_global_att_value_(nc, att_name, att_val, bad_data_float,
                                      error_out, method_name);

   return(status);
}

////////////////////////////////////////////////////////////////////////

bool get_global_att(const NcFile *nc, const ConcatString& att_name,
                    double &att_val, bool error_out) {
   static const char *method_name = "\nget_global_att(double) -> ";
   bool status;
   status = get_global_att_value_(nc, att_name, att_val, bad_data_double,
                                  false, method_name);
   if (!status) {
      float tmp_att_val;
      status = get_global_att_value_(nc, att_name, tmp_att_val, bad_data_float,
                                     error_out, method_name);
      if (status) att_val = tmp_att_val;
   }

   return (status);
}

////////////////////////////////////////////////////////////////////////

int get_version_no(const NcFile *nc) {
   int version_no = 0;
   float att_version_no;
   get_global_att(nc, (const ConcatString)nc_att_obs_version, att_version_no);
   version_no = (int)(att_version_no * 100);
   return version_no;
}

////////////////////////////////////////////////////////////////////////

bool is_version_less_than_1_02(const NcFile *nc) {
   int version_no = get_version_no(nc);
   return (version_no < 102);
}

////////////////////////////////////////////////////////////////////////

void add_att(NcFile *nc, const string &att_name, const int att_val) {
   nc->putAtt(att_name, NcType::nc_INT, att_val);
}

////////////////////////////////////////////////////////////////////////

void add_att(NcFile *nc, const string &att_name, const string att_val) {
   nc->putAtt(att_name, att_val);
}

////////////////////////////////////////////////////////////////////////

void add_att(NcFile *nc, const string &att_name, const char *att_val) {
   nc->putAtt(att_name, att_val);
}

////////////////////////////////////////////////////////////////////////

void add_att(NcFile *nc, const string &att_name, const ConcatString att_val) {
  nc->putAtt(att_name, att_val.text());
}

////////////////////////////////////////////////////////////////////////

void add_att(NcVar *var, const string &att_name, const string att_val) {
   var->putAtt(att_name, att_val);
}

////////////////////////////////////////////////////////////////////////

void add_att(NcVar *var, const string &att_name, const int att_val) {
   var->putAtt(att_name, NcType::nc_INT, att_val);
}

////////////////////////////////////////////////////////////////////////

void add_att(NcVar *var, const string &att_name, const float att_val) {
   var->putAtt(att_name, NcType::nc_FLOAT, att_val);
}

////////////////////////////////////////////////////////////////////////

void add_att(NcVar *var, const string &att_name, const double att_val) {
   var->putAtt(att_name, NcType::nc_DOUBLE, att_val);
}


////////////////////////////////////////////////////////////////////////

int get_var_names(NcFile *nc, StringArray *varNames) {

   int i, varCount;
   NcVar var;

   varCount = nc->getVarCount();

   i = 0;
   multimap<string,NcVar>::iterator itVar;
   multimap<string,NcVar> mapVar = GET_NC_VARS_P(nc);
   for (itVar = mapVar.begin(); itVar != mapVar.end(); ++itVar) {
      var = (*itVar).second;
      varNames->add(var.getName());
      i++;
   }

   if (i != varCount) {
      mlog << Error << "\n\tget_var_names() -> "
           << "does not match array, allocated " << varCount << " but assigned "
           << i << ".\n\n";
   }
   return(varCount);
}

////////////////////////////////////////////////////////////////////////

bool get_var_att_double(const NcVar *var, const ConcatString &att_name,
                        double &att_val) {
   bool status = get_var_att_num_(var, att_name, att_val, bad_data_double);

   return(status);
}

////////////////////////////////////////////////////////////////////////

bool get_var_att_float(const NcVar *var, const ConcatString &att_name,
                       float &att_val) {
   bool status = get_var_att_num_(var, att_name, att_val, bad_data_float);

   return(status);
}

////////////////////////////////////////////////////////////////////////

double get_var_add_offset(const NcVar *var) {
   double v;

   if(!get_var_att_double(var, add_offset_att_name, v)) {
      v = 0.f;
   }

   return(v);
}

////////////////////////////////////////////////////////////////////////

bool get_var_axis(const NcVar *var, ConcatString &att_val) {
   return(get_nc_att_value(var, axis_att_name, att_val));
}

////////////////////////////////////////////////////////////////////////

void set_def_fill_value(ncbyte *val) { *val = bad_data_int/100; }
void set_def_fill_value(char   *val) { *val = bad_data_char; }
void set_def_fill_value(double *val) { *val = bad_data_double; }
void set_def_fill_value(float  *val) { *val = bad_data_float; }
void set_def_fill_value(int    *val) { *val = bad_data_int; }
void set_def_fill_value(long   *val) { *val = (long)bad_data_int; }
void set_def_fill_value(short  *val) { *val = (short)bad_data_int; }
void set_def_fill_value(long long          *val) { *val = (long long)bad_data_int; }
void set_def_fill_value(unsigned char      *val) { *val = (unsigned char)-1; }
void set_def_fill_value(unsigned int       *val) { *val = (unsigned int)-1; }
void set_def_fill_value(unsigned long      *val) { *val = (unsigned long)-1; }
void set_def_fill_value(unsigned long long *val) { *val = (unsigned long long)-1; }
void set_def_fill_value(unsigned short     *val) { *val = (unsigned short)-1; }

////////////////////////////////////////////////////////////////////////

bool get_var_grid_mapping(const NcVar *var, ConcatString &att_val) {
   return(get_nc_att_value(var, grid_mapping_att_name, att_val));
}

////////////////////////////////////////////////////////////////////////

bool get_var_grid_mapping_name(const NcVar *var, ConcatString &att_val) {
   return(get_nc_att_value(var, grid_mapping_name_att_name, att_val));
}

////////////////////////////////////////////////////////////////////////

bool get_var_long_name(const NcVar *var, ConcatString &att_val) {
   return(get_nc_att_value(var, long_name_att_name, att_val));
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

double get_var_scale_factor(const NcVar *var) {
   double v;

   if(!get_var_att_double(var, scale_factor_att_name, v)) {
      v = 1.f;
   }

   return(v);
}

////////////////////////////////////////////////////////////////////////

bool get_var_standard_name(const NcVar *var, ConcatString &att_val) {
   return(get_nc_att_value(var, standard_name_att_name, att_val));
}

////////////////////////////////////////////////////////////////////////

bool get_var_units(const NcVar *var, ConcatString &att_val) {

   return(get_nc_att_value(var, units_att_name, att_val));
}

////////////////////////////////////////////////////////////////////////

char get_char_val(NcFile * nc, const char * var_name, const int index) {
   NcVar var = get_var(nc, var_name);
   return (get_char_val(&var, index));
}

////////////////////////////////////////////////////////////////////////

char get_char_val(NcVar *var, const int index) {
   char k;
   vector<size_t> start;
   vector<size_t> count;

   //
   // Retrieve the character array value from the NetCDF variable.
   //
   start.push_back(index);
   count.push_back(1);
   var->getVar(start, count, &k);

   return (k);
}

////////////////////////////////////////////////////////////////////////

ConcatString* get_string_val(NcFile * nc, const char * var_name, const int index,
                             const int len, ConcatString &tmp_cs) {
   NcVar var = get_var(nc, var_name);

   return (get_string_val(&var, index, len, tmp_cs));
}

////////////////////////////////////////////////////////////////////////

ConcatString* get_string_val(NcVar *var, const int index,
                             const int len, ConcatString &tmp_cs) {
   char tmp_str[len];
   vector<size_t> start;
   vector<size_t> count;
   const char *method_name = "get_string_val() ";

   if (2 != get_dim_count(var)) {
      mlog << Error << "\n" << method_name << GET_NC_NAME_P(var)
           << " is not a two dimensional variablle. start offset and count: ("
           << index << ", " << len << ").\n\n";
      exit(1);
   }
   else {
      int dim_size1 = get_dim_size(var, 0);
      int dim_size2 = get_dim_size(var, 1);
      if ((index > dim_size1) || (len > dim_size2)) {
         mlog << Error << "\n" << method_name << "The start offset and count ("
              << index << ", " << len << ") exceeds the dimension size ("
              << dim_size1 << ", " << dim_size2 << ") for the variable "
              << GET_NC_NAME_P(var) << ".\n\n";
         exit(1);
      }
   }

   //
   // Retrieve the character array value from the NetCDF variable.
   //
   start.push_back(index);
   start.push_back(0);
   count.push_back(1);
   count.push_back(len);
   var->getVar(start, count, &tmp_str);

   //
   // Store the character array as a ConcatString
   //
   tmp_cs = tmp_str;

   return (&tmp_cs);
}

////////////////////////////////////////////////////////////////////////

int get_int_var(NcFile * nc, const char * var_name, const int index) {
   NcVar var = get_var(nc, var_name);
   return(get_int_var(&var, index));
}

////////////////////////////////////////////////////////////////////////

int get_int_var(NcVar * var, const int index) {
   int k;
   vector<size_t> start;
   vector<size_t> count;
   const char *method_name = "get_int_var() ";

   k = bad_data_int;
   if (IS_VALID_NC_P(var)) {
      int dim_idx = 0;
      int dim_size = get_dim_size(var, dim_idx);
      if (0 >= dim_size) {
         if (index > 0) {
            mlog << Error << "\n" << method_name << "The start offset ("
                 << index << ") should be 0 because of no dimension at the variable "
                 << GET_NC_NAME_P(var) << ".\n\n";
            exit(1);
         }
      }
      else if (index > dim_size) {
         NcDim nc_dim = get_nc_dim(var, dim_idx);
         mlog << Error << "\n" << method_name << "The start offset ("
              << index << ") exceeds the dimension " << dim_size << " "
              << (IS_VALID_NC(nc_dim) ? GET_NC_NAME(nc_dim) : " ")
              << " for the variable " << GET_NC_NAME_P(var) << ".\n\n";
         exit(1);
      }

      start.push_back(index);
      count.push_back(1);
      var->getVar(start, count, &k);
   }

   return(k);
}

////////////////////////////////////////////////////////////////////////

double get_nc_time(NcVar * var, const int index) {
   double k;
   vector<size_t> start;
   vector<size_t> count;
   const char *method_name = "get_nc_time() -> ";

   k = bad_data_double;
   if (IS_VALID_NC_P(var)) {
      int dim_idx = 0;
      int dim_size = get_dim_size(var, dim_idx);
      if (0 >= dim_size) {
         if (index > 0) {
            mlog << Error << "\n" << method_name << "The start offset ("
                 << index << ") should be 0 because of no dimension at the variable "
                 << GET_NC_NAME_P(var) << ".\n\n";
            exit(1);
         }
      }
      else if (index > dim_size) {
         NcDim nc_dim = get_nc_dim(var, dim_idx);
         mlog << Error << "\n" << method_name << "The start offset ("
              << index << ") exceeds the dimension " << dim_size << " "
              << (IS_VALID_NC(nc_dim) ? GET_NC_NAME(nc_dim) : " ")
              << " for the variable " << GET_NC_NAME_P(var) << ".\n\n";
         exit(1);
      }

      start.push_back(index);
      count.push_back(1);

      int vi;
      short vs;
      float vf;
      ncbyte vb;
      long long vl;
      int dataType = GET_NC_TYPE_ID_P(var);
      switch (dataType) {
         case NC_DOUBLE:
            var->getVar(start, count, &k);
            break;
         case NC_FLOAT:
            var->getVar(start, count, &vf);
            k = (double)vf;
            break;
         case NC_SHORT:
            var->getVar(start, count, &vs);
            k = (double)vs;
            break;
         case NC_BYTE:
            var->getVar(start, count, &vb);
            k = (double)vb;
            break;
         case NC_INT:
            var->getVar(start, count, &vi);
            k = (double)vi;
            break;
         case NC_INT64:
            long long converted_vl;
            var->getVar(start, count, &vl);
            k = (double)vl;
            converted_vl = (long long)k;
            if (converted_vl != vl) {
               mlog << Warning << "\n" << method_name
                    << " the value was changed during type conversion: "
                    << converted_vl << " (was  " << vl << ")\n";
            }
            break;
         default:
            mlog << Error << "\n" << method_name
                 << "data type mismatch (double vs. \"" << GET_NC_TYPE_NAME_P(var)
                 << "\").\nPlease correct the data type to double for variable \""
                 << GET_NC_NAME_P(var) << "\".\n\n";
            exit(1);
      }
   }

   return(k);
}

////////////////////////////////////////////////////////////////////////

float get_float_var(NcFile * nc, const char * var_name, const int index) {
   NcVar var = get_var(nc, var_name);
   return(get_float_var(&var, index));
}

////////////////////////////////////////////////////////////////////////

float get_float_var(NcVar * var, const int index) {
   float k;
   vector<size_t> start;
   vector<size_t> count;
   const char *method_name = "get_float_var() -> ";

   k = bad_data_float;
   if (IS_VALID_NC_P(var)) {
      int dim_idx = 0;
      int dim_size = get_dim_size(var, dim_idx);
      if (0 >= dim_size) {
         if (index > 0) {
            mlog << Error << "\n" << method_name << "The start offset ("
                 << index << ") should be 0 because of no dimension at the variable "
                 << GET_NC_NAME_P(var) << ".\n\n";
            exit(1);
         }
      }
      else if ((index > dim_size) && (0 < dim_size)){
         NcDim nc_dim = get_nc_dim(var, dim_idx);
         mlog << Error << "\n" << method_name << "The start offset ("
              << index << ") exceeds the dimension " << dim_size << " "
              << (IS_VALID_NC(nc_dim) ? GET_NC_NAME(nc_dim) : " ")
              << " for the variable " << GET_NC_NAME_P(var) << ".\n\n";
         exit(1);
      }

      start.push_back(index);
      count.push_back(1);
      var->getVar(start, count, &k);
   }

   return(k);
}

////////////////////////////////////////////////////////////////////////

bool get_nc_data(NcVar *var, int *data, const long *curs) {
   bool return_status = get_nc_data_(var, data, bad_data_int, curs);

   return(return_status);
}

////////////////////////////////////////////////////////////////////////

bool get_nc_data(NcVar *var, time_t *data) {
   bool return_status = get_nc_data_(var, data, (time_t)bad_data_int);

   return(return_status);
}

////////////////////////////////////////////////////////////////////////

bool get_nc_data(NcVar *var, int *data) {
   bool return_status = get_nc_data_(var, data, bad_data_int);
   return(return_status);
}

////////////////////////////////////////////////////////////////////////

bool get_nc_data(NcVar *var, int *data, const long dim, const long cur) {
   return(get_nc_data_(var, data, bad_data_int, dim, cur));
}

////////////////////////////////////////////////////////////////////////

bool get_nc_data(NcVar *var, int *data, const long *dims, const long *curs) {
   bool return_status = get_nc_data_(var, data, bad_data_int, dims, curs);

   return(return_status);
}

////////////////////////////////////////////////////////////////////////

bool get_nc_data(NcVar *var, short *data, const long *curs) {
   bool return_status = get_nc_data_(var, data, (short)bad_data_int, curs);

   return(return_status);
}

////////////////////////////////////////////////////////////////////////

bool get_nc_data(NcVar *var, short *data, const long *dims, const long *curs) {
   bool return_status = get_nc_data_(var, data, (short)bad_data_int, dims, curs);

   return(return_status);
}

////////////////////////////////////////////////////////////////////////

bool get_nc_data(NcVar *var, float *data) {
   bool return_status = false;
   clock_t start_clock = clock();
   static const char *method_name = "get_nc_data(NcVar *, float *) ";

   if (IS_VALID_NC_P(var)) {
      //
      // Retrieve the float value from the NetCDF variable.
      // Note: missing data was checked here
      //
      int type_id = GET_NC_TYPE_ID_P(var);
      int cell_count = get_data_size(var);

      return_status = true;
      if (NcType::nc_FLOAT == type_id) {

         get_nc_data_t(var, data);

         float fill_value;
         bool has_fill_value = get_var_fill_value(var, fill_value);
         if (has_fill_value) {
            for (int idx=0; idx<cell_count; idx++) {
               if(is_eq(data[idx], fill_value)) data[idx] = bad_data_float;
            }
         }
      }
      else {
         int unpacked_count = 0;
         float add_offset = get_var_add_offset(var);
         float scale_factor = get_var_scale_factor(var);
         bool do_scale_factor = has_scale_factor_attr(var) || has_add_offset_attr(var);
         bool unsigned_value = has_unsigned_attribute(var);
         mlog << Debug(6) << method_name << GET_NC_NAME_P(var)
              << " data_size=" << cell_count << ", is_unsigned_value: "
              << unsigned_value << "\n";
         if (do_scale_factor) {
            mlog << Debug(6) << method_name << GET_NC_NAME_P(var)
                 << " add_offset = " << add_offset
                 << ", scale_factor=" << scale_factor << "\n";
         }

         switch ( type_id )  {
            case NcType::nc_DOUBLE:
               {
                  double *packed_data = new double[cell_count];

                  get_nc_data_t(var, packed_data);

                  double fill_value;
                  bool has_fill_value = get_var_fill_value(var, fill_value);
                  for (int idx=0; idx<cell_count; idx++) {
                     if(has_fill_value && is_eq(data[idx], fill_value))
                        data[idx] = bad_data_float;
                     else data[idx] = (float)packed_data[idx];
                  }
                  delete [] packed_data;
               }
               break;
            case NcType::nc_INT64:
               {
                  long long *packed_data = new long long[cell_count];

                  var->getVar(packed_data);
                  copy_nc_data_(var, data, packed_data, cell_count,
                                "int64", add_offset, scale_factor);
                  delete [] packed_data;
               }
               break;
            case NcType::nc_INT:
               {
                  int *packed_data = new int[cell_count];

                  var->getVar(packed_data);
                  copy_nc_data_(var, data, packed_data, cell_count,
                                "int", add_offset, scale_factor);
                  delete [] packed_data;
               }
               break;
            case NcType::nc_SHORT:
               {
                  short missing_value;
                  bool has_missing = get_var_fill_value(var, missing_value);
                  short *packed_data = new short[cell_count];

                  var->getVar(packed_data);
                  if (unsigned_value) {
                     unsigned short *ushort_data = new unsigned short[cell_count];
                     for (int idx=0; idx<cell_count; idx++) {
                        ushort_data[idx] =(unsigned short)packed_data[idx];
                     }
                     copy_nc_data_t(var, data, ushort_data, cell_count, 
                                    "ushort", add_offset, scale_factor,
                                    has_missing, (unsigned short)missing_value);
                     delete [] ushort_data;
                  }
                  else {
                     copy_nc_data_t(var, data, packed_data, cell_count, 
                                    "short", add_offset, scale_factor,
                                    has_missing, missing_value);
                  }
                  delete [] packed_data;
               }
               break;
            case NcType::nc_USHORT:
               {
                  unsigned short *packed_data = new unsigned short[cell_count];

                  var->getVar(packed_data);
                  copy_nc_data_(var, data, packed_data, cell_count,
                                "unsigned short", add_offset, scale_factor);
                  delete [] packed_data;
               }
               break;
            case NcType::nc_BYTE:
               {
                  ncbyte missing_value;
                  bool has_missing = get_var_fill_value(var, missing_value);
                  ncbyte *packed_data = new ncbyte[cell_count];

                  var->getVar(packed_data);
                  if (unsigned_value) {
                     unsigned char *ubyte_data = new unsigned char[cell_count];
                     for (int idx=0; idx<cell_count; idx++) {
                        ubyte_data[idx] =(unsigned char)packed_data[idx];
                     }
                     copy_nc_data_t(var, data, ubyte_data, cell_count,
                                    "ncubyte", add_offset, scale_factor,
                                    has_missing, (unsigned char)missing_value);
                     delete [] ubyte_data;
                  }
                  else {
                     copy_nc_data_t(var, data, packed_data, cell_count,
                                    "ncbyte", add_offset, scale_factor,
                                    has_missing, missing_value);
                  }
                  delete [] packed_data;
               }
               break;
            case NcType::nc_UBYTE:
               {
                  unsigned char *packed_data = new unsigned char[cell_count];

                  var->getVar(packed_data);
                  copy_nc_data_(var, data, packed_data, cell_count,
                                "unsigned char", add_offset, scale_factor);
                  delete [] packed_data;
               }
               break;
            default:
               return_status = false;
               mlog << Debug(1) << method_name << "Did not read data because of unsupported data type ("
                    << type_id << ", type name: " << GET_NC_TYPE_NAME_P(var)
                    << ") for " << GET_NC_NAME_P(var) << "\n";
         }
      }
   }

   mlog << Debug(6) << method_name << "took "
        << (clock()-start_clock)/double(CLOCKS_PER_SEC)
        << " seconds for " << GET_NC_NAME_P(var) << "\n";
   return(return_status);
}

////////////////////////////////////////////////////////////////////////

bool get_nc_data(NcVar *var, float *data, const long *curs) {
   bool return_status = get_nc_data_(var, data, bad_data_float, curs);

   return(return_status);
}

////////////////////////////////////////////////////////////////////////

bool get_nc_data(NcVar *var, float *data, const long *dims, const long *curs) {
   bool return_status = get_nc_data_(var, data, bad_data_float, dims, curs);

   return(return_status);
}

////////////////////////////////////////////////////////////////////////

bool get_nc_data(NcVar *var, float *data, const long dim, const long cur) {
   bool return_status = get_nc_data_(var, data, bad_data_float, dim, cur);

   return(return_status);
}

////////////////////////////////////////////////////////////////////////

bool get_nc_data(NcFile *nc, const char *var_name, double *data,
                 const long *dims, const long *curs) {

   //
   // Retrieve the input variables
   //
   NcVar var = get_var(nc, var_name);
   return get_nc_data(&var, data, dims, curs);
}

////////////////////////////////////////////////////////////////////////

bool get_nc_data(NcVar *var, double *data) {
   bool return_status = false;
   static const char *method_name = "get_nc_data(NcVar *, double *) ";

   if (IS_VALID_NC_P(var)) {
      //
      // Retrieve the double value from the NetCDF variable.
      // Note: missing data was checked here
      //
      int unpacked_count = 0;
      int type_id = GET_NC_TYPE_ID_P(var);
      const int cell_count = get_data_size(var);

      return_status = true;
      if (NcType::nc_DOUBLE == type_id) {

         var->getVar(data);

         double fill_value;
         bool has_fill_value = get_var_fill_value(var, fill_value);
         if (has_fill_value) {
            for (int idx=0; idx<cell_count; idx++) {
               if(is_eq(data[idx], fill_value)) data[idx] = bad_data_double;
            }
         }
      }
      else {
         bool unsigned_value = has_unsigned_attribute(var);
         const double add_offset = get_var_add_offset(var);
         const double scale_factor = get_var_scale_factor(var);
         bool do_scale_factor = has_scale_factor_attr(var) || has_add_offset_attr(var);
         mlog << Debug(6) << method_name << GET_NC_NAME_P(var)
              << " data_size=" << cell_count << ", is_unsigned_value: "
              << unsigned_value << "\n";
         if (do_scale_factor) {
            mlog << Debug(6) << method_name << GET_NC_NAME_P(var)
                 << " add_offset = " << add_offset
                 << ", scale_factor=" << scale_factor << "\n";
         }

         switch ( type_id )  {
            case NcType::nc_FLOAT:
               {
                  float *packed_data = new float[cell_count];

                  var->getVar(packed_data);

                  float fill_value;
                  bool has_fill_value = get_var_fill_value(var, fill_value);
                  for (int idx=0; idx<cell_count; idx++) {
                     if (has_fill_value && is_eq(data[idx], fill_value))
                        data[idx] = bad_data_double;
                     else data[idx] = (double)packed_data[idx];
                  }
                  delete [] packed_data;
               }
               break;
            case NcType::nc_INT64:
               {
                  long long *packed_data = new long long[cell_count];

                  var->getVar(packed_data);
                  copy_nc_data_(var, data, packed_data, cell_count,
                                "int64", add_offset, scale_factor);
                  delete [] packed_data;
               }
               break;
            case NcType::nc_INT:
               {
                  int *packed_data = new int[cell_count];

                  var->getVar(packed_data);
                  copy_nc_data_(var, data, packed_data, cell_count,
                                "int", add_offset, scale_factor);
                  delete [] packed_data;
               }
               break;
            case NcType::nc_SHORT:
               {
                  short missing_value;
                  bool has_missing = get_var_fill_value(var, missing_value);
                  short *packed_data = new short[cell_count];
                  var->getVar(packed_data);
                  if (unsigned_value) {
                     unsigned short *ushort_data = new unsigned short[cell_count];
                     for (int idx=0; idx<cell_count; idx++) {
                        ushort_data[idx] =(unsigned short)packed_data[idx];
                     }
                     copy_nc_data_t(var, data, ushort_data, cell_count, 
                                    "ushort", add_offset, scale_factor,
                                    has_missing, (unsigned short)missing_value);
                     delete [] ushort_data;
                  }
                  else {
                     copy_nc_data_t(var, data, packed_data, cell_count, 
                                    "short", add_offset, scale_factor,
                                    has_missing, missing_value);
                  }
                  delete [] packed_data;
               }
               break;
            case NcType::nc_USHORT:
               {
                  unsigned short *packed_data = new unsigned short[cell_count];

                  var->getVar(packed_data);
                  copy_nc_data_(var, data, packed_data, cell_count,
                                "ushort", add_offset, scale_factor);
                  delete [] packed_data;
               }
               break;
            case NcType::nc_BYTE:
               {
                  ncbyte missing_value;
                  bool has_missing = get_var_fill_value(var, missing_value);
                  ncbyte *packed_data = new ncbyte[cell_count];

                  var->getVar(packed_data);
                  if (unsigned_value) {
                     unsigned char *ubyte_data = new unsigned char[cell_count];
                     for (int idx=0; idx<cell_count; idx++) {
                        ubyte_data[idx] =(unsigned char)packed_data[idx];
                     }
                     copy_nc_data_t(var, data, ubyte_data, cell_count,
                                    "ncubyte", add_offset, scale_factor,
                                    has_missing, (unsigned char)missing_value);
                     delete [] ubyte_data;
                  }
                  else {
                     copy_nc_data_t(var, data, packed_data, cell_count,
                                    "ncbyte", add_offset, scale_factor,
                                    has_missing, missing_value);
                  }
                  delete [] packed_data;
               }
               break;
            case NcType::nc_UBYTE:
               {
                  unsigned char *packed_data = new unsigned char[cell_count];

                  var->getVar(packed_data);
                  copy_nc_data_(var, data, packed_data, cell_count,
                                "ncubyte", add_offset, scale_factor);
                  delete [] packed_data;
               }
               break;
            default:
                 return_status = false;
                 mlog << Debug(1) << method_name << "Did not read data because of unsupported data type ("
                      << type_id << ", type name: " << GET_NC_TYPE_NAME_P(var)
                      << ") for " << GET_NC_NAME_P(var) << "\n";

         }
      }
   }
   return(return_status);
}

////////////////////////////////////////////////////////////////////////

bool get_nc_data(NcVar *var, double *data, const long *curs) {
   bool return_status = get_nc_data_(var, data, bad_data_double, curs);
   return(return_status);
}

////////////////////////////////////////////////////////////////////////

bool get_nc_data(NcVar *var, double *data, const long dim, const long cur) {
   bool return_status = get_nc_data_(var, data, bad_data_double, dim, cur);;

   return(return_status);
}

////////////////////////////////////////////////////////////////////////

bool get_nc_data(NcVar *var, double *data, const long *dims, const long *curs) {
   bool return_status = get_nc_data_(var, data, bad_data_double, dims, curs);

   return(return_status);
}

////////////////////////////////////////////////////////////////////////

bool get_nc_data(NcVar *var, char *data) {
   bool return_status = get_nc_data_t(var, data);

   return(return_status);
}

////////////////////////////////////////////////////////////////////////

bool get_nc_data(NcVar *var, uchar *data) {
   bool return_status = false;
   int data_type = GET_NC_TYPE_ID_P(var);
   static const char *method_name = "get_nc_data(NcVar *, uchar *) -> ";
   if (NC_UBYTE == data_type) return_status = get_nc_data_t(var, data);
   else if (NC_BYTE == data_type && has_unsigned_attribute(var)) {
      int cell_count = get_data_size(var);
      ncbyte *signed_data = new ncbyte[cell_count];
      return_status = get_nc_data_t(var, signed_data);
      for (int idx=0; idx<cell_count; idx++) {
         data[idx] = (uchar)signed_data[idx];
      }
      delete [] signed_data;
   }
   else {
      mlog << Error << "\n" << method_name
           << "does not process \"" << GET_NC_TYPE_NAME_P(var)
           << "\" data type for variable \"" << GET_NC_NAME_P(var) << "\".\n\n";
      exit(1);
   }

   // Do not aplly scale_factor & add_offset to byte/char array output
   return(return_status);
}

////////////////////////////////////////////////////////////////////////

bool get_nc_data(NcVar *var, unsigned short *data) {
   bool return_status = false;
   int data_type = GET_NC_TYPE_ID_P(var);
   static const char *method_name = "get_nc_data(NcVar *, unsigned short *) -> ";
   if (NC_USHORT == data_type) return_status = get_nc_data_t(var, data);
   else if (NC_SHORT == data_type && has_unsigned_attribute(var)) {
      int cell_count = get_data_size(var);
      short fill_value = (short)bad_data_int;
      NcVarAtt *att_fill_value  = get_nc_att(var, (string)"_FillValue");
      bool has_fill_value = IS_VALID_NC_P(att_fill_value);
      if (has_fill_value) fill_value = get_att_value_int(att_fill_value);

      short *short_data = new short[cell_count];
      return_status = get_nc_data_t(var, short_data);
      for (int idx=0; idx<cell_count; idx++) {
         if (has_fill_value && fill_value == short_data[idx])
            data[idx] = (unsigned short)bad_data_int;
         else
            data[idx] = (unsigned short)short_data[idx];
      }
      delete [] short_data;
      if (att_fill_value) delete att_fill_value;
   }
   else {
      mlog << Error << "\n" << method_name
           << "does not process \"" << GET_NC_TYPE_NAME_P(var)
           << "\" data type for variable \"" << GET_NC_NAME_P(var) << "\".\n\n";
      exit(1);
   }

   // Do not aplly scale_factor & add_offset to short array output
   return(return_status);
}

////////////////////////////////////////////////////////////////////////

bool get_nc_data(NcFile *nc, const char *var_name, char *data,
                 const long *dims, const long *curs) {

   //
   // Retrieve the input variables
   //
   NcVar var = get_var(nc, var_name);
   return get_nc_data(&var, data, dims, curs);
}

////////////////////////////////////////////////////////////////////////

bool get_nc_data(NcVar *var, char *data, const long dim, const long cur) {
   bool return_status = get_nc_data_(var, data, bad_data_char, dim, cur);

   return(return_status);
}

////////////////////////////////////////////////////////////////////////

bool get_nc_data(NcVar *var, char *data, const long *dims, const long *curs) {
   bool return_status = get_nc_data_(var, data, bad_data_char, dims, curs);

   return(return_status);
}

////////////////////////////////////////////////////////////////////////

/*
bool get_nc_data(NcVar *var, ncbyte *data) {
   bool return_status = get_nc_data_no_scale_T(var, data);

   return(return_status);
}
*/

////////////////////////////////////////////////////////////////////////

bool get_nc_data(NcFile *nc, const char *var_name, ncbyte *data,
                 const long *dims, const long *curs) {

   //
   // Retrieve the input variables
   //
   NcVar var = get_var(nc, var_name);
   return get_nc_data(&var, data, dims, curs);
}

////////////////////////////////////////////////////////////////////////

bool get_nc_data(NcVar *var, ncbyte *data, const long dim, const long cur) {
   bool return_status = get_nc_data_(var, data, (ncbyte)bad_data_char, dim, cur);

   return(return_status);
}

////////////////////////////////////////////////////////////////////////

bool get_nc_data(NcVar *var, ncbyte *data, const long *dims, const long *curs) {
   bool return_status = get_nc_data_(var, data, (ncbyte)bad_data_char, dims, curs);

   return(return_status);
}

////////////////////////////////////////////////////////////////////////
// returns matching offset or bad_data_int if not found

int get_index_at_nc_data(NcVar *var, double value, const string dim_name, bool is_time) {
   int offset = bad_data_int;
   static const char *method_name = "get_index_at_nc_data() -> ";
   if (IS_VALID_NC_P(var)) {
      int data_size = get_data_size(var);
      double *values = new double[data_size];
      
      if (get_nc_data(var, values)) {
         unixtime ut;
         int sec_per_unit;
         bool no_leap_year = get_att_no_leap_year(var);
         ut = sec_per_unit = 0;
         if (is_time) {
            ConcatString units;
            bool has_attr = get_var_units(var, units);
            if (has_attr && (0 < units.length()))
                parse_cf_time_string(units.c_str(), ut, sec_per_unit);
            else {
               mlog << Warning << "\n" << method_name
                    << "the time variable \"" << GET_NC_NAME_P(var)
                    << "\" must contain a \""
                    << units_att_name << "\" attribute.\n\n";
            }
         }
         for (int idx=0; idx<data_size; idx++) {
            if (is_eq(values[idx], value)) {
               offset = idx;
               break;
            }
            if (is_time) {
               if (is_eq(add_to_unixtime(ut, sec_per_unit, values[idx], no_leap_year), value)) {
                  offset = idx;
                  break;
               }
            }
         }
      }
      if (values) delete [] values;

      ConcatString value_str;
      if (is_time && (value > 10000000.)) value_str << unix_to_yyyymmdd_hhmmss(value);
      else value_str << value;
      if (offset == bad_data_int)
         mlog << Debug(7) << method_name << "Not found value " << value_str
              << " at " << GET_NC_NAME_P(var)
              << " by dimension name \"" << dim_name << "\"\n";
      else
         mlog << Debug(7) << method_name << "Found value " << value_str
              << " (index=" << offset << ") at " << GET_NC_NAME_P(var)
              << " by dimension name \"" << dim_name << "\"\n";
   }
   else {
      mlog << Debug(7) << method_name << "Not found a dimension variable for \""
           << dim_name << "\"\n";
   }
   return(offset);
}

////////////////////////////////////////////////////////////////////////

bool get_nc_data_to_array(NcVar *var, StringArray *array_buf) {
   bool result = false;
   static const char *method_name = "get_nc_data_to_array(NcVar) -> ";
   if (IS_INVALID_NC_P(var)) {
      mlog << Error << "\n" << method_name << "the variable does not exist!\n\n";
   }
   else {
      int dim_count = var->getDimCount();
      if (2 != dim_count) {
         mlog << Error << "\n" << method_name
              << "Invalid dimensions " <<  dim_count << " for "
              << GET_NC_NAME_P(var) << "\n\n";
      }
      else {
         long offsets[2] = { 0, 0 };
         long lengths[2] = { 1, 1 };
         NcDim count_dim = var->getDim(dim_count-2);
         NcDim str_dim = var->getDim(dim_count-1);
         int count = get_dim_size(&count_dim);
         int str_len = get_dim_size(&str_dim);
         lengths[1] = str_len;
         char str_buffer[str_len+1];
         result = true;
         for (int idx=0; idx<count; idx++) {
            if(!get_nc_data(var, str_buffer, lengths, offsets)) {
               result = false;
               break;
            }
            else {
               array_buf->add(str_buffer);
            }
            offsets[0]++;
         }
      }
   }
   return result;
}

////////////////////////////////////////////////////////////////////////

bool get_nc_data_to_array(NcFile *nc_in, const char *var_name,
                          StringArray *array_buf) {
   bool result = false;
   static const char *method_name = "get_nc_data_to_array(NcFile) -> ";
   NcVar obs_var = get_nc_var(nc_in, var_name);
   if (IS_INVALID_NC(obs_var)) {
      mlog << Error << "\n" << method_name << "the variable \"" << var_name
           << "\" does not exist!\n\n";
   }
   else {
      result = get_nc_data_to_array(&obs_var, array_buf);
   }
   return result;
}

///////////////////////////////////////////////////////////////////////////////

int get_nc_string_length(NcVar *var) {
   int str_length = 0;
   if (IS_VALID_NC_P(var)) {
      int dim_count = var->getDimCount();
      NcDim str_dim = var->getDim(dim_count-1);
      if (IS_VALID_NC(str_dim)) str_length = get_dim_size(&str_dim);
   }
   return str_length;
}

///////////////////////////////////////////////////////////////////////////////

int get_nc_string_length(NcFile *nc_in, const char *var_name) {
   int str_length = 0;
   NcVar obs_var = get_nc_var(nc_in, var_name);
   str_length = get_nc_string_length(&obs_var);
   return str_length;
}

///////////////////////////////////////////////////////////////////////////////

int get_nc_string_length(NcFile *nc_file, NcVar var, const char *var_name) {
   int string_len = IS_INVALID_NC(var)
                        ? get_nc_string_length(nc_file, var_name)
                        : get_nc_string_length(&var);
   return string_len;
}

////////////////////////////////////////////////////////////////////////

bool put_nc_data(NcVar *var, const int data, long offset0, long offset1, long offset2) {
   return put_nc_data_T(var, data, offset0, offset1, offset2);
}

////////////////////////////////////////////////////////////////////////

bool put_nc_data(NcVar *var, const char data, long offset0, long offset1, long offset2) {
   return put_nc_data_T(var, data, offset0, offset1, offset2);
}

////////////////////////////////////////////////////////////////////////

bool put_nc_data(NcVar *var, const float data , long offset0, long offset1, long offset2) {
   return put_nc_data_T(var, data, offset0, offset1, offset2);
}

////////////////////////////////////////////////////////////////////////

bool put_nc_data(NcVar *var, const double data, long offset0, long offset1, long offset2) {
   return put_nc_data_T(var, data, offset0, offset1, offset2);
}

////////////////////////////////////////////////////////////////////////

bool put_nc_data(NcVar *var, const ncbyte data, long offset0, long offset1, long offset2) {
   return put_nc_data_T(var, data, offset0, offset1, offset2);
}

////////////////////////////////////////////////////////////////////////

bool put_nc_data(NcVar *var, const int *data    ) {
   var->putVar(data);
   return true;
}

////////////////////////////////////////////////////////////////////////

bool put_nc_data(NcVar *var, const char *data   ) {
   var->putVar(data);
   return true;
}

////////////////////////////////////////////////////////////////////////

bool put_nc_data(NcVar *var, const float *data  ) {
   var->putVar(data);
   return true;
}

////////////////////////////////////////////////////////////////////////

bool put_nc_data(NcVar *var, const double *data ) {
   var->putVar(data);
   return true;
}

////////////////////////////////////////////////////////////////////////

bool put_nc_data(NcVar *var, const ncbyte *data ) {
   var->putVar(data);
   return true;
}

////////////////////////////////////////////////////////////////////////

bool put_nc_data(NcVar *var, const int *data,    const long length, const long offset) {
   put_nc_data_T(var, data, length, offset);
   return true;
}

////////////////////////////////////////////////////////////////////////

bool put_nc_data(NcVar *var, const char *data,   const long length, const long offset) {
   put_nc_data_T(var, data, length, offset);
   return true;
}

////////////////////////////////////////////////////////////////////////

bool put_nc_data(NcVar *var, const float *data , const long length, const long offset) {
   put_nc_data_T(var, data, length, offset);
   return true;
}

////////////////////////////////////////////////////////////////////////

bool put_nc_data(NcVar *var, const double *data, const long length, const long offset) {
   put_nc_data_T(var, data, length, offset);
   return true;
}

////////////////////////////////////////////////////////////////////////

bool put_nc_data(NcVar *var, const ncbyte *data, const long length, const long offset) {
   put_nc_data_T(var, data, length, offset);
   return true;
}

////////////////////////////////////////////////////////////////////////

bool put_nc_data(NcVar *var, const float *data , const long *lengths, const long *offsets) {
   put_nc_data_T(var, data , lengths, offsets);
   return true;
}

////////////////////////////////////////////////////////////////////////

bool put_nc_data(NcVar *var, const char *data , const long *lengths, const long *offsets) {
   put_nc_data_T(var, data , lengths, offsets);
   return true;
}

////////////////////////////////////////////////////////////////////////

bool put_nc_data(NcVar *var, const int *data , const long *lengths, const long *offsets) {
   put_nc_data_T(var, data , lengths, offsets);
   return true;
}

////////////////////////////////////////////////////////////////////////

bool put_nc_data_with_dims(NcVar *var, const int *data,
                           const int len0, const int len1, const int len2) {
   return put_nc_data_with_dims(var, data, (long)len0, (long)len1, (long)len2);
}

////////////////////////////////////////////////////////////////////////

bool put_nc_data_with_dims(NcVar *var, const int *data,
                           const long len0, const long len1, const long len2) {
   put_nc_data_T_with_dims(var, data, len0, len1, len2);
   return true;
}

////////////////////////////////////////////////////////////////////////

bool put_nc_data_with_dims(NcVar *var, const float *data,
                           const int len0, const int len1, const int len2) {
   return put_nc_data_with_dims(var, data, (long)len0, (long)len1, (long)len2);
}

////////////////////////////////////////////////////////////////////////

bool put_nc_data_with_dims(NcVar *var, const float *data,
                           const long len0, const long len1, const long len2) {
   put_nc_data_T_with_dims(var, data, len0, len1, len2);
   return true;
}

////////////////////////////////////////////////////////////////////////

bool put_nc_data_with_dims(NcVar *var, const double *data,
                           const int len0, const int len1, const int len2) {
   return put_nc_data_with_dims(var, data, (long)len0, (long)len1, (long)len2);
}

////////////////////////////////////////////////////////////////////////

bool put_nc_data_with_dims(NcVar *var, const double *data,
                           const long len0, const long len1, const long len2) {
   put_nc_data_T_with_dims(var, data, len0, len1, len2);
   return true;
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

NcVar get_var(NcFile *nc, const char *var_name) {
   string new_var_name = var_name;
   patch_nc_name(&new_var_name);

   //
   // Retrieve the variable from the NetCDF file.
   //
   NcVar var;
   multimap<string,NcVar> varMap = GET_NC_VARS_P(nc);
   multimap<string,NcVar>::iterator it = varMap.find(new_var_name);
   if (it != varMap.end()) {
      NcVar tmpVar = it->second;
      if(IS_INVALID_NC(tmpVar)) {
         mlog << Error << "\nget_var() -> "
              << "can't read \"" << new_var_name << "\" variable.\n\n";
         exit(1);
      }

      var = tmpVar;
   }

   return(var);
}

////////////////////////////////////////////////////////////////////////

NcVar get_nc_var(NcFile *nc, const char *var_name, bool log_as_error) {
   string new_var_name = var_name;
   patch_nc_name(&new_var_name);

   //
   // Retrieve the variable from the NetCDF file.
   //
   NcVar var = nc->getVar(new_var_name);
   if(IS_INVALID_NC(var)) {
      ConcatString log_message;
      log_message << "\nget_nc_var(NcFile) --> The variable \""
                  << new_var_name << "\" does not exist!\n\n";
      if (log_as_error)
         mlog << Error << log_message;
      else
         mlog << Warning << log_message;
   }

   return(var);
}

////////////////////////////////////////////////////////////////////////

void copy_nc_att_byte(NcFile *nc_to, NcGroupAtt *from_att) {
   size_t att_length = from_att->getAttLength();
   ncbyte value[att_length];
   from_att->getValues((void *)&value);
   nc_to->putAtt(GET_NC_NAME_P(from_att), from_att->getType(), att_length, (void *)value);
}

void copy_nc_att_char(NcFile *nc_to, NcGroupAtt *from_att) {
   size_t att_length = from_att->getAttLength();
   char value[att_length];
   from_att->getValues((void *)&value);
   nc_to->putAtt(GET_NC_NAME_P(from_att), from_att->getType(), att_length, (void *)value);
}

void copy_nc_att_double(NcFile *nc_to, NcGroupAtt *from_att) {
   size_t att_length = from_att->getAttLength();
   if (att_length == 1) {
      double value;
      from_att->getValues(&value);
      nc_to->putAtt(GET_NC_NAME_P(from_att), from_att->getType(), value);
   }
   else {
      double values[att_length];
      from_att->getValues(&values);
      nc_to->putAtt(GET_NC_NAME_P(from_att), from_att->getType(), att_length, values);
   }
}

void copy_nc_att_float(NcFile *nc_to, NcGroupAtt *from_att) {
   size_t att_length = from_att->getAttLength();
   if (att_length == 1) {
      float value;
      from_att->getValues(&value);
      nc_to->putAtt(GET_NC_NAME_P(from_att), from_att->getType(), value);
   }
   else {
      float values[att_length];
      from_att->getValues(&values);
      nc_to->putAtt(GET_NC_NAME_P(from_att), from_att->getType(), att_length, values);
   }
}

void copy_nc_att_int(NcFile *nc_to, NcGroupAtt *from_att) {
   size_t att_length = from_att->getAttLength();
   if (att_length == 1) {
      int value;
      from_att->getValues(&value);
      nc_to->putAtt(GET_NC_NAME_P(from_att), from_att->getType(), value);
   }
   else {
      int values[att_length];
      from_att->getValues(&values);
      nc_to->putAtt(GET_NC_NAME_P(from_att), from_att->getType(), att_length, values);
   }
}

void copy_nc_att_int64(NcFile *nc_to, NcGroupAtt *from_att) {
   size_t att_length = from_att->getAttLength();
   if (att_length == 1) {
      long long value;
      from_att->getValues(&value);
      nc_to->putAtt(GET_NC_NAME_P(from_att), from_att->getType(), value);
   }
   else {
      long long values[att_length];
      from_att->getValues(&values);
      nc_to->putAtt(GET_NC_NAME_P(from_att), from_att->getType(), att_length, values);
   }
}

void copy_nc_att_short(NcFile *nc_to, NcGroupAtt *from_att) {
   size_t att_length = from_att->getAttLength();
   if (att_length == 1) {
      short value;
      from_att->getValues(&value);
      nc_to->putAtt(GET_NC_NAME_P(from_att), from_att->getType(), value);
   }
   else {
      short values[att_length];
      from_att->getValues(&values);
      nc_to->putAtt(GET_NC_NAME_P(from_att), from_att->getType(), att_length, values);
   }
}

////////////////////////////////////////////////////////////////////////

void copy_nc_att_char(NcVar *var_to, NcGroupAtt *from_att) {
   size_t att_length = from_att->getAttLength();
   char value[att_length];
   from_att->getValues((void *)&value);
   var_to->putAtt(GET_NC_NAME_P(from_att), from_att->getType(), att_length, (void *)value);
}

void copy_nc_att_double(NcVar *var_to, NcGroupAtt *from_att) {
   size_t att_length = from_att->getAttLength();
   if (att_length == 1) {
      double value;
      from_att->getValues(&value);
      var_to->putAtt(GET_NC_NAME_P(from_att), from_att->getType(), value);
   }
   else {
      double values[att_length];
      from_att->getValues(&values);
      var_to->putAtt(GET_NC_NAME_P(from_att), from_att->getType(), att_length, values);
   }
}
void copy_nc_att_float(NcVar *var_to, NcGroupAtt *from_att) {
   size_t att_length = from_att->getAttLength();
   if (att_length == 1) {
      float value;
      from_att->getValues(&value);
      var_to->putAtt(GET_NC_NAME_P(from_att), from_att->getType(), value);
   }
   else {
      float values[att_length];
      from_att->getValues(&values);
      var_to->putAtt(GET_NC_NAME_P(from_att), from_att->getType(), att_length, values);
   }
}

void copy_nc_att_int(NcVar *var_to, NcGroupAtt *from_att) {
   size_t att_length = from_att->getAttLength();
   if (att_length == 1) {
      int value;
      from_att->getValues(&value);
      var_to->putAtt(GET_NC_NAME_P(from_att), from_att->getType(), value);
   }
   else {
      int values[att_length];
      from_att->getValues(&values);
      var_to->putAtt(GET_NC_NAME_P(from_att), from_att->getType(), att_length, values);
   }
}

void copy_nc_att_int64(NcVar *var_to, NcGroupAtt *from_att) {
   size_t att_length = from_att->getAttLength();
   if (att_length == 1) {
      long long value;
      from_att->getValues(&value);
      var_to->putAtt(GET_NC_NAME_P(from_att), from_att->getType(), value);
   }
   else {
      long long values[att_length];
      from_att->getValues(&values);
      var_to->putAtt(GET_NC_NAME_P(from_att), from_att->getType(), att_length, values);
   }
}

void copy_nc_att_short(NcVar *var_to, NcGroupAtt *from_att) {
   size_t att_length = from_att->getAttLength();
   if (att_length == 1) {
      short value;
      from_att->getValues(&value);
      var_to->putAtt(GET_NC_NAME_P(from_att), from_att->getType(), value);
   }
   else {
      short values[att_length];
      from_att->getValues(&values);
      var_to->putAtt(GET_NC_NAME_P(from_att), from_att->getType(), att_length, values);
   }
}

////////////////////////////////////////////////////////////////////////

void copy_nc_att_byte(NcVar *var_to, NcVarAtt *from_att) {
   size_t att_length = from_att->getAttLength();
   ncbyte value[att_length];
   from_att->getValues((void *)&value);
   var_to->putAtt(GET_NC_NAME_P(from_att), from_att->getType(), att_length, (void *)value);
}

void copy_nc_att_char(NcVar *var_to, NcVarAtt *from_att) {
   size_t att_length = from_att->getAttLength();
   char value[att_length];
   from_att->getValues((void *)&value);
   var_to->putAtt(GET_NC_NAME_P(from_att), from_att->getType(), att_length, (void *)value);
}

void copy_nc_att_double(NcVar *var_to, NcVarAtt *from_att) {
   size_t att_length = from_att->getAttLength();
   if (att_length == 1) {
      double value;
      from_att->getValues(&value);
      var_to->putAtt(GET_NC_NAME_P(from_att), from_att->getType(), value);
   }
   else {
      double values[att_length];
      from_att->getValues(&values);
      var_to->putAtt(GET_NC_NAME_P(from_att), from_att->getType(), att_length, values);
   }
}

void copy_nc_att_float(NcVar *var_to, NcVarAtt *from_att) {
   size_t att_length = from_att->getAttLength();
   if (att_length == 1) {
      float value;
      from_att->getValues(&value);
      var_to->putAtt(GET_NC_NAME_P(from_att), from_att->getType(), value);
   }
   else {
      float values[att_length];
      from_att->getValues(&values);
      var_to->putAtt(GET_NC_NAME_P(from_att), from_att->getType(), att_length, values);
   }
}

void copy_nc_att_int(NcVar *var_to, NcVarAtt *from_att) {
   size_t att_length = from_att->getAttLength();
   if (att_length == 1) {
      int value;
      from_att->getValues(&value);
      var_to->putAtt(GET_NC_NAME_P(from_att), from_att->getType(), value);
   }
   else {
      int values[att_length];
      from_att->getValues(&values);
      var_to->putAtt(GET_NC_NAME_P(from_att), from_att->getType(), att_length, values);
   }
}

void copy_nc_att_int64(NcVar *var_to, NcVarAtt *from_att) {
   size_t att_length = from_att->getAttLength();
   if (att_length == 1) {
      long long value;
      from_att->getValues(&value);
      var_to->putAtt(GET_NC_NAME_P(from_att), from_att->getType(), value);
   }
   else {
      long long values[att_length];
      from_att->getValues(&values);
      var_to->putAtt(GET_NC_NAME_P(from_att), from_att->getType(), att_length, values);
   }
}

void copy_nc_att_short(NcVar *var_to, NcVarAtt *from_att) {
   size_t att_length = from_att->getAttLength();
   if (att_length == 1) {
      short value;
      from_att->getValues(&value);
      var_to->putAtt(GET_NC_NAME_P(from_att), from_att->getType(), value);
   }
   else {
      short values[att_length];
      from_att->getValues(&values);
      var_to->putAtt(GET_NC_NAME_P(from_att), from_att->getType(), att_length, values);
   }
}


NcVar *copy_nc_var(NcFile *to_nc, NcVar *from_var,
      const int deflate_level, const bool all_attrs) {
   vector<NcDim> dims = from_var->getDims();
   for(unsigned int idx=0; idx<dims.size(); idx++) {
      NcDim dim = dims[idx];
      if (!has_dim(to_nc, GET_NC_NAME(dim).c_str())) {
         add_dim(to_nc, GET_NC_NAME(dim), dim.getSize());
      }
   }
   NcVar tmp_var = add_var(to_nc, GET_NC_NAME_P(from_var),
         from_var->getType(), dims, deflate_level);
   NcVar *to_var = new NcVar(tmp_var);
   copy_nc_atts(from_var, to_var, all_attrs);
   copy_nc_var_data(from_var, to_var);
   return to_var;
}

////////////////////////////////////////////////////////////////////////

void copy_nc_att(NcFile *nc_from, NcVar *var_to, const ConcatString attr_name) {
   NcGroupAtt *from_att = get_nc_att(nc_from, attr_name);
   if (IS_VALID_NC_P(from_att)) {
      int dataType = GET_NC_TYPE_ID_P(from_att);
      switch (dataType) {
      case NC_DOUBLE:
         copy_nc_att_double(var_to, from_att);
         break;
      case NC_FLOAT:
         copy_nc_att_float(var_to, from_att);
         break;
      case NC_SHORT:
         copy_nc_att_short(var_to, from_att);
         break;
      case NC_INT:
         copy_nc_att_int(var_to, from_att);
         break;
      case NC_INT64:
         copy_nc_att_int64(var_to, from_att);
         break;
      case NC_CHAR:
         copy_nc_att_char(var_to, from_att);
         break;
      default:
         mlog << Error << "\ncopy_nc_att(NcFile, NcVar, attr_name) -> "
              << "Does not copy this type \"" << dataType << "\" global NetCDF attribute.\n\n";
         exit(1);
      }
   }
   if(from_att) delete from_att;
}

////////////////////////////////////////////////////////////////////////

void copy_nc_att(NcVar *var_from, NcVar *var_to, const ConcatString attr_name) {
   NcVarAtt *from_att = get_nc_att(var_from, attr_name);
   if (IS_VALID_NC_P(from_att)) {
      int dataType = GET_NC_TYPE_ID_P(from_att);
      switch (dataType) {
      case NC_DOUBLE:
         copy_nc_att_double(var_to, from_att);
         break;
      case NC_FLOAT:
         copy_nc_att_float(var_to, from_att);
         break;
      case NC_SHORT:
         copy_nc_att_short(var_to, from_att);
         break;
      case NC_INT:
         copy_nc_att_int(var_to, from_att);
         break;
      case NC_INT64:
         copy_nc_att_int64(var_to, from_att);
         break;
      case NC_CHAR:
         copy_nc_att_char(var_to, from_att);
         break;
      default:
         mlog << Error << "\ncopy_nc_att(NcVar) -> "
              << "Does not copy this type \"" << dataType << "\" NetCDF attributes from \""
              << GET_NC_TYPE_NAME_P(var_from) << "\".\n\n";
         exit(1);
      }
   }
   if(from_att) delete from_att;
}

////////////////////////////////////////////////////////////////////////

void copy_nc_atts(NcFile *nc_from, NcFile *nc_to, const bool all_attrs) {
   multimap<string,NcGroupAtt> ncAttMap = nc_from->getAtts();
   for (multimap<string,NcGroupAtt>::iterator itr = ncAttMap.begin();
         itr != ncAttMap.end(); ++itr) {
      if (all_attrs ||
            (  (itr->first != "Conventions")
            && (itr->first != "missing_value") ) ) {
         NcGroupAtt *from_att = &(itr->second);
         int dataType = GET_NC_TYPE_ID_P(from_att);
         switch (dataType) {
         case NC_DOUBLE:
            copy_nc_att_double(nc_to, from_att);
            break;
         case NC_FLOAT:
            copy_nc_att_float(nc_to, from_att);
            break;
         case NC_SHORT:
            copy_nc_att_short(nc_to, from_att);
            break;
         case NC_INT:
            copy_nc_att_int(nc_to, from_att);
            break;
         case NC_INT64:
            copy_nc_att_int64(nc_to, from_att);
            break;
         case NC_CHAR:
            copy_nc_att_char(nc_to, from_att);
            break;
         default:
            mlog << Error << "\ncopy_nc_atts(NcFile) -> "
                 << "Does not copy this type \"" << dataType << "\" global NetCDF attributes.\n\n";
            exit(1);
         }
      }
   }
}

////////////////////////////////////////////////////////////////////////

void copy_nc_atts(NcVar *var_from, NcVar *var_to, const bool all_attrs) {
   map<string,NcVarAtt> ncAttMap = var_from->getAtts();
   for (map<string,NcVarAtt>::iterator itr = ncAttMap.begin();
         itr != ncAttMap.end(); ++itr) {
      if (all_attrs ||
            (  (itr->first != "scale_factor")
            && (itr->first != "add_offset")
            && (itr->first != "_FillValue")
            && (itr->first != "_Unsigned")
            && (itr->first != "valid_range")
            && (itr->first != "missing_value")
            && (itr->first != "grid_mapping")
            && (itr->first != "coordinates")
            && (itr->first != "cell_methods")
            && (itr->first != "_Com") ) ) {
         NcVarAtt *from_att = &(itr->second);
         int dataType = GET_NC_TYPE_ID_P(from_att);
         switch (dataType) {
         case NC_DOUBLE:
            copy_nc_att_double(var_to, from_att);
            break;
         case NC_FLOAT:
            copy_nc_att_float(var_to, from_att);
            break;
         case NC_SHORT:
            copy_nc_att_short(var_to, from_att);
            break;
         case NC_INT:
            copy_nc_att_int(var_to, from_att);
            break;
         case NC_INT64:
            copy_nc_att_int64(var_to, from_att);
            break;
         case NC_CHAR:
            copy_nc_att_char(var_to, from_att);
            break;
         case NC_BYTE:
            copy_nc_att_byte(var_to, from_att);
            break;
         default:
            mlog << Error << "\ncopy_nc_atts(NcVar) -> "
                 << "Does not copy this type \"" << dataType << "\" NetCDF attributes from \""
                 << GET_NC_TYPE_NAME_P(var_from) << "\".\n\n";
            exit(1);
         }
      }
   }
}

////////////////////////////////////////////////////////////////////////

void copy_nc_data_char(NcVar *var_from, NcVar *var_to, int data_size) {
   //const string method_name = "copy_nc_data_char";
   char *data = new char[data_size];
   var_from->getVar(data);
   var_to->putVar(data);
   //   mlog << Error << "\n" << method_name << " -> error writing the variable "
   //        << GET_NC_NAME_P(var_to) << " to the netCDF file\n\n";
   //   exit(1);
   delete[] data;
}

////////////////////////////////////////////////////////////////////////

void copy_nc_data_double(NcVar *var_from, NcVar *var_to, int data_size) {
   //const string method_name = "copy_nc_data_double";
   double *data = new double[data_size];
   var_from->getVar(data);
   var_to->putVar(data);
   //   mlog << Error << "\n" << method_name << " -> error writing the variable "
   //        << GET_NC_NAME_P(var_to) << " to the netCDF file\n\n";
   //   exit(1);
   delete[] data;
}

////////////////////////////////////////////////////////////////////////

void copy_nc_data_float(NcVar *var_from, NcVar *var_to, int data_size) {
   //const string method_name = "copy_nc_data_float";
   float *data = new float[data_size];
   var_from->getVar(data);
   var_to->putVar(data);
   //   mlog << Error << "\n" << method_name << " -> error writing the variable "
   //        << GET_NC_NAME_P(var_to) << " to the netCDF file\n\n";
   //   exit(1);
   delete[] data;
}

////////////////////////////////////////////////////////////////////////

void copy_nc_data_int(NcVar *var_from, NcVar *var_to, int data_size) {
   //const string method_name = "copy_nc_data_int";
   int *data = new int[data_size];
   var_from->getVar(data);
   var_to->putVar(data);
   //   mlog << Error << "\n" << method_name << " -> error writing the variable "
   //        << GET_NC_NAME_P(var_to) << " to the netCDF file\n\n";
   //   exit(1);
   delete[] data;
}

////////////////////////////////////////////////////////////////////////

void copy_nc_data_short(NcVar *var_from, NcVar *var_to, int data_size) {
   const string method_name = "copy_nc_data_double";
   short *data = new short[data_size];
   var_from->getVar(data);
   var_to->putVar(data);
   //   mlog << Error << "\n" << method_name << " -> error writing the variable "
   //        << GET_NC_NAME_P(var_to) << " to the netCDF file\n\n";
   //   exit(1);
   delete[] data;
}


void copy_nc_var_data(NcVar *var_from, NcVar *var_to) {
   const string method_name = "copy_nc_var_data()";
   int data_size = get_data_size(var_from);
   int dataType = GET_NC_TYPE_ID_P(var_from);
   switch (dataType) {
   case NC_DOUBLE:
      copy_nc_data_double(var_from, var_to, data_size);
      break;

   case NC_FLOAT:
      copy_nc_data_float(var_from, var_to, data_size);
      break;
   case NC_SHORT:
      copy_nc_data_short(var_from, var_to, data_size);
      break;
   case NC_INT:
      copy_nc_data_int(var_from, var_to, data_size);
      break;

   case NC_CHAR:
      copy_nc_data_char(var_from, var_to, data_size);
      break;

   default:
      mlog << Error << "\n" << method_name << " -> "
           << "Does not copy this type \"" << dataType << "\" NetCDF data from \""
           << GET_NC_TYPE_NAME_P(var_from) << "\".\n\n";
      exit(1);
   }
}

////////////////////////////////////////////////////////////////////////

void copy_nc_var_dims(NcVar *var_from, NcVar *var_to) {
   int dim_count = var_from->getDimCount();
   for (int idx=0; idx<dim_count; idx++) {
      NcDim fromDim = var_from->getDim(idx);
      GET_NC_NAME(fromDim);
   }
}

////////////////////////////////////////////////////////////////////////

bool has_var(NcFile *nc, const char * var_name) {
   NcVar v = get_var(nc, var_name);
   return IS_VALID_NC(v);
}

////////////////////////////////////////////////////////////////////////

NcVar add_var(NcFile *nc, const string &var_name, const NcType ncType, const int deflate_level) {
   vector<NcDim> ncDimVector;
   string new_var_name = var_name;
   patch_nc_name(&new_var_name);
   NcVar var = nc->addVar(new_var_name, ncType, ncDimVector);

   if (deflate_level > 0) {
      mlog << Debug(3) << "    nc_utils.add_var() deflate_level: " << deflate_level << "\n";
      var.setCompression(false, true, deflate_level);
   }
   return var;
}

////////////////////////////////////////////////////////////////////////

NcVar add_var(NcFile *nc, const string &var_name, const NcType ncType,
              const NcDim ncDim, const int deflate_level) {
   string new_var_name = var_name;
   patch_nc_name(&new_var_name);
   NcVar var = nc->addVar(new_var_name, ncType, ncDim);

   if (deflate_level > 0) {
      mlog << Debug(3) << "    nc_utils.add_var() deflate_level: " << deflate_level << "\n";
      var.setCompression(false, true, deflate_level);
   }
   return var;
}

////////////////////////////////////////////////////////////////////////

NcVar add_var(NcFile *nc, const string &var_name, const NcType ncType,
              const NcDim ncDim1, const NcDim ncDim2, const int deflate_level) {
   vector<NcDim> ncDims;
   ncDims.push_back(ncDim1);
   ncDims.push_back(ncDim2);
   return add_var(nc, var_name, ncType, ncDims, deflate_level);
}

////////////////////////////////////////////////////////////////////////

NcVar add_var(NcFile *nc, const string &var_name, const NcType ncType,
              const NcDim ncDim1, const NcDim ncDim2, const NcDim ncDim3, const int deflate_level) {
   vector<NcDim> ncDims;
   ncDims.push_back(ncDim1);
   ncDims.push_back(ncDim2);
   ncDims.push_back(ncDim3);
   return add_var(nc, var_name, ncType, ncDims, deflate_level);
}

////////////////////////////////////////////////////////////////////////

NcVar add_var(NcFile *nc, const string &var_name, const NcType ncType,
              const NcDim ncDim1, const NcDim ncDim2, const NcDim ncDim3,
              const NcDim ncDim4, const int deflate_level) {
   vector<NcDim> ncDims;
   ncDims.push_back(ncDim1);
   ncDims.push_back(ncDim2);
   ncDims.push_back(ncDim3);
   ncDims.push_back(ncDim4);
   return add_var(nc, var_name, ncType, ncDims, deflate_level);
}

////////////////////////////////////////////////////////////////////////

NcVar add_var(NcFile *nc, const string &var_name, const NcType ncType,
              const vector<NcDim> ncDims, const int deflate_level) {
   string new_var_name = var_name;
   patch_nc_name(&new_var_name);
   NcVar var = nc->addVar(new_var_name, ncType, ncDims);
   if (deflate_level > 0) {
      mlog << Debug(3) << "    nc_utils.add_var() deflate_level: " << deflate_level << "\n";
      var.setCompression(false, true, deflate_level);
   }

   // Check for lat and lon dimensions
   ConcatString cs;
   bool has_lat_dim, has_lon_dim;
   vector<NcDim>::const_iterator itDim;
   for (itDim = ncDims.begin(), has_lat_dim = has_lon_dim = false;
        itDim != ncDims.end(); ++itDim) {
           if (itDim->getName() == "lat") has_lat_dim = true;
      else if (itDim->getName() == "lon") has_lon_dim = true;
      if (itDim != ncDims.begin()) cs << " ";
      cs << itDim->getName();
   }

   // Add the coordinates variable attribute for variables
   // with both lat and lon dimensions
   if (has_lat_dim && var_name != "lat" &&
       has_lon_dim && var_name != "lon") {
      add_att(&var, "coordinates", cs.c_str());
   }

   return var;
}

////////////////////////////////////////////////////////////////////////

NcDim add_dim(NcFile *nc, const string &dim_name) {
   string new_dim_name = dim_name;
   patch_nc_name(&new_dim_name);
   return nc->addDim(new_dim_name);
}

////////////////////////////////////////////////////////////////////////

NcDim add_dim(NcFile *nc, const string &dim_name, const size_t dim_size) {
   string new_dim_name = dim_name;
   patch_nc_name(&new_dim_name);
   return nc->addDim(new_dim_name, dim_size);
}

////////////////////////////////////////////////////////////////////////

bool has_dim(NcFile *nc, const char * dim_name) {
   NcDim d = nc->getDim(dim_name);
   return IS_VALID_NC(d);
}

////////////////////////////////////////////////////////////////////////

bool get_dim(const NcFile *nc, const ConcatString &dim_name,
             int &dim_val, bool error_out) {
   NcDim dim;
   bool status = false;

   // Initialize
   dim_val = bad_data_int;

   dim = nc->getDim((string)dim_name);

   if(IS_VALID_NC(dim)) {
      dim_val = (int) (dim.getSize());
      status = true;
   }

   // Check error_out status
   if(error_out && !status) {
      mlog << Error << "\nget_dim() -> "
           << "can't find NetCDF dimension \"" << dim_name << "\".\n\n";
      exit(1);
   }

   return(status);
}

////////////////////////////////////////////////////////////////////////

int get_dim_count(const NcFile *nc) {
   return( IS_INVALID_NC_P(nc) ? -1 : nc->getDimCount());
}

////////////////////////////////////////////////////////////////////////

int get_dim_count(const NcVar *var) {
   return( IS_INVALID_NC_P(var) ? -1 : var->getDimCount());
}

////////////////////////////////////////////////////////////////////////

int get_dim_size(const NcDim *dim) {

   return( IS_INVALID_NC_P(dim) ? -1 : dim->getSize() );
}

////////////////////////////////////////////////////////////////////////

int get_dim_size(const NcVar *var, const int dim_offset) {
   int dim_size = -1;
   if(IS_VALID_NC_P(var)) {
      NcDim nc_dim = get_nc_dim(var, dim_offset);
      if (IS_VALID_NC(nc_dim)) dim_size = nc_dim.getSize();
   }

   return( dim_size );
}

////////////////////////////////////////////////////////////////////////

int get_dim_value(const NcFile *nc, const string &dim_name, const bool error_out) {
   NcDim dim;
   int dim_val;
   bool status = false;

   // Initialize
   dim_val = bad_data_int;

   dim = nc->getDim((string)dim_name);

   if(IS_VALID_NC(dim)) {
      dim_val = (int) (dim.getSize());
      status = true;
   }

   // Check error_out status
   if(error_out && !status) {
      mlog << Error << "\nget_dim() -> "
           << "can't find NetCDF dimension \"" << dim_name << "\".\n\n";
      exit(1);
   }

   return(dim_val);
}


////////////////////////////////////////////////////////////////////////

NcDim get_nc_dim(const NcFile *nc, const string &dim_name) {
   return nc->getDim(dim_name);
}

////////////////////////////////////////////////////////////////////////

NcDim get_nc_dim(const NcVar *var, const string &dim_name) {
   NcDim d;
   int dimCount = var->getDimCount();
   for (int idx=0; idx<dimCount; idx++) {
      NcDim dim = var->getDim(idx);
      if (strcmp(dim.getName().c_str(), dim_name.c_str()) == 0) {
         d = dim;
         break;
      }
   }
   return d;
}

////////////////////////////////////////////////////////////////////////

NcDim get_nc_dim(const NcVar *var, const int dim_offset) {
   if (var->getDimCount() > dim_offset)
      return var->getDim(dim_offset);
   else {
      NcDim d;
      return d;
   }
}


////////////////////////////////////////////////////////////////////////

bool get_dim_names(const NcFile *nc, StringArray *dimNames) {

   int i, dimCount;
   //NcDim dim;
   bool status = false;

   dimCount = nc->getDimCount();

   i = 0;
   multimap<string, NcDim>::iterator itDim;
   multimap<string, NcDim> dims = nc->getDims();
   for (itDim = dims.begin(); itDim != dims.end(); ++itDim) {
      dimNames->add((*itDim).first);
      i++;
   }

   if (i != dimCount) {
      mlog << Error << "\n\tget_dim_names(nc) -> "
           << "does not match array, allocated " << dimCount << " but assigned " << i << ".\n\n";
   }
   return(status);
}

////////////////////////////////////////////////////////////////////////

bool get_dim_names(const NcVar *var, StringArray *dimNames) {

   int i, dimCount;
   NcDim dim;
   bool status = false;

   dimCount = var->getDimCount();

   i = 0;
   vector<NcDim>::iterator itDim;
   vector<NcDim> dims = var->getDims();
   for (itDim = dims.begin(); itDim != dims.end(); ++itDim) {
      dim = (*itDim);
      dimNames->add(dim.getName());
      i++;
   }

   if (i != dimCount) {
      mlog << Error << "\n\tget_dim_names(var) -> "
           << "does not match array, allocated " << dimCount << " but assigned " << i << ".\n\n";
   }
   return(status);
}

////////////////////////////////////////////////////////////////////////

vector<NcDim> get_dims(const NcVar *var, int *dim_count) {

   int dimCount;

   dimCount = var->getDimCount();
   *dim_count = dimCount;

   return var->getDims();
}

////////////////////////////////////////////////////////////////////////

bool is_nc_name_lat(const ConcatString name) {
   bool is_latitude = (name == "lat" || name == "LAT"
           || name == "Lat" || name == "Latitude"
           || name == "latitude" || name == "LATITUDE");
   return is_latitude;
}

////////////////////////////////////////////////////////////////////////

bool is_nc_name_lon(const ConcatString name) {
   bool is_longitude = (name == "lon" || name == "LON"
           || name == "Lon" || name == "Longitude"
           || name == "longitude" || name == "LONGITUDE");
   return is_longitude;
}

////////////////////////////////////////////////////////////////////////

bool is_nc_name_time(const ConcatString name) {
   bool is_time = (name == "t" || name == "time" || name == "Time" || name == "TIME"
           || name == "datetime" || name == "Datetime" || name == "DATETIME");
   return is_time;
}

////////////////////////////////////////////////////////////////////////

bool is_nc_attr_lat(const ConcatString name) {
   bool is_latitude = (is_nc_name_lat(name) || name == "x" || name == "X");
   return is_latitude;
}

////////////////////////////////////////////////////////////////////////

bool is_nc_attr_lon(const ConcatString name) {
   bool is_longitude = (is_nc_name_lon(name) || name == "y" || name == "Y");
   return is_longitude;
}

////////////////////////////////////////////////////////////////////////

bool is_nc_attr_time(const ConcatString name) {
   bool is_time = (is_nc_name_time(name) || name == "T");
   return is_time;
}

////////////////////////////////////////////////////////////////////////

NcVar get_nc_var_lat(const NcFile *nc) {
   NcVar var;
   bool found = false;
   multimap<string,NcVar> mapVar = GET_NC_VARS_P(nc);
   static const char *method_name = "get_nc_var_lat() ";

   for (multimap<string,NcVar>::iterator itVar = mapVar.begin();
        itVar != mapVar.end(); ++itVar) {
      ConcatString name = (*itVar).first;
      //if (is_nc_name_lat(name)) found = true;
      if (get_var_standard_name(&(*itVar).second, name)) {
         if (is_nc_name_lat(name)) found = true;
      }
      if (!found && get_var_units(&(*itVar).second, name)) {
         if (is_nc_unit_latitude(name.c_str())) {
            if (get_nc_att_value(&(*itVar).second, axis_att_name, name)) {
               if (is_nc_attr_lat(name)) found = true;
            }
            else if (get_nc_att_value(&(*itVar).second,
                                      coordinate_axis_type_att_name, name)) {
               if (is_nc_attr_lat(name)) found = true;
            }
         }
      }
      if (found) {
         var = (*itVar).second;
         break;
      }
   }

   if (found) {
      mlog << Debug(6) << method_name << "found the latitude variable \""
           << GET_NC_NAME(var) << "\"\n";
   }
   else {
      mlog << Debug(6) << method_name << "fail to find the latitude variable\n";
   }
   return var;
}

////////////////////////////////////////////////////////////////////////

NcVar get_nc_var_lon(const NcFile *nc) {
   NcVar var;
   bool found = false;
   multimap<string,NcVar> mapVar = GET_NC_VARS_P(nc);
   static const char *method_name = "get_nc_var_lon() ";

   for (multimap<string,NcVar>::iterator itVar = mapVar.begin();
        itVar != mapVar.end(); ++itVar) {
      ConcatString name = (*itVar).first;
      //if (is_nc_name_lon(name)) found = true;
      if (get_var_standard_name(&(*itVar).second, name)) {
         if (is_nc_name_lon(name)) found = true;
      }
      if (!found && get_var_units(&(*itVar).second, name)) {
         if (is_nc_unit_longitude(name.c_str())) {
            if (get_nc_att_value(&(*itVar).second, axis_att_name, name)) {
               if (is_nc_attr_lon(name)) found = true;
            }
            else if (get_nc_att_value(&(*itVar).second,
                                      coordinate_axis_type_att_name, name)) {
               if (is_nc_attr_lon(name)) found = true;
            }
         }
      }
      if (found) {
         var = (*itVar).second;
         break;
      }
   }

   if (found) {
      mlog << Debug(6) << method_name << "found the longitude variable \""
           << GET_NC_NAME(var) << "\"\n";
   }
   else {
      mlog << Debug(6) << method_name << "fail to find the longitude variable\n";
   }
   return var;
}

////////////////////////////////////////////////////////////////////////

NcVar get_nc_var_time(const NcFile *nc) {
   NcVar var;
   bool found = false;
   multimap<string,NcVar> mapVar = GET_NC_VARS_P(nc);
   static const char *method_name = "get_nc_var_time() ";

   for (multimap<string,NcVar>::iterator itVar = mapVar.begin();
        itVar != mapVar.end(); ++itVar) {
      ConcatString name = (*itVar).first;
      //if (is_nc_name_time(name)) found = true;
      if (get_var_standard_name(&(*itVar).second, name)) {
         if (is_nc_name_time(name)) found = true;
         mlog << Debug(7) << method_name << "checked variable \""
           << name << "\"  is_time: " << found << "\n";
      }
      if (!found && get_var_units(&(*itVar).second, name)) {
         if (is_nc_unit_time(name.c_str())) {
            if (get_nc_att_value(&(*itVar).second, axis_att_name, name)) {
               if (is_nc_attr_time(name)) found = true;
            }
            else if (get_nc_att_value(&(*itVar).second,
                                      coordinate_axis_type_att_name, name)) {
               if (is_nc_attr_time(name)) found = true;
            }
         }
      }
      if (found) {
         var = (*itVar).second;
         break;
      }
   }

   if (found) {
      mlog << Debug(6) << method_name << "found the time variable \""
           << GET_NC_NAME(var) << "\"\n";
   }
   else {
      mlog << Debug(6) << method_name << "fail to find the time variable\n";
   }
   return var;
}

////////////////////////////////////////////////////////////////////////

NcFile *open_ncfile(const char * nc_name, bool write) {
   NcFile *nc = (NcFile *)0;

   try {
      if (write) {
         nc = new NcFile(nc_name, NcFile::replace, NcFile::nc4);
      }
      else {
         struct stat fileInfo;
         if (stat(nc_name, &fileInfo) == 0) {
            nc = new NcFile(nc_name, NcFile::read);
         }
      }
   }
   catch(NcException& e) {
   }
   return nc;
}

////////////////////////////////////////////////////////////////////////
// Implement the old API var->num_vals()

int get_data_size(NcVar *var) {
   int dimCount = 0;
   int data_size = 1;

   dimCount = var->getDimCount();
   for (int i=0; i<dimCount; i++) {
      data_size *= var->getDim(i).getSize();
   }
   return data_size;
}

////////////////////////////////////////////////////////////////////////

unixtime get_reference_unixtime(NcVar *time_var, int &sec_per_unit,
                                bool &no_leap_year) {
   unixtime ref_ut = 0;
   ConcatString time_unit_str;
   static const char *method_name = "get_reference_unixtime() -> ";

   if (get_var_units(time_var, time_unit_str)) {
      parse_cf_time_string(time_unit_str.c_str(), ref_ut, sec_per_unit);
      no_leap_year = (sec_per_day == sec_per_unit) ? get_att_no_leap_year(time_var) : false;
   }
   else {
      sec_per_unit = 1;
   }

   return ref_ut;
}

////////////////////////////////////////////////////////////////////////

bool is_nc_unit_latitude(const char *units) {
   bool axis_unit = (strcmp(units, "degrees_north") == 0 ||
        strcmp(units, "degree_north") == 0 ||
        strcmp(units, "degree_N") == 0 ||
        strcmp(units, "degrees_N") == 0 ||
        strcmp(units, "degreeN") == 0 ||
        strcmp(units, "degreesN") == 0);
   return axis_unit;
}

////////////////////////////////////////////////////////////////////////

bool is_nc_unit_longitude(const char *units) {
   bool axis_unit = (strcmp(units, "degrees_east") == 0 ||
        strcmp(units, "degree_east") == 0 ||
        strcmp(units, "degree_E") == 0 ||
        strcmp(units, "degrees_E") == 0 ||
        strcmp(units, "degreeE") == 0 ||
        strcmp(units, "degreesE") == 0);
   return axis_unit;
}

////////////////////////////////////////////////////////////////////////

bool is_nc_unit_time(const char *units) {
   return check_reg_exp(nc_time_unit_exp, units);
}

////////////////////////////////////////////////////////////////////////

void parse_cf_time_string(const char *str, unixtime &ref_ut,
                          int &sec_per_unit) {
   static const char *method_name = "parse_cf_time_string() -> ";

   // Initialize
   ref_ut = sec_per_unit = 0;

   // Check for expected time string format:
   //   [seconds|minutes|hours|days] since YYYY-MM-DD HH:MM:SS
   if(!check_reg_exp(nc_time_unit_exp, str)) {
      mlog << Warning << "\n" << method_name
           << "unexpected NetCDF CF convention time unit \""
           << str << "\"\n\n";
      return;
   }
   else {
      // Tokenize the input string
      // Parse using spaces or 'T' for timestrings such as:
      //   minutes since 2016-01-28T12:00:00Z
      //   seconds since 1977-08-07 12:00:00Z
      StringArray tok;
      tok.parse_delim(str, " T");
      tok.set_ignore_case(true);

      // Determine the time step
           if(tok.has("second")  ||
              tok.has("seconds") ||
              tok.has("s"))      sec_per_unit = 1;
      else if(tok.has("minute")  ||
              tok.has("minutes") ||
              tok.has("min"))    sec_per_unit = 60;
      else if(tok.has("hour")    ||
              tok.has("hours")   ||
              tok.has("hr")      ||
              tok.has("h"))      sec_per_unit = 3600;
      else if(tok.has("day")     ||
              tok.has("days")    ||
              tok.has("d"))      sec_per_unit = sec_per_day;
      else if(tok.has("month")   ||
              tok.has("months")  ||
              tok.has("m"))      sec_per_unit = sec_per_day * 30;
      else if(tok.has("year")    ||
              tok.has("years")   ||
              tok.has("y"))      sec_per_unit = sec_per_day * 30 * 12;
      else {
         mlog << Warning << "\n" << method_name
              << "Unsupported time step in the CF convention time unit \""
              << str << "\"\n\n";
         return;
      }

      // Parse the reference time
      StringArray ymd, hms;
      ymd.parse_delim(tok[2], "-");
      if(tok.n_elements() > 3) hms.parse_delim(tok[3], ":");
      else                     hms.parse_delim("00:00:00", ":");
      ref_ut = mdyhms_to_unix(atoi(ymd[1].c_str()), atoi(ymd[2].c_str()),
                              atoi(ymd[0].c_str()), atoi(hms[0].c_str()),
                              hms.n_elements() > 1 ? atoi(hms[1].c_str()) : 0,
                              hms.n_elements() > 2 ? atoi(hms[2].c_str()) : 0);
   }

   mlog << Debug(4) << method_name
        << "parsed NetCDF CF convention time unit string \"" << str
        << "\"\n\t\t as a reference time of " << unix_to_yyyymmdd_hhmmss(ref_ut)
        << " and " << sec_per_unit << " second(s) per time step.\n";

   return;
}

////////////////////////////////////////////////////////////////////////
