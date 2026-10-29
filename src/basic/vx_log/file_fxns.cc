// *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*
// ** Copyright UCAR (c) 1992 - 2026
// ** University Corporation for Atmospheric Research (UCAR)
// ** National Center for Atmospheric Research (NCAR)
// ** Research Applications Lab (RAL)
// ** P.O.Box 3000, Boulder, Colorado, 80307-3000, USA
// *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*
////////////////////////////////////////////////////////////////////////

#include <cstdio>
#include <iostream>
#include <fstream>
#include <climits>
#include <stdlib.h>
#include <errno.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <dirent.h>
#include <fcntl.h>

#include "vx_log.h"

#include "file_fxns.h"

using namespace std;


////////////////////////////////////////////////////////////////////////

bool is_regular_file(const char *path) {
   struct stat path_stat;
   return (!stat(path, &path_stat) && S_ISREG(path_stat.st_mode));
}

////////////////////////////////////////////////////////////////////////

bool is_directory(const char *path) {
   struct stat path_stat;
   return (!stat(path, &path_stat) && S_ISDIR(path_stat.st_mode));
}

////////////////////////////////////////////////////////////////////////
//
// Does replace(met_base_str, MET_BASE) on the output string, first
// checking the MET_BASE environment variable.
//
////////////////////////////////////////////////////////////////////////

ConcatString replace_path(const ConcatString path) {
   ConcatString s, met_base_env, met_base_val;

   // initialize
   s = path;
   
   // Use the MET_BASE environment variable, if set.
   // Otherwise, use the compile-time value.
   if(get_env(met_base_str, met_base_env)) met_base_val = met_base_env;
   else                                    met_base_val = MET_BASE;

   s.replace(met_base_str, met_base_val.c_str());

   return s;
}

////////////////////////////////////////////////////////////////////////

ConcatString replace_path(const char * path) {
   return replace_path(ConcatString(path));
}

////////////////////////////////////////////////////////////////////////
//
// Reject paths that cannot name a real file: at least PATH_MAX
// characters long or containing a control character.  Neither can
// appear in a legitimate MET file name, so this rejects nothing a user
// would ask for.
//
// Embedded null bytes need no check here.  The path arrives as a C
// string, so a null byte is its terminator rather than part of it, and
// nothing upstream can deliver one: the configuration file scanner
// truncates at the null when it builds a ConcatString from the lexeme,
// and the command line and environment are C strings already.
//
////////////////////////////////////////////////////////////////////////

bool is_valid_path(const char *path, const char *caller) {

   if(!path) return false;

      //
      // PATH_MAX counts the terminating null, so a path of exactly
      // PATH_MAX characters is already one too long.  strnlen() stops
      // there rather than walking a string that may not be terminated.
      //

   size_t len = strnlen(path, PATH_MAX);

   if(len >= PATH_MAX) {
      mlog << Error << "\n" << caller
           << "file name is " << PATH_MAX << " characters or longer, and the "
           << "maximum is " << PATH_MAX - 1 << "!\n\n";
      return false;
   }

   for(size_t i=0; i<len; i++) {
      auto c = static_cast<unsigned char>(path[i]);
      if(c < 0x20 || c == 0x7F) {
         mlog << Error << "\n" << caller
              << "file name contains control character " << (int) c
              << " at position " << (int) i << "!\n\n";
         return false;
      }
   }

   return true;
}

////////////////////////////////////////////////////////////////////////
//
// Wrapper functions to expand instances of MET_BASE, validate the
// resulting path, and then call the requested function.  This groups
// all open calls to a single file for ease in tracking Fortify
// compliance.
//
// Where the named file or directory must already exist, the path is
// also canonicalized with realpath(), which resolves "..", "." and
// symbolic links before the call.  met_open(ofstream &) and
// met_mkdir() cannot do this, because realpath() fails on a path that
// does not exist yet, which is exactly what those two are given.
//
////////////////////////////////////////////////////////////////////////

int met_open(const char *path, int oflag) {
   ConcatString s = replace_path(path);
   if(!is_valid_path(s.c_str(), "met_open(const char *, int) -> ")) {
      errno = EINVAL;
      return -1;
   }

   char real_path[PATH_MAX];
   if(!realpath(s.c_str(), real_path)) return -1;

   return open(real_path, oflag);
}

////////////////////////////////////////////////////////////////////////

void met_open(ifstream &in, const char *path) {
   ConcatString s = replace_path(path);
   if(!is_valid_path(s.c_str(), "met_open(ifstream &, const char *) -> ")) {
      in.setstate(ios::failbit);
      return;
   }

   char real_path[PATH_MAX];
   if(!realpath(s.c_str(), real_path)) {
      in.setstate(ios::failbit);
      return;
   }

   in.open(real_path);
   return;
}

////////////////////////////////////////////////////////////////////////

void met_open(ofstream &out, const char *path) {
   ConcatString s = replace_path(path);
   if(!is_valid_path(s.c_str(), "met_open(ofstream &, const char *) -> ")) {
      out.setstate(ios::failbit);
      return;
   }
   out.open(s.c_str());
   return;
}

////////////////////////////////////////////////////////////////////////

FILE *met_fopen(const char *path, const char *mode) {
   ConcatString s = replace_path(path);
   if(!is_valid_path(s.c_str(), "met_fopen() -> ")) {
      errno = EINVAL;
      return nullptr;
   }

   if(!mode) {
      mlog << Error << "\nmet_fopen() -> "
           << "no file mode specified!\n\n";
      errno = EINVAL;
      return nullptr;
   }

      //
      // only a read mode names a file that must already exist
      //

   if(*mode == 'r') {
      char real_path[PATH_MAX];
      if(!realpath(s.c_str(), real_path)) return nullptr;
      return fopen(real_path, mode);
   }

   return fopen(s.c_str(), mode);
}

////////////////////////////////////////////////////////////////////////

DIR *met_opendir(const char *path) {
   ConcatString s = replace_path(path);
   if(!is_valid_path(s.c_str(), "met_opendir() -> ")) {
      errno = EINVAL;
      return nullptr;
   }

   char real_path[PATH_MAX];
   if(!realpath(s.c_str(), real_path)) return nullptr;

   return opendir(real_path);
}

////////////////////////////////////////////////////////////////////////

int met_remove(const char *path) {
   ConcatString s = replace_path(path);
   if(!is_valid_path(s.c_str(), "met_remove() -> ")) {
      errno = EINVAL;
      return -1;
   }
      //
      // realpath() resolves symbolic links, so this removes what a
      // link points at rather than the link itself.  MET only removes
      // files it created, never a link it was handed.
      //

   char real_path[PATH_MAX];
   if(!realpath(s.c_str(), real_path)) return -1;

   return remove(real_path);
}

////////////////////////////////////////////////////////////////////////

int met_mkdir(const char *path, mode_t mode) {
   ConcatString s = replace_path(path);
   if(!is_valid_path(s.c_str(), "met_mkdir() -> ")) {
      errno = EINVAL;
      return -1;
   }
   return mkdir(s.c_str(), mode);
}

////////////////////////////////////////////////////////////////////////

void met_closedir(DIR * & dp) {

   int status = ::closedir(dp);
   dp = nullptr;

   if(status < 0) {

      mlog << Error << "\nmet_closedir(DIR *) -> "
           << "trouble closing directory ... " << strerror(errno)
           << "\n\n";

      exit(1);
   }

   return;
}

////////////////////////////////////////////////////////////////////////
