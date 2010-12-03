// *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*
// ** Copyright UCAR (c) 1992 - 2007
// ** University Corporation for Atmospheric Research (UCAR)
// ** National Center for Atmospheric Research (NCAR)
// ** Research Applications Lab (RAL)
// ** P.O.Box 3000, Boulder, Colorado, 80307-3000, USA
// *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*



////////////////////////////////////////////////////////////////////////


#ifndef  __MET_ANALYSIS_TOOL_UTILS_H__
#define  __MET_ANALYSIS_TOOL_UTILS_H__


////////////////////////////////////////////////////////////////////////


#include "vx_util/vx_util.h"
#include "vx_cal/vx_cal.h"


////////////////////////////////////////////////////////////////////////


   //
   //  parses a space delimited string of options into a string array
   //


extern StringArray parse_line(const char * line);


   //
   //  tells if a string is all digits
   //


extern int all_digits(const char *);


////////////////////////////////////////////////////////////////////////


#endif   /*  __MET_ANALYSIS_TOOL_UTILS_H__  */


////////////////////////////////////////////////////////////////////////

