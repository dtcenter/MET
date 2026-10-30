// *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*
// ** Copyright UCAR (c) 1992 - 2026
// ** University Corporation for Atmospheric Research (UCAR)
// ** National Center for Atmospheric Research (NCAR)
// ** Research Applications Lab (RAL)
// ** P.O.Box 3000, Boulder, Colorado, 80307-3000, USA
// *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*
////////////////////////////////////////////////////////////////////////

#include <array>
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
// Reject control characters and over-long paths, neither of which can
// appear in a legitimate MET file name.  Embedded nulls need no check:
// the path is a C string, so a null is its terminator.
//
////////////////////////////////////////////////////////////////////////

bool is_valid_path(const char *path, const char *caller) {

   if(!path) return false;

   // PATH_MAX counts the terminating null
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
// Wrappers naming a path that must already exist also canonicalize it
// with realpath().  Those that create one cannot: realpath() fails on
// a path that does not exist yet.
//
////////////////////////////////////////////////////////////////////////

int met_open(const char *path, int oflag) {
   ConcatString s = replace_path(path);
   if(!is_valid_path(s.c_str(), "met_open(const char *, int) -> ")) {
      errno = EINVAL;
      return -1;
   }

   std::array<char, PATH_MAX> real_path{};
   if(!realpath(s.c_str(), real_path.data())) return -1;

   return open(real_path.data(), oflag);
}

////////////////////////////////////////////////////////////////////////

void met_open(ifstream &in, const char *path) {
   ConcatString s = replace_path(path);
   if(!is_valid_path(s.c_str(), "met_open(ifstream &, const char *) -> ")) {
      in.setstate(ios::failbit);
      return;
   }

   std::array<char, PATH_MAX> real_path{};
   if(!realpath(s.c_str(), real_path.data())) {
      in.setstate(ios::failbit);
      return;
   }

   in.open(real_path.data());
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

   // only a read mode names a file that must already exist
   if(*mode == 'r') {
      std::array<char, PATH_MAX> real_path{};
      if(!realpath(s.c_str(), real_path.data())) return nullptr;
      return fopen(real_path.data(), mode);
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

   std::array<char, PATH_MAX> real_path{};
   if(!realpath(s.c_str(), real_path.data())) return nullptr;

   return opendir(real_path.data());
}

////////////////////////////////////////////////////////////////////////

int met_remove(const char *path) {
   ConcatString s = replace_path(path);
   if(!is_valid_path(s.c_str(), "met_remove() -> ")) {
      errno = EINVAL;
      return -1;
   }
   // realpath() resolves links, so this removes the target, not the
   // link.  MET only removes files it created.
   std::array<char, PATH_MAX> real_path{};
   if(!realpath(s.c_str(), real_path.data())) return -1;

   return remove(real_path.data());
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
