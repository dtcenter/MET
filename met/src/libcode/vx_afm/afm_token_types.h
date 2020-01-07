// *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*
// ** Copyright UCAR (c) 1992 - 2020
// ** University Corporation for Atmospheric Research (UCAR)
// ** National Center for Atmospheric Research (NCAR)
// ** Research Applications Lab (RAL)
// ** P.O.Box 3000, Boulder, Colorado, 80307-3000, USA
// *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*



////////////////////////////////////////////////////////////////////////


#ifndef  __AFM_TOKEN_TYPES_H__
#define  __AFM_TOKEN_TYPES_H__


////////////////////////////////////////////////////////////////////////


enum AfmTokenType {

   afm_token_string, 
   afm_token_name, 
   afm_token_number, 
   afm_token_integer, 
   afm_token_boolean,

   afm_token_keyword,

   afm_token_endofline, 

   no_afm_token_type

};


////////////////////////////////////////////////////////////////////////


#endif   //  __AFM_TOKEN_TYPES_H__


////////////////////////////////////////////////////////////////////////


