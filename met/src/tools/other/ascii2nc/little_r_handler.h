

   // *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*
   // ** Copyright UCAR (c) 1992 - 2012
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

  // Process the header records in the file

  bool _prepareHeaders(LineDataFile &ascii_file);

  // Process the observations in the file

  bool _processObs(LineDataFile &ascii_file,
		   const string &nc_filename);
  
//  int _nFileColumns;
};


////////////////////////////////////////////////////////////////////////


#endif   /*  __LITTLERHANDLER_H__  */


////////////////////////////////////////////////////////////////////////


