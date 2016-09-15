

// *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*
// ** Copyright UCAR (c) 1992 - 2016
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
  
  NcError err(NcError::silent_nonfatal);

  // Open the file

  _ncFile = open_ncfile(filename);

  if (!(_ncFile->is_valid()))
  {
    close();
    return false;
  }

  // Pull out the dimensions

  NcDim *num_recs_dim = _ncFile->get_dim("recNum");
  if (num_recs_dim == 0)
  {
    mlog << Error << "\n" << method_name << " -> "
	 << "recNum dimension not found in file\n";
    
    return false;
  }
  
  _numRecords = num_recs_dim->size();
  _currRecord = 0;
  
  // Pull out the needed variables

  // aircraftId

  NcDim *aircraft_id_len_dim = _ncFile->get_dim("aircraftIdLen");
  if (aircraft_id_len_dim == 0)
  {
    mlog << Error << "\n" << method_name << " -> "
	 << "aircraftIdLen dimension not found in file\n";
    
    return false;
  }
  
  long aircraft_id_len = aircraft_id_len_dim->size();
  
  NcVar *aircraft_id_var = _ncFile->get_var("aircraftId");
  if (aircraft_id_var == 0)
  {
    mlog << Error << "\n" << method_name << " -> "
	 << "aircraftId variable not found in file\n";
    
    return false;
  }
  
  char *aircraft_id = new char[_numRecords * aircraft_id_len];
  
  if (!aircraft_id_var->get(aircraft_id, _numRecords, aircraft_id_len))
  {
    mlog << Error << "\n" << method_name << " -> "
	 << "error retrieving aircraftId values from file\n";
    
    return false;
  }
  
  _aircraftId = new string[_numRecords];
  
  for (int i = 0; i < _numRecords; ++i)
    _aircraftId[i] = &aircraft_id[i * aircraft_id_len];
  
  // timeObs

  NcVar *time_obs_var = _ncFile->get_var("timeObs");
  if (time_obs_var == 0)
  {
    mlog << Error << "\n" << method_name << " -> "
	 << "timeObs variable not found in file\n";
    
    return false;
  }
  
  _timeObs = new time_t[_numRecords];
  
  if (!time_obs_var->get(_timeObs, _numRecords))
  {
    mlog << Error << "\n" << method_name << " -> "
	 << "error retrieving timeObs variable from file\n";
    
    return false;
  }
  
  // latitude

  NcVar *latitude_var = _ncFile->get_var("latitude");
  if (latitude_var == 0)
  {
    mlog << Error << "\n" << method_name << " -> "
	 << "latitude variable not found in file\n";
    
    return false;
  }
  
  _latitude = new double[_numRecords];
  
  if (!latitude_var->get(_latitude, _numRecords))
  {
    mlog << Error << "\n" << method_name << " -> "
	 << "error retrieving latitude values from file\n";
    
    return false;
  }
    
  // longitude

  NcVar *longitude_var = _ncFile->get_var("longitude");
  if (longitude_var == 0)
  {
    mlog << Error << "\n" << method_name << " -> "
	 << "longitude variable not found in file\n";
    
    return false;
  }
  
  _longitude = new double[_numRecords];
  
  if (!longitude_var->get(_longitude, _numRecords))
  {
    mlog << Error << "\n" << method_name << " -> "
	 << "error retrieving longitude values from file\n";
    
    return false;
  }
  
  // altitude

  NcVar *altitude_var = _ncFile->get_var("altitude");
  if (altitude_var == 0)
  {
    mlog << Error << "\n" << method_name << " -> "
	 << "altitude variable not found in file\n";
    
    return false;
  }
  
  _altitude = new double[_numRecords];
  
  if (!altitude_var->get(_altitude, _numRecords))
  {
    mlog << Error << "\n" << method_name << " -> "
	 << "retrieving altitude values from file\n";
    
    return false;
  }
  
  // QCconfidence

  NcVar *qc_confidence_var = _ncFile->get_var("QCconfidence");
  if (qc_confidence_var == 0)
  {
    mlog << Error << "\n" << method_name << " -> "
	 << "QCconfidence variable not found in file\n";
    
    return false;
  }
  
  _QCconfidence = new double[_numRecords];
  
  if (!qc_confidence_var->get(_QCconfidence, _numRecords))
  {
    mlog << Error << "\n" << method_name << " -> "
	 << "error retrieving QCconfidence values from file\n";
    
    return false;
  }
  
  // medEDR

  NcVar *med_edr_var = _ncFile->get_var("medEDR");
  if (med_edr_var == 0)
  {
    mlog << Error << "\n" << method_name << " -> "
	 << "medEDR variable not found in file\n";
    
    return false;
  }
  
  _medEDR = new double[_numRecords];
  
  if (!med_edr_var->get(_medEDR, _numRecords))
  {
    mlog << Error << "\n" << method_name << " -> "
	 << "error retrieving medEDR values from file\n";
    
    return false;
  }
  
  // maxEDR

  NcVar *max_edr_var = _ncFile->get_var("maxEDR");
  if (max_edr_var == 0)
  {
    mlog << Error << "\n" << method_name << " -> "
	 << "maxEDR variable not found in file\n";
    
    return false;
  }
  
  _maxEDR = new double[_numRecords];
  
  if (!max_edr_var->get(_maxEDR, _numRecords))
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
