// *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*
// ** Copyright UCAR (c) 1992 - 2023
// ** University Corporation for Atmospheric Research (UCAR)
// ** National Center for Atmospheric Research (NCAR)
// ** Research Applications Lab (RAL)
// ** P.O.Box 3000, Boulder, Colorado, 80307-3000, USA
// *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*

///////////////////////////////////////////////////////////////////////////////


#ifndef  __MODE_DATA_TYPE_H__
#define  __MODE_DATA_TYPE_H__

#include <string>
using std::string;

///////////////////////////////////////////////////////////////////////////////


enum ModeDataType
{
   ModeDataType_Traditional,   // default
   ModeDataType_MvMode_Obs,           // mvmode, obs data only
   ModeDataType_MvMode_Fcst,          // mvmode, fcst data only
   ModeDataType_MvMode_Both           // mvmode, fcst and obs
};


///////////////////////////////////////////////////////////////////////////////

inline string sprintModeDataType(ModeDataType type)
{
   switch (type)
   {
   case ModeDataType_MvMode_Obs:
      return "MvMode_Obs";
   case ModeDataType_MvMode_Fcst:
      return "MvMode_Fcst";
   case ModeDataType_MvMode_Both:
      return "MvMode_Both";
   case ModeDataType_Traditional:
   default:
      return "TraditionalMode";
   }
}

#endif

///////////////////////////////////////////////////////////////////////////////
