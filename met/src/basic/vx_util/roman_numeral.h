// *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*
// ** Copyright UCAR (c) 1992 - 2011
// ** University Corporation for Atmospheric Research (UCAR)
// ** National Center for Atmospheric Research (NCAR)
// ** Research Applications Lab (RAL)
// ** P.O.Box 3000, Boulder, Colorado, 80307-3000, USA
// *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*



////////////////////////////////////////////////////////////////////////


#ifndef  __VX_ROMAN_NUMERALS_H__
#define  __VX_ROMAN_NUMERALS_H__


////////////////////////////////////////////////////////////////////////


static const int roman_numeral_min =    1;
static const int roman_numeral_max = 3999;


////////////////////////////////////////////////////////////////////////


extern void roman_numeral(int, char * out, const int lower_case_flag = 0);


////////////////////////////////////////////////////////////////////////


#endif   //  __VX_ROMAN_NUMERALS_H__


////////////////////////////////////////////////////////////////////////


