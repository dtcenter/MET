// *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*
// ** Copyright UCAR (c) 1992 - 2011
// ** University Corporation for Atmospheric Research (UCAR)
// ** National Center for Atmospheric Research (NCAR)
// ** Research Applications Lab (RAL)
// ** P.O.Box 3000, Boulder, Colorado, 80307-3000, USA
// *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*



////////////////////////////////////////////////////////////////////////


#ifndef  __STRING_FXNS_H__
#define  __STRING_FXNS_H__


////////////////////////////////////////////////////////////////////////


extern const char * get_short_name(const char * path);

extern void append_char(char *, const char);

extern void strip_char(char *, const char);

extern bool check_reg_exp(const char *, const char *);

extern int num_tokens(const char *, const char *);

extern void replace_string(const char *old_str, const char *new_str, 
                           const char *in_str, char *out_str);


////////////////////////////////////////////////////////////////////////


#endif   //  __STRING_FXNS_H__


////////////////////////////////////////////////////////////////////////


