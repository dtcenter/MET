

// *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*
// ** Copyright UCAR (c) 1992 - 2023
// ** University Corporation for Atmospheric Research (UCAR)
// ** National Center for Atmospheric Research (NCAR)
// ** Research Applications Lab (RAL)
// ** P.O.Box 3000, Boulder, Colorado, 80307-3000, USA
// *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*




////////////////////////////////////////////////////////////////////////


using namespace std;

#include <iostream>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <cmath>

#include <netcdf>
using namespace netCDF;

#include "is_netcdf_file.h"

#include "vx_nc_util.h"


////////////////////////////////////////////////////////////////////////


static const char netcdf_magic  [] = "CDF";
static const char hdf_magic     [] = "HDF";
static const int  netcdf_magic_len = m_strlen(netcdf_magic);

static const string nccf_att_value  = "CF-";
static const string nccf_att_value2 = "CF ";
static const string nccf_att_value3 = "COARDS";
static const string mpas_att_value = "MPAS";
static const string ugrid_att_value = "UGRID";

static const string ncmet_att_version    = "MET_version";
static const string ncmet_att_projection = "Projection";

static const string nctitle_att_name     = "TITLE";
static const char ncpinterp_att_value [] = "ON PRES LEVELS";
static const char ncwrf_att_value []     = "OUTPUT FROM WRF";

static const string mesh_spec_att_name  = "mesh_spec";

////////////////////////////////////////////////////////////////////////


bool is_netcdf_file(const char * filename)

{
   if ( !filename || !(*filename) )  return false;

   int fd = -1;
   int n_read;
   char buf[netcdf_magic_len];

   if ( (fd = open(filename, O_RDONLY)) < 0 )  return false;

   n_read = read(fd, buf, netcdf_magic_len);

   close(fd);

   if ( n_read != netcdf_magic_len )  return false;

   if ( strncmp(buf, netcdf_magic, netcdf_magic_len) == 0
     || strncmp(buf, hdf_magic, netcdf_magic_len) == 0)  return true;

   //
   //  done
   //

   return false;

}


////////////////////////////////////////////////////////////////////////


bool is_nccf_file(const char * filename)
{
   bool status = false;
   try {
      ConcatString att_val;
      NcFile *nc_file = open_ncfile(filename);

      if (!IS_INVALID_NC_P(nc_file)) {
         bool found = get_cf_conventions(nc_file, att_val); // "Conventions" attrribute
         if (found) {
            status = (att_val.compare(0, nccf_att_value.length(),
                      nccf_att_value) == 0  ||
                      att_val.compare(0, nccf_att_value2.length(),
                      nccf_att_value2) == 0 ||
                      att_val.compare(0, nccf_att_value3.length(),
                      nccf_att_value3) == 0);
         }
      }

      delete nc_file;

   }
   catch(...) {
   }

   return status;

}


////////////////////////////////////////////////////////////////////////


bool is_ncmet_file(const char * filename)
{
   bool status = false;
   try {
      ConcatString att_val;

      NcFile *nc_file = open_ncfile(filename);

      if (!IS_INVALID_NC_P(nc_file)) {
         status = (get_global_att(nc_file, ncmet_att_version,    att_val) ||
                   get_global_att(nc_file, ncmet_att_projection, att_val));
      }

      delete nc_file;

   }catch(...) {
   }

   return status;

}


////////////////////////////////////////////////////////////////////////


bool is_ncpinterp_file(const char * filename)
{
   bool status = false;
   try {
      ConcatString att_val;
      NcFile *nc_file = open_ncfile(filename);

      if (!IS_INVALID_NC_P(nc_file)) {
         // Get the global attribute
         if (get_global_att(nc_file, nctitle_att_name, att_val)) {
            // Check the attribute value for the target string
            status = (strstr(att_val.c_str(), ncwrf_att_value) &&
                      strstr(att_val.c_str(), ncpinterp_att_value));
         }
      }

      delete nc_file;

   }catch(...) {
   }

   return status;

}


////////////////////////////////////////////////////////////////////////


bool is_ncwrf_file(const char * filename)
{
   bool status = false;
   try {
      ConcatString att_val;
      NcFile *nc_file = open_ncfile(filename);

      if (!IS_INVALID_NC_P(nc_file)) {
         // Get the global attribute
         if (get_global_att(nc_file, nctitle_att_name, att_val)) {
            // Check the attribute value for the target string
            // Distinguish between WRF and PInterp output
            status = ( strstr(att_val.c_str(), ncwrf_att_value) &&
                      !strstr(att_val.c_str(), ncpinterp_att_value));
         }
      }

      delete nc_file;

   }catch(...) {
   }

   return status;

}


////////////////////////////////////////////////////////////////////////


bool is_ugrid_file(const char * filename)
{
   bool status = false;
   try {
      ConcatString att_val;
      NcFile *nc_file = open_ncfile(filename);

      if (!IS_INVALID_NC_P(nc_file)) {
         // Get the global attribute
         // Check Conventions attrribute (Conventions= "UGRID" or "MPAS")
         if (get_cf_conventions(nc_file, att_val)) {
            status = (0 == att_val.compare(0, ugrid_att_value.length(),
                                           ugrid_att_value)
                   || 0 == att_val.compare(0, mpas_att_value.length(),
                                           mpas_att_value));
         }
         if (!status) {
            status = get_global_att(nc_file, mesh_spec_att_name, att_val); // for MPAS
         }
      }
      if (nullptr != nc_file) delete nc_file;

   }catch(...) {
   }

   return status;

}


////////////////////////////////////////////////////////////////////////
