// *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*
// ** Copyright UCAR (c) 1992 - 2023
// ** University Corporation for Atmospheric Research (UCAR)
// ** National Center for Atmospheric Research (NCAR)
// ** Research Applications Lab (RAL)
// ** P.O.Box 3000, Boulder, Colorado, 80307-3000, USA
// *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*


////////////////////////////////////////////////////////////////////////


#ifndef  __BOOL_TO_STRING_H__
#define  __BOOL_TO_STRING_H__


////////////////////////////////////////////////////////////////////////


inline const char * bool_to_string(bool _tf) { return ( _tf ? "true" : "false" ); }
inline bool string_to_bool(const char *s) { return ( strcasecmp(s, "true") == 0 ? true : false ); }


////////////////////////////////////////////////////////////////////////


#endif   /*  __BOOL_TO_STRING_H__  */


////////////////////////////////////////////////////////////////////////


