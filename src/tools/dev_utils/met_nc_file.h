// *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*
// ** Copyright UCAR (c) 1992 - 2023
// ** University Corporation for Atmospheric Research (UCAR)
// ** National Center for Atmospheric Research (NCAR)
// ** Research Applications Lab (RAL)
// ** P.O.Box 3000, Boulder, Colorado, 80307-3000, USA
// *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*




////////////////////////////////////////////////////////////////////////


#ifndef  __MET_NC_FILE_H__
#define  __MET_NC_FILE_H__


////////////////////////////////////////////////////////////////////////


#include <cstdio>
#include <string>
#include <vector>

#include "vx_time_series.h"

////////////////////////////////////////////////////////////////////////


class MetNcFile
{

public:

  MetNcFile(const std::string &file_path);
  
  virtual ~MetNcFile();

  bool readFile(const int desired_grib_code,
		const std::string &desired_station_id,
		const std::string &desired_message_type,
		std::vector< SDObservation > &observations);
  

protected:

  ///////////////////////
  // Protected members //
  ///////////////////////

  std::string _filePath;
  
  netCDF::NcFile *_ncFile;

  netCDF::NcDim *_hdrArrDim;
  netCDF::NcDim *_obsArrDim;
  netCDF::NcDim *_nhdrDim;
  netCDF::NcDim *_nobsDim;
  netCDF::NcDim *_strlDim;

  netCDF::NcDim hdrArrDim;
  netCDF::NcDim obsArrDim;
  netCDF::NcDim nhdrDim;
  netCDF::NcDim nobsDim;
  netCDF::NcDim strlDim;
  netCDF::NcDim strllDim;

  netCDF::NcVar *_hdrArrVar;
  netCDF::NcVar *_hdrTypeVar;
  netCDF::NcVar *_hdrSidVar;
  netCDF::NcVar *_hdrVldVar;
  netCDF::NcVar *_obsArrVar;
  
  netCDF::NcVar hdrArrVar;
  netCDF::NcVar hdrTypeVar;
  netCDF::NcVar hdrSidVar;
  netCDF::NcVar hdrVldVar;
  netCDF::NcVar obsArrVar;
  
  ///////////////////////
  // Protected methods //
  ///////////////////////

};


////////////////////////////////////////////////////////////////////////


#endif   /*  __MET_NC_FILE_H__  */


////////////////////////////////////////////////////////////////////////


