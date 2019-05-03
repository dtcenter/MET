

// *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*
// ** Copyright UCAR (c) 1992 - 2017
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

#include "is_netcdf_file.h"

#include "vx_nc_util.h"


////////////////////////////////////////////////////////////////////////


static const char netcdf_magic  [] = "CDF";
static const char hdf_magic     [] = "HDF";
static const int  netcdf_magic_len = strlen(netcdf_magic);

static const char nccf_att_name[]  = "Conventions";
static const char nccf_att_name_l[]  = "conventions";
static const char nccf_att_name_U[]  = "CONVENTIONS";
static const char nccf_att_value[] = "CF-";

static const char ncmet_att_version[]    = "MET_version";
static const char ncmet_att_projection[] = "Projection";

static const char ncpinterp_att_name[]  = "TITLE";
static const char ncpinterp_att_value[] = "ON PRES LEVELS";

////////////////////////////////////////////////////////////////////////


bool is_netcdf_file(const char * filename)

{

   if ( !filename || !(*filename) )  return ( false );

   int fd = -1;
   int n_read;
   char buf[netcdf_magic_len];

   if ( (fd = open(filename, O_RDONLY)) < 0 )  return ( false );

   n_read = read(fd, buf, netcdf_magic_len);

   close(fd);  fd = -1;

   if ( n_read != netcdf_magic_len )  return ( false );

   if ( strncmp(buf, netcdf_magic, netcdf_magic_len) == 0
     || strncmp(buf, hdf_magic, netcdf_magic_len) == 0)  return ( true );

   //
   //  done
   //

   return ( false );

}


////////////////////////////////////////////////////////////////////////


bool is_nccf_file(const char * filename)
{
   bool status = false;
   try {
      ConcatString att_val;
      NcFile *nc_file = open_ncfile(filename);

      if (!IS_INVALID_NC_P(nc_file)) {
         bool found = get_global_att(nc_file, nccf_att_name, att_val);
         if (!found) found = get_global_att(nc_file, nccf_att_name_l, att_val);
         if (!found) found = get_global_att(nc_file, nccf_att_name_U, att_val);
         if (found) {
            status = (strncmp(att_val, nccf_att_value, strlen(nccf_att_value)) == 0);
         }
      }
   }
   catch(...) {
   }
   return ( status );

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
   }catch(...) {
   }
   return ( status );

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
         if (get_global_att(nc_file, ncpinterp_att_name, att_val)) {
            // Check the attribute value for the target string
            status = (strstr(att_val, ncpinterp_att_value));
         }
      }
   }catch(...) {
   }
   return ( status );

}


////////////////////////////////////////////////////////////////////////
