// *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*
// ** Copyright UCAR (c) 1992 - 2019
// ** University Corporation for Atmospheric Research (UCAR)
// ** National Center for Atmospheric Research (NCAR)
// ** Research Applications Lab (RAL)
// ** P.O.Box 3000, Boulder, Colorado, 80307-3000, USA
// *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*



////////////////////////////////////////////////////////////////////////


#ifndef  __MTD_READ_DATA_H__
#define  __MTD_READ_DATA_H__


////////////////////////////////////////////////////////////////////////


#include "string_array.h"
#include "mtd_file_float.h"
#include "mtd_config_info.h"
#include "var_info.h"


////////////////////////////////////////////////////////////////////////


extern void mtd_read_data(MtdConfigInfo &, VarInfo &,
                          const StringArray & filenames, MtdFloatFile &);


////////////////////////////////////////////////////////////////////////


#endif   /*  __MTD_READ_DATA_H__  */


////////////////////////////////////////////////////////////////////////


