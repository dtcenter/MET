// *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*
// ** Copyright UCAR (c) 1992 - 2024
// ** University Corporation for Atmospheric Research (UCAR)
// ** National Center for Atmospheric Research (NCAR)
// ** Research Applications Lab (RAL)
// ** P.O.Box 3000, Boulder, Colorado, 80307-3000, USA
// *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*


////////////////////////////////////////////////////////////////////////


#ifndef  __MET_HANDLER_H__
#define  __MET_HANDLER_H__


////////////////////////////////////////////////////////////////////////


#include <iostream>

#include "file_handler.h"


////////////////////////////////////////////////////////////////////////


class MetHandler : public FileHandler
{

public:

  MetHandler(const std::string &program_name);
  virtual ~MetHandler();

  virtual bool isFileType(LineDataFile &ascii_file) const;
  
  static std::string getFormatString()
  {
    return "met_point";
  }

protected:  

  // Read the observations from the given file and add them to the
  // _observations vector.

  virtual bool _readObservations(LineDataFile &ascii_file);
  
  int _nFileColumns;
};


////////////////////////////////////////////////////////////////////////


#endif   /*  __MET_HANDLER_H__  */


////////////////////////////////////////////////////////////////////////


