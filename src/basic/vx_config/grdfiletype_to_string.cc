// *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*
// ** Copyright UCAR (c) 1992 - 2024
// ** University Corporation for Atmospheric Research (UCAR)
// ** National Center for Atmospheric Research (NCAR)
// ** Research Applications Lab (RAL)
// ** P.O.Box 3000, Boulder, Colorado, 80307-3000, USA
// *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*


////////////////////////////////////////////////////////////////////////


   //
   //  Warning:
   //
   //     This file is machine generated.
   //
   //     Do not edit by hand.
   //
   //     Created by enum_to_string from file "data_file_type.h"
   //


////////////////////////////////////////////////////////////////////////


#include <string.h>

#include "grdfiletype_to_string.h"

using namespace std;


////////////////////////////////////////////////////////////////////////


ConcatString grdfiletype_to_string(const GrdFileType t)

{

const char * s = (const char *) nullptr;

switch ( t )  {

   case GrdFileType_None:             s = "GrdFileType_None";             break;
   case GrdFileType_Gb1:              s = "GrdFileType_Gb1";              break;
   case GrdFileType_Gb2:              s = "GrdFileType_Gb2";              break;
   case GrdFileType_NcMet:            s = "GrdFileType_NcMet";            break;
   case GrdFileType_General_Netcdf:   s = "GrdFileType_General_Netcdf";   break;

   case GrdFileType_NcWrf:            s = "GrdFileType_NcWrf";            break;
   case GrdFileType_NcPinterp:        s = "GrdFileType_NcPinterp";        break;
   case GrdFileType_NcCF:             s = "GrdFileType_NcCF";             break;
   case GrdFileType_HdfEos:           s = "GrdFileType_HdfEos";           break;
   case GrdFileType_Bufr:             s = "GrdFileType_Bufr";             break;

   case GrdFileType_Python_Numpy:     s = "GrdFileType_Python_Numpy";     break;
   case GrdFileType_Python_Xarray:    s = "GrdFileType_Python_Xarray";    break;
   case GrdFileType_UGrid:            s = "GrdFileType_UGrid";            break;

   default:
      s = "(bad value)";
      break;

}   //  switch


return ConcatString(s);

}


////////////////////////////////////////////////////////////////////////


bool string_to_grdfiletype(const char * text, GrdFileType & t)

{

     if ( strcmp(text, "GrdFileType_None"          ) == 0 )   { t = GrdFileType_None;             return true; }
else if ( strcmp(text, "GrdFileType_Gb1"           ) == 0 )   { t = GrdFileType_Gb1;              return true; }
else if ( strcmp(text, "GrdFileType_Gb2"           ) == 0 )   { t = GrdFileType_Gb2;              return true; }
else if ( strcmp(text, "GrdFileType_NcMet"         ) == 0 )   { t = GrdFileType_NcMet;            return true; }
else if ( strcmp(text, "GrdFileType_General_Netcdf") == 0 )   { t = GrdFileType_General_Netcdf;   return true; }

else if ( strcmp(text, "GrdFileType_NcWrf"         ) == 0 )   { t = GrdFileType_NcWrf;            return true; }
else if ( strcmp(text, "GrdFileType_NcPinterp"     ) == 0 )   { t = GrdFileType_NcPinterp;        return true; }
else if ( strcmp(text, "GrdFileType_NcCF"          ) == 0 )   { t = GrdFileType_NcCF;             return true; }
else if ( strcmp(text, "GrdFileType_HdfEos"        ) == 0 )   { t = GrdFileType_HdfEos;           return true; }
else if ( strcmp(text, "GrdFileType_Bufr"          ) == 0 )   { t = GrdFileType_Bufr;             return true; }

else if ( strcmp(text, "GrdFileType_Python_Numpy"  ) == 0 )   { t = GrdFileType_Python_Numpy;     return true; }
else if ( strcmp(text, "GrdFileType_Python_Xarray" ) == 0 )   { t = GrdFileType_Python_Xarray;    return true; }
else if ( strcmp(text, "GrdFileType_UGrid"         ) == 0 )   { t = GrdFileType_UGrid;            return true; }
   //
   //  nope
   //

return false;

}


////////////////////////////////////////////////////////////////////////


