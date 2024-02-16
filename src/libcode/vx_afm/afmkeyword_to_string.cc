// *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*
// ** Copyright UCAR (c) 1992 - 2024
// ** University Corporation for Atmospheric Research (UCAR)
// ** National Center for Atmospheric Research (NCAR)
// ** Research Applications Lab (RAL)
// ** P.O.Box 3000, Boulder, Colorado, 80307-3000, USA
// *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*


////////////////////////////////////////////////////////////////////////


   //
   //  Warning:
   //
   //     This file is machine generated.
   //
   //     Do not edit by hand.
   //
   //     Created by enum_to_string from file "afm_keywords.h"
   //


////////////////////////////////////////////////////////////////////////


using namespace std;


#include <string.h>

#include "afmkeyword_to_string.h"


////////////////////////////////////////////////////////////////////////


ConcatString afmkeyword_to_string(const AfmKeyword t)

{

const char * s = (const char *) 0;

switch ( t )  {

   case afm_keyword_StartFontMetrics:     s = "afm_keyword_StartFontMetrics";     break;
   case afm_keyword_EndFontMetrics:       s = "afm_keyword_EndFontMetrics";       break;
   case afm_keyword_FontName:             s = "afm_keyword_FontName";             break;
   case afm_keyword_FullName:             s = "afm_keyword_FullName";             break;
   case afm_keyword_FamilyName:           s = "afm_keyword_FamilyName";           break;

   case afm_keyword_Weight:               s = "afm_keyword_Weight";               break;
   case afm_keyword_FontBBox:             s = "afm_keyword_FontBBox";             break;
   case afm_keyword_Version:              s = "afm_keyword_Version";              break;
   case afm_keyword_Notice:               s = "afm_keyword_Notice";               break;
   case afm_keyword_EncodingScheme:       s = "afm_keyword_EncodingScheme";       break;

   case afm_keyword_MappingScheme:        s = "afm_keyword_MappingScheme";        break;
   case afm_keyword_EscChar:              s = "afm_keyword_EscChar";              break;
   case afm_keyword_CharacterSet:         s = "afm_keyword_CharacterSet";         break;
   case afm_keyword_Characters:           s = "afm_keyword_Characters";           break;
   case afm_keyword_IsBaseFont:           s = "afm_keyword_IsBaseFont";           break;

   case afm_keyword_VVector:              s = "afm_keyword_VVector";              break;
   case afm_keyword_IsFixedV:             s = "afm_keyword_IsFixedV";             break;
   case afm_keyword_CapHeight:            s = "afm_keyword_CapHeight";            break;
   case afm_keyword_XHeight:              s = "afm_keyword_XHeight";              break;
   case afm_keyword_Ascender:             s = "afm_keyword_Ascender";             break;

   case afm_keyword_Descender:            s = "afm_keyword_Descender";            break;
   case afm_keyword_Comment:              s = "afm_keyword_Comment";              break;
   case afm_keyword_ItalicAngle:          s = "afm_keyword_ItalicAngle";          break;
   case afm_keyword_IsFixedPitch:         s = "afm_keyword_IsFixedPitch";         break;
   case afm_keyword_UnderlinePosition:    s = "afm_keyword_UnderlinePosition";    break;

   case afm_keyword_UnderlineThickness:   s = "afm_keyword_UnderlineThickness";   break;
   case afm_keyword_StartCharMetrics:     s = "afm_keyword_StartCharMetrics";     break;
   case afm_keyword_EndCharMetrics:       s = "afm_keyword_EndCharMetrics";       break;
   case afm_keyword_StartComposites:      s = "afm_keyword_StartComposites";      break;
   case afm_keyword_EndComposites:        s = "afm_keyword_EndComposites";        break;

   case afm_keyword_StartKernData:        s = "afm_keyword_StartKernData";        break;
   case afm_keyword_EndKernData:          s = "afm_keyword_EndKernData";          break;
   case afm_keyword_StartKernPairs:       s = "afm_keyword_StartKernPairs";       break;
   case afm_keyword_EndKernPairs:         s = "afm_keyword_EndKernPairs";         break;
   case afm_keyword_C:                    s = "afm_keyword_C";                    break;

   case afm_keyword_CC:                   s = "afm_keyword_CC";                   break;
   case afm_keyword_PCC:                  s = "afm_keyword_PCC";                  break;
   case afm_keyword_KPX:                  s = "afm_keyword_KPX";                  break;
   case afm_keyword_WX:                   s = "afm_keyword_WX";                   break;
   case afm_keyword_N:                    s = "afm_keyword_N";                    break;

   case afm_keyword_B:                    s = "afm_keyword_B";                    break;
   case afm_keyword_L:                    s = "afm_keyword_L";                    break;
   case no_afm_keyword:                   s = "no_afm_keyword";                   break;

   default:
      s = "(bad value)";
      break;

}   //  switch


return ( ConcatString (s) );

}


////////////////////////////////////////////////////////////////////////


