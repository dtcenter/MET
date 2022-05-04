// *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*
// ** Copyright UCAR (c) 1992 - 2022
// ** University Corporation for Atmospheric Research (UCAR)
// ** National Center for Atmospheric Research (NCAR)
// ** Research Applications Lab (RAL)
// ** P.O.Box 3000, Boulder, Colorado, 80307-3000, USA
// *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*



////////////////////////////////////////////////////////////////////////


using namespace std;


#include <string.h>

#include "afmkeyword_to_string.h"


////////////////////////////////////////////////////////////////////////


ConcatString afmkeyword_to_string(const AfmKeyword t)

{

ConcatString out;

switch ( t )  {

   case afm_keyword_StartFontMetrics:     out = "afm_keyword_StartFontMetrics";     break;
   case afm_keyword_EndFontMetrics:       out = "afm_keyword_EndFontMetrics";       break;
   case afm_keyword_FontName:             out = "afm_keyword_FontName";             break;
   case afm_keyword_FullName:             out = "afm_keyword_FullName";             break;
   case afm_keyword_FamilyName:           out = "afm_keyword_FamilyName";           break;

   case afm_keyword_Weight:               out = "afm_keyword_Weight";               break;
   case afm_keyword_FontBBox:             out = "afm_keyword_FontBBox";             break;
   case afm_keyword_Version:              out = "afm_keyword_Version";              break;
   case afm_keyword_Notice:               out = "afm_keyword_Notice";               break;
   case afm_keyword_EncodingScheme:       out = "afm_keyword_EncodingScheme";       break;

   case afm_keyword_MappingScheme:        out = "afm_keyword_MappingScheme";        break;
   case afm_keyword_EscChar:              out = "afm_keyword_EscChar";              break;
   case afm_keyword_CharacterSet:         out = "afm_keyword_CharacterSet";         break;
   case afm_keyword_Characters:           out = "afm_keyword_Characters";           break;
   case afm_keyword_IsBaseFont:           out = "afm_keyword_IsBaseFont";           break;

   case afm_keyword_VVector:              out = "afm_keyword_VVector";              break;
   case afm_keyword_IsFixedV:             out = "afm_keyword_IsFixedV";             break;
   case afm_keyword_CapHeight:            out = "afm_keyword_CapHeight";            break;
   case afm_keyword_XHeight:              out = "afm_keyword_XHeight";              break;
   case afm_keyword_Ascender:             out = "afm_keyword_Ascender";             break;

   case afm_keyword_Descender:            out = "afm_keyword_Descender";            break;
   case afm_keyword_Comment:              out = "afm_keyword_Comment";              break;
   case afm_keyword_ItalicAngle:          out = "afm_keyword_ItalicAngle";          break;
   case afm_keyword_IsFixedPitch:         out = "afm_keyword_IsFixedPitch";         break;
   case afm_keyword_UnderlinePosition:    out = "afm_keyword_UnderlinePosition";    break;

   case afm_keyword_UnderlineThickness:   out = "afm_keyword_UnderlineThickness";   break;
   case afm_keyword_StartCharMetrics:     out = "afm_keyword_StartCharMetrics";     break;
   case afm_keyword_EndCharMetrics:       out = "afm_keyword_EndCharMetrics";       break;
   case afm_keyword_StartComposites:      out = "afm_keyword_StartComposites";      break;
   case afm_keyword_EndComposites:        out = "afm_keyword_EndComposites";        break;

   case afm_keyword_StartKernData:        out = "afm_keyword_StartKernData";        break;
   case afm_keyword_EndKernData:          out = "afm_keyword_EndKernData";          break;
   case afm_keyword_StartKernPairs:       out = "afm_keyword_StartKernPairs";       break;
   case afm_keyword_EndKernPairs:         out = "afm_keyword_EndKernPairs";         break;
   case afm_keyword_C:                    out = "afm_keyword_C";                    break;

   case afm_keyword_CC:                   out = "afm_keyword_CC";                   break;
   case afm_keyword_PCC:                  out = "afm_keyword_PCC";                  break;
   case afm_keyword_KPX:                  out = "afm_keyword_KPX";                  break;
   case afm_keyword_WX:                   out = "afm_keyword_WX";                   break;
   case afm_keyword_N:                    out = "afm_keyword_N";                    break;

   case afm_keyword_B:                    out = "afm_keyword_B";                    break;
   case afm_keyword_L:                    out = "afm_keyword_L";                    break;
   case no_afm_keyword:                   out = "no_afm_keyword";                   break;

   default:
      out = "(bad value)";
      break;

}   //  switch


return ( out );

}


////////////////////////////////////////////////////////////////////////


