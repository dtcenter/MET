

// *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*
// ** Copyright UCAR (c) 1992 - 2020
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
#include <cstdio>
#include <cmath>
#include <string>

#include "vx_math.h"
#include "vx_cal.h"
#include "vx_log.h"

#include "insitu_nc_file.h"
#include "nc_utils.h"


////////////////////////////////////////////////////////////////////////


   //
   //  Code for class InsituNcFile
   //


////////////////////////////////////////////////////////////////////////


InsituNcFile::InsituNcFile() :
  _numRecords(0),
  _currRecord(0),
  _aircraftId(0),
  _timeObs(0),
  _latitude(0),
  _longitude(0),
  _altitude(0),
  _QCconfidence(0),
  _medEDR(0),
  _maxEDR(0)
{
  init_from_scratch();
}


////////////////////////////////////////////////////////////////////////


InsituNcFile::~InsituNcFile()
{
  close();
}


////////////////////////////////////////////////////////////////////////


void InsituNcFile::init_from_scratch()

{
  // Initialize the pointers

  _ncFile = (NcFile *) 0;

  // Close any existing file

  close();

  return;
}


////////////////////////////////////////////////////////////////////////


void InsituNcFile::close()
{

  // Reclaim the file pointer

  if (_ncFile)
  {
    delete _ncFile;
    _ncFile = (NcFile *)0;
  }

  // Reclaim the space used for the variables

  delete [] _aircraftId;
  delete [] _timeObs;
  delete [] _latitude;
  delete [] _longitude;
  delete [] _altitude;
  delete [] _QCconfidence;
  delete [] _medEDR;
  delete [] _maxEDR;
  
  return;
}


////////////////////////////////////////////////////////////////////////


bool InsituNcFile::open(const char * filename)
{
  static const string method_name = "InsituNcFile::open()";
  
  // Close any open files and clear out the associated members

  close();

  // Change the error behavior of the netCDF C++ API by creating an
  // NcError object. Until it is destroyed, this NcError object will
  // ensure that the netCDF C++ API silently returns error codes
  // on any failure, and leaves any other error handling to the
  // calling program. In the case of this example, we just exit with
  // an NC_ERR error code.

  // FIXME COmmmented out for NetCDF4
  //NcError err(NcError::silent_nonfatal);

  // Open the file

  _ncFile = open_ncfile(filename);

  if (!(IS_INVALID_NC_P(_ncFile)))
  {
    close();
    return false;
  }

  // Pull out the dimensions

  NcDim num_recs_dim = get_nc_dim(_ncFile, "recNum");
  if (IS_INVALID_NC(num_recs_dim))
  {
    mlog << Error << "\n" << method_name << " -> "
	 << "recNum dimension not found in file\n";
    
    return false;
  }
  
  _numRecords = GET_NC_SIZE(num_recs_dim);
  _currRecord = 0;
  
  // Pull out the needed variables

  // aircraftId

  NcDim aircraft_id_len_dim = get_nc_dim(_ncFile, "aircraftIdLen");
  if (IS_INVALID_NC(aircraft_id_len_dim))
  {
    mlog << Error << "\n" << method_name << " -> "
	 << "aircraftIdLen dimension not found in file\n";
    
    return false;
  }
  
  long aircraft_id_len = GET_NC_SIZE(aircraft_id_len_dim);
  
  NcVar aircraft_id_var = get_nc_var(_ncFile, "aircraftId");
  if (IS_INVALID_NC(aircraft_id_var))
  {
    mlog << Error << "\n" << method_name << " -> "
	 << "aircraftId variable not found in file\n";
    
    return false;
  }
  
  char *aircraft_id= new char[_numRecords * aircraft_id_len];
  
  if (!get_nc_data(&aircraft_id_var, aircraft_id))
  {
    mlog << Error << "\n" << method_name << " -> "
	 << "error retrieving aircraftId values from file\n";
    if(aircraft_id) delete[] aircraft_id;
    return false;
  }
  
  _aircraftId = new string[_numRecords];
  
  for (int i = 0; i < _numRecords; ++i)
    _aircraftId[i] = &aircraft_id[i * aircraft_id_len];

  if(aircraft_id) delete[] aircraft_id;
  
  // timeObs

  NcVar time_obs_var = get_nc_var(_ncFile, "timeObs");
  if (IS_INVALID_NC(time_obs_var))
  {
    mlog << Error << "\n" << method_name << " -> "
	 << "timeObs variable not found in file\n";
    
    return false;
  }
  
  _timeObs = new time_t[_numRecords];
  
  //if (!get_nc_data(time_obs_var, _timeObs, _numRecords))
  if (!get_nc_data(&time_obs_var, _timeObs))
  {
    mlog << Error << "\n" << method_name << " -> "
	 << "error retrieving timeObs variable from file\n";
    
    return false;
  }
  
  // latitude

  NcVar latitude_var = get_nc_var(_ncFile, "latitude");
  if (IS_INVALID_NC(latitude_var))
  {
    mlog << Error << "\n" << method_name << " -> "
	 << "latitude variable not found in file\n";
    
    return false;
  }
  
  _latitude = new double[_numRecords];
  
  if (!get_nc_data(&latitude_var, _latitude, _numRecords))
  {
    mlog << Error << "\n" << method_name << " -> "
	 << "error retrieving latitude values from file\n";
    
    return false;
  }
    
  // longitude

  NcVar longitude_var = get_nc_var(_ncFile, "longitude");
  if (IS_INVALID_NC(longitude_var))
  {
    mlog << Error << "\n" << method_name << " -> "
	 << "longitude variable not found in file\n";
    
    return false;
  }
  
  _longitude = new double[_numRecords];
  
  if (!get_nc_data(&longitude_var, _longitude, _numRecords))
  {
    mlog << Error << "\n" << method_name << " -> "
	 << "error retrieving longitude values from file\n";
    
    return false;
  }
  
  // altitude

  NcVar altitude_var = get_nc_var(_ncFile, "altitude");
  if (IS_INVALID_NC(altitude_var))
  {
    mlog << Error << "\n" << method_name << " -> "
	 << "altitude variable not found in file\n";
    
    return false;
  }
  
  _altitude = new double[_numRecords];
  
  if (!get_nc_data(&altitude_var, _altitude, _numRecords))
  {
    mlog << Error << "\n" << method_name << " -> "
	 << "retrieving altitude values from file\n";
    
    return false;
  }
  
  // QCconfidence

  NcVar qc_confidence_var = get_nc_var(_ncFile, "QCconfidence");
  if (IS_INVALID_NC(qc_confidence_var))
  {
    mlog << Error << "\n" << method_name << " -> "
	 << "QCconfidence variable not found in file\n";
    
    return false;
  }
  
  _QCconfidence = new double[_numRecords];
  
  if (!get_nc_data(&qc_confidence_var, _QCconfidence, _numRecords))
  {
    mlog << Error << "\n" << method_name << " -> "
	 << "error retrieving QCconfidence values from file\n";
    
    return false;
  }
  
  // medEDR

  NcVar med_edr_var = get_nc_var(_ncFile, "medEDR");
  if (IS_INVALID_NC(med_edr_var))
  {
    mlog << Error << "\n" << method_name << " -> "
	 << "medEDR variable not found in file\n";
    
    return false;
  }
  
  _medEDR = new double[_numRecords];
  
  if (!get_nc_data(&med_edr_var, _medEDR, _numRecords))
  {
    mlog << Error << "\n" << method_name << " -> "
	 << "error retrieving medEDR values from file\n";
    
    return false;
  }
  
  // maxEDR

  NcVar max_edr_var = get_nc_var(_ncFile, "maxEDR");
  if (IS_INVALID_NC(max_edr_var))
  {
    mlog << Error << "\n" << method_name << " -> "
	 << "maxEDR variable not found in file\n";
    
    return false;
  }
  
  _maxEDR = new double[_numRecords];
  
  if (!get_nc_data(&max_edr_var, _maxEDR, _numRecords))
  {
    mlog << Error << "\n" << method_name << " -> "
	 << "error retrieving maxEDR values from file\n";
    
    return false;
  }
  
  return true;
}


////////////////////////////////////////////////////////////////////////


bool InsituNcFile::getNextRecord(string &aircraftId, time_t &timeObs,
				 double &latitude, double &longitude,
				 double &altitude, double &QCconfidence,
				 double &medEDR, double &maxEDR)
{
  // If we don't have any more records, return

  if (_currRecord >= _numRecords)
    return false;
  
  // Set all of the values

  aircraftId = _aircraftId[_currRecord];
  timeObs = _timeObs[_currRecord];
  latitude = _latitude[_currRecord];
  longitude = _longitude[_currRecord];
  altitude = _altitude[_currRecord];
  QCconfidence = _QCconfidence[_currRecord];
  medEDR = _medEDR[_currRecord];
  maxEDR = _maxEDR[_currRecord];
  
  // Increment the current record so we're ready for the next call

  ++_currRecord;
  
  return true;
}
      

////////////////////////////////////////////////////////////////////////
