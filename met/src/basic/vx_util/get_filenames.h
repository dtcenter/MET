// *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*
// ** Copyright UCAR (c) 1992 - 2016
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


extern StringArray get_filenames(const StringArray & search_dirs, const StringArray & suffix);

extern StringArray get_filenames_from_dir(const char * directory_path, const StringArray & suffix);

extern StringArray parse_ascii_file_list(const char * path);


////////////////////////////////////////////////////////////////////////


#endif   //  __GET_FILENAMES__


////////////////////////////////////////////////////////////////////////


