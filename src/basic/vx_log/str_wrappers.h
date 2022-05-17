// *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*
// ** Copyright UCAR (c) 1992 - 2022
// ** University Corporation for Atmospheric Research (UCAR)
// ** National Center for Atmospheric Research (NCAR)
// ** Research Applications Lab (RAL)
// ** P.O.Box 3000, Boulder, Colorado, 80307-3000, USA
// *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*



////////////////////////////////////////////////////////////////////////


#ifndef  __STR_WRAPPERS_H__
#define  __STR_WRAPPERS_H__


////////////////////////////////////////////////////////////////////////

extern int m_strlen(const char *str);

extern void m_strcpy(char *to_str, const char *from_str, const char *method_name,
                     const char *extra_msg=(char *)0);
extern char *m_strcpy2(const char *from_str, const char *method_name,
                       const char *extra_msg=(char *)0);
extern void m_strncpy(char *to_str, const char *from_str, const int buf_len,
                      const char *method_name, const char *extra_msg=(char *)0,
                      bool truncate=false);

extern void m_rstrip(char *str_buf, const int buf_len=-1);

extern bool is_whitespaces(char cur_char);

////////////////////////////////////////////////////////////////////////


#endif   //  __STR_WRAPPERS_H__


////////////////////////////////////////////////////////////////////////




