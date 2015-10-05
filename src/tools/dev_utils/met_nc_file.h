// *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*
// ** Copyright UCAR (c) 1992 - 2015
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

#include "netcdf.hh"

#include "vx_time_series.h"

////////////////////////////////////////////////////////////////////////


class MetNcFile
{

public:

  MetNcFile(const string &file_path);
  
  virtual ~MetNcFile();

  bool readFile(const int desired_grib_code,
		const string &desired_station_id,
		const string &desired_message_type,
		vector< SDObservation > &observations);
  

protected:

  ///////////////////////
  // Protected members //
  ///////////////////////

  string _filePath;
  
  NcFile *_ncFile;

  NcDim *_hdrArrDim;
  NcDim *_obsArrDim;
  NcDim *_nhdrDim;
  NcDim *_nobsDim;
  NcDim *_strlDim;

  NcVar *_hdrArrVar;
  NcVar *_hdrTypeVar;
  NcVar *_hdrSidVar;
  NcVar *_hdrVldVar;
  NcVar *_obsArrVar;
  
  ///////////////////////
  // Protected methods //
  ///////////////////////

};


////////////////////////////////////////////////////////////////////////


#endif   /*  __MET_NC_FILE_H__  */


////////////////////////////////////////////////////////////////////////


