// *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*
// ** Copyright UCAR (c) 1992 - 2024
// ** University Corporation for Atmospheric Research (UCAR)
// ** National Center for Atmospheric Research (NCAR)
// ** Research Applications Lab (RAL)
// ** P.O.Box 3000, Boulder, Colorado, 80307-3000, USA
// *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*



////////////////////////////////////////////////////////////////////////


#ifndef  __PARSE_FILE_LIST__
#define  __PARSE_FILE_LIST__


////////////////////////////////////////////////////////////////////////


#include "vx_config.h"
#include "vx_log.h"


////////////////////////////////////////////////////////////////////////


extern StringArray parse_file_list(const StringArray&);

extern StringArray parse_ascii_file_list(const char * path);

extern GrdFileType parse_file_list_type(const StringArray&);

extern void log_missing_file(const char *method_name,
                             const char *desc_str,
                             const std::string &file_name);


////////////////////////////////////////////////////////////////////////


#endif   //  __PARSE_FILE_LIST__


////////////////////////////////////////////////////////////////////////


