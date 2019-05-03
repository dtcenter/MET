// *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*
// ** Copyright UCAR (c) 1992 - 2019
// ** University Corporation for Atmospheric Research (UCAR)
// ** National Center for Atmospheric Research (NCAR)
// ** Research Applications Lab (RAL)
// ** P.O.Box 3000, Boulder, Colorado, 80307-3000, USA
// *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*
////////////////////////////////////////////////////////////////////////

#ifndef  __FILE_FXNS_H__
#define  __FILE_FXNS_H__

////////////////////////////////////////////////////////////////////////

#include <cstdio>
#include <iostream>
#include <fstream>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <dirent.h>
#include <fcntl.h>

#include "concat_string.h"
#include "file_fxns.h"

////////////////////////////////////////////////////////////////////////

static const char met_base_str[] = "MET_BASE";

////////////////////////////////////////////////////////////////////////


extern ConcatString replace_path(const ConcatString path);
extern ConcatString replace_path(const char * path);

extern int          met_open(const char *path, int oflag);

extern void         met_open(ifstream &in, const char *path);

extern void         met_open(ofstream &out, const char *path);

extern FILE *       met_fopen(const char *path, const char *mode);


extern DIR *        met_opendir(const char *path);

extern void         met_closedir(DIR * &);


////////////////////////////////////////////////////////////////////////

#endif   //  __FILE_FXNS_H__

////////////////////////////////////////////////////////////////////////



