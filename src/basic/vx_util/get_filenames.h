// *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*
// ** Copyright UCAR (c) 1992 - 2025
// ** University Corporation for Atmospheric Research (UCAR)
// ** National Center for Atmospheric Research (NCAR)
// ** Research Applications Lab (RAL)
// ** P.O.Box 3000, Boulder, Colorado, 80307-3000, USA
// *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*



////////////////////////////////////////////////////////////////////////


#ifndef  __GET_FILENAMES__
#define  __GET_FILENAMES__


////////////////////////////////////////////////////////////////////////


#include "vx_log.h"


////////////////////////////////////////////////////////////////////////


extern StringArray get_filenames(const StringArray & search_dir_list,
                      const char * regex1, const char * regex2,
                      bool check_regular = false);

extern StringArray get_filenames(const ConcatString & search_dir,
                      const char * regex1, const char * regex2,
                      bool check_regular = false);

extern StringArray get_filenames_from_dir(const char * directory_path,
                      const char * regex1, const char * regex2);

extern bool        check_filename_regex(const char * path,
                      const char * regex1, const char * regex2);

extern bool        check_prefix_suffix(const char * path,
                      const char * prefix, const char * suffix);


////////////////////////////////////////////////////////////////////////


#endif   //  __GET_FILENAMES__


////////////////////////////////////////////////////////////////////////


