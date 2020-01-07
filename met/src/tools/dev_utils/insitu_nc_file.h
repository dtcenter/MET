

// *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*
// ** Copyright UCAR (c) 1992 - 2020
// ** University Corporation for Atmospheric Research (UCAR)
// ** National Center for Atmospheric Research (NCAR)
// ** Research Applications Lab (RAL)
// ** P.O.Box 3000, Boulder, Colorado, 80307-3000, USA
// *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*




////////////////////////////////////////////////////////////////////////


#ifndef  __INSITU_NC_FILE_H__
#define  __INSITU_NC_FILE_H__


////////////////////////////////////////////////////////////////////////


#include <ostream>

#include <netcdf>
using namespace netCDF;

#include "vx_grid.h"
#include "data_plane.h"
#include "long_array.h"
#include "nc_var_info.h"


////////////////////////////////////////////////////////////////////////


static const char nccf_lat_var_name [] = "lat";
static const char nccf_lon_var_name [] = "lon";


////////////////////////////////////////////////////////////////////////


class InsituNcFile {

   public:

      InsituNcFile();
     ~InsituNcFile();

      bool open(const char * filename);

      void close();

      NcFile * _ncFile;      //  allocated

      long _numRecords;
      long _currRecord;
      
      // output columns:
      // type  station_id  yyyymmdd_hhmmss lat lon elev grib_code pressure height quality_string value

      //
      // Variables
      //

      string *_aircraftId;
      time_t*_timeObs;
      double *_latitude;
      double *_longitude;
      double *_altitude;
      double *_QCconfidence;
      double *_medEDR;
      double *_maxEDR;
      
      //
      // Get the next record in the file
      //

      bool getNextRecord(string &aircraftId, time_t &timeObs,
			 double &latitude, double &longitude,
			 double &altitude, double &QCconfidence,
			 double &medEDR, double &maxEDR);
      
      void init_from_scratch();

};


////////////////////////////////////////////////////////////////////////


#endif   /*  __INSITU_NC_FILE_H__  */


////////////////////////////////////////////////////////////////////////


