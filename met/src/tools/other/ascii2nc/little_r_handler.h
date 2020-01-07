// *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*
// ** Copyright UCAR (c) 1992 - 2020
// ** University Corporation for Atmospheric Research (UCAR)
// ** National Center for Atmospheric Research (NCAR)
// ** Research Applications Lab (RAL)
// ** P.O.Box 3000, Boulder, Colorado, 80307-3000, USA
// *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*


////////////////////////////////////////////////////////////////////////


#ifndef  __LITTLERHANDLER_H__
#define  __LITTLERHANDLER_H__


////////////////////////////////////////////////////////////////////////


#include <iostream>
#include <map>
#include <string>

#include "file_handler.h"


////////////////////////////////////////////////////////////////////////


class LittleRHandler : public FileHandler
{

public:

  LittleRHandler(const string &program_name);
  virtual ~LittleRHandler();

  virtual bool isFileType(LineDataFile &ascii_file) const;
  
  static string getFormatString()
  {
    return "little_r";
  }

protected:
  
  ///////////////////////
  // Protected members //
  ///////////////////////


  ///////////////////////
  // Protected methods //
  ///////////////////////

  // Read the observations from the given file and add them to the
  // _observations vector.

  virtual bool _readObservations(LineDataFile &ascii_file);
  
};


////////////////////////////////////////////////////////////////////////


#endif   /*  __LITTLERHANDLER_H__  */


////////////////////////////////////////////////////////////////////////


