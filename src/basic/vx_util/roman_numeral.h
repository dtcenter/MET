// *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*
// ** Copyright UCAR (c) 1992 - 2019
// ** University Corporation for Atmospheric Research (UCAR)
// ** National Center for Atmospheric Research (NCAR)
// ** Research Applications Lab (RAL)
// ** P.O.Box 3000, Boulder, Colorado, 80307-3000, USA
// *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*



////////////////////////////////////////////////////////////////////////


#ifndef  __ROMAN_NUMERALS_H__
#define  __ROMAN_NUMERALS_H__


////////////////////////////////////////////////////////////////////////


static const int roman_numeral_min =    1;
static const int roman_numeral_max = 3999;


////////////////////////////////////////////////////////////////////////


extern void roman_numeral(int, char * out, const int lower_case_flag = 0);


////////////////////////////////////////////////////////////////////////


#endif   //  __ROMAN_NUMERALS_H__


////////////////////////////////////////////////////////////////////////


