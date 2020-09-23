// *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*
// ** Copyright UCAR (c) 1992 - 2020
// ** University Corporation for Atmospheric Research (UCAR)
// ** National Center for Atmospheric Research (NCAR)
// ** Research Applications Lab (RAL)
// ** P.O.Box 3000, Boulder, Colorado, 80307-3000, USA
// *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*


////////////////////////////////////////////////////////////////////////


#ifndef  __GS_PS_FONT_MAP_H__
#define  __GS_PS_FONT_MAP_H__


////////////////////////////////////////////////////////////////////////


// #include "standard_ps_fonts.h"


////////////////////////////////////////////////////////////////////////


struct GsPsMapInfo {

   const char * gs_pfb_name;

   const char * ps_font_name;

   int number;   //  my ps font number

};


////////////////////////////////////////////////////////////////////////


static const GsPsMapInfo gs_ps_map_info [total_vx_ps_fonts] = {


   { "a010013l",  "AvantGarde-Book",                  0 }, 
   { "a010033l",  "AvantGarde-BookOblique",           1 }, 
   { "a010015l",  "AvantGarde-Demi",                  2 }, 

   { "b018015l",  "Bookman-Demi",                     3 }, 
   { "b018035l",  "Bookman-DemiItalic",               4 }, 
   { "b018012l",  "Bookman-Light",                    5 }, 
   { "b018032l",  "Bookman-LightItalic",              6 }, 

   { "n022003l",  "Courier",                          7 }, 
   { "n022004l",  "Courier-Bold",                     8 }, 
   { "n022024l",  "Courier-BoldOblique",              9 }, 
   { "n022023l",  "Courier-Oblique",                 10 }, 

   { "n019003l",  "Helvetica",                       11 }, 
   { "n019004l",  "Helvetica-Bold",                  12 }, 
   { "n019024l",  "Helvetica-BoldOblique",           13 }, 
   { "n019043l",  "Helvetica-Narrow",                14 }, 
   { "n019044l",  "Helvetica-Narrow-Bold",           15 }, 
   { "n019064l",  "Helvetica-Narrow-BoldOblique",    16 }, 
   { "n019063l",  "Helvetica-Narrow-Oblique",        17 }, 
   { "n019023l",  "Helvetica-Oblique",               18 }, 

   { "c059016l",  "NewCenturySchlbk-Bold",           19 }, 
   { "c059036l",  "NewCenturySchlbk-BoldItalic",     20 }, 
   { "c059033l",  "NewCenturySchlbk-Italic",         21 }, 
   { "c059013l",  "NewCenturySchlbk-Roman",          22 }, 

   { "p052004l",  "Palatino-Bold",                   23 }, 
   { "p052024l",  "Palatino-BoldItalic",             24 }, 
   { "p052023l",  "Palatino-Italic",                 25 }, 
   { "p052003l",  "Palatino-Roman",                  26 }, 

   { "s050000l",  "Symbol",                          27 }, 

   { "n021004l",  "Times-Bold",                      28 }, 
   { "n021024l",  "Times-BoldItalic",                29 }, 
   { "n021023l",  "Times-Italic",                    30 }, 
   { "n021003l",  "Times-Roman",                     31 }, 

   { "z003034l",  "ZapfChancery-MediumItalic",       32 }, 

   { "d050000l",  "ZapfDingbats",                    33 }


};


////////////////////////////////////////////////////////////////////////


#endif   /*  __GS_PS_FONT_MAP_H__  */


////////////////////////////////////////////////////////////////////////


