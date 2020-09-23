// *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*
// ** Copyright UCAR (c) 1992 - 2020
// ** University Corporation for Atmospheric Research (UCAR)
// ** National Center for Atmospheric Research (NCAR)
// ** Research Applications Lab (RAL)
// ** P.O.Box 3000, Boulder, Colorado, 80307-3000, USA
// *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*


////////////////////////////////////////////////////////////////////////


#ifndef  __VX_EMPTY_STRING_H__
#define  __VX_EMPTY_STRING_H__


////////////////////////////////////////////////////////////////////////


inline bool    empty (const char * s)  { return ( !s || !(*s) ); }

inline bool nonempty (const char * s)  { return ( s && (*s) ); }


////////////////////////////////////////////////////////////////////////


#endif   /*  __VX_EMPTY_STRING_H__  */


////////////////////////////////////////////////////////////////////////


