// *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*
// ** Copyright UCAR (c) 1992 - 2019
// ** University Corporation for Atmospheric Research (UCAR)
// ** National Center for Atmospheric Research (NCAR)
// ** Research Applications Lab (RAL)
// ** P.O.Box 3000, Boulder, Colorado, 80307-3000, USA
// *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*

///////////////////////////////////////////////////////////////////////////////

#ifndef __MET_2D_DATA_FILE_FACTORY_H__
#define __MET_2D_DATA_FILE_FACTORY_H__

///////////////////////////////////////////////////////////////////////////////

#include "data_class.h"
#include "concat_string.h"

///////////////////////////////////////////////////////////////////////////////

class Met2dDataFileFactory
{
   public:
      static Met2dDataFile *new_met_2d_data_file(GrdFileType type);
      static Met2dDataFile *new_met_2d_data_file(const char *filename);
      static Met2dDataFile *new_met_2d_data_file(const char *filename, GrdFileType type);
};

///////////////////////////////////////////////////////////////////////////////

extern bool is_2d_data_file(const ConcatString &filename,
                            const ConcatString &config_str);

///////////////////////////////////////////////////////////////////////////////

#endif  // __MET_2D_DATA_FILE_FACTORY_H__

///////////////////////////////////////////////////////////////////////////////
