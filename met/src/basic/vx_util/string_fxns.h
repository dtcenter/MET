// *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*
// ** Copyright UCAR (c) 1992 - 2020
// ** University Corporation for Atmospheric Research (UCAR)
// ** National Center for Atmospheric Research (NCAR)
// ** Research Applications Lab (RAL)
// ** P.O.Box 3000, Boulder, Colorado, 80307-3000, USA
// *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*



////////////////////////////////////////////////////////////////////////


#ifndef  __STRING_FXNS_H__
#define  __STRING_FXNS_H__


////////////////////////////////////////////////////////////////////////


#include "concat_string.h"


////////////////////////////////////////////////////////////////////////


extern bool match_met_version(const char *);

extern void check_met_version(const char *);

extern ConcatString parse_version(const char *, const int);

extern ConcatString parse_version_major(const char *);

extern ConcatString parse_version_major_minor(const char *);

extern const char * get_short_name(const char * path);

extern void append_char(char *, const char);

extern void strip_char(char *, const char);

extern int num_tokens(const char *, const char *);

extern bool has_prefix(const char **prefix_list, int n_prefix,
                       const char *str);

extern int regex_apply(const char* pat, int num_mat, const char* str, char** &mat);

extern void regex_clean(char** &mat);

extern ConcatString str_replace(const char* data, const char* old, const char* repl);

extern ConcatString str_replace_all(const char* data, const char* old, const char* repl);

extern ConcatString str_format(const char *fmt, ...);

extern ConcatString str_trim(const ConcatString str);

extern int parse_thresh_index(const char *str);


////////////////////////////////////////////////////////////////////////


#endif   //  __STRING_FXNS_H__


////////////////////////////////////////////////////////////////////////


