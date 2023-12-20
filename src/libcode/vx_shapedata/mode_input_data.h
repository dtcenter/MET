// ** Copyright UCAR (c) 1992 - 2023
// ** University Corporation for Atmospheric Research (UCAR)
// ** National Center for Atmospheric Research (NCAR)
// ** Research Applications Lab (RAL)
// ** P.O.Box 3000, Boulder, Colorado, 80307-3000, USA
// *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*


////////////////////////////////////////////////////////////////////////


#ifndef  __MODE_INPUT_DATA_H__
#define  __MODE_INPUT_DATA_H__

#include <string>
#include "data_plane.h"
#include "grid_base.h"
#include "data_file_type.h"

////////////////////////////////////////////////////////////////////////


class ModeInputData {

 private:

 public:

   ModeInputData(const std::string &name, const DataPlane &dp,
                 const Grid &g, const GrdFileType t) :
      _name(name), _dataPlane(dp), _grid(g), _fileType(t)  {  }
      
   ~ModeInputData() {}

   std::string _name;
   DataPlane _dataPlane;
   Grid _grid;
   GrdFileType _fileType;
};


#endif


/////////////////////////////////////////////////////////////////////////
