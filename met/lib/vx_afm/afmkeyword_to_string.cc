// *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*
// ** Copyright UCAR (c) 1992 - 2007
// ** University Corporation for Atmospheric Research (UCAR)
// ** National Center for Atmospheric Research (NCAR)
// ** Research Applications Lab (RAL)
// ** P.O.Box 3000, Boulder, Colorado, 80307-3000, USA
// *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*



////////////////////////////////////////////////////////////////////////


   //
   //  Warning:
   //
   //    This file is machine generated.
   //
   //    Do not edit by hand.
   //


////////////////////////////////////////////////////////////////////////


using namespace std;


#include <string.h>

#include "vx_afm/afmkeyword_to_string.h"


////////////////////////////////////////////////////////////////////////


void afmkeyword_to_string(const AfmKeyword t, char * out)

{

switch ( t )  {

   case afm_keyword_StartFontMetrics:     strcpy(out, "afm_keyword_StartFontMetrics");     break;
   case afm_keyword_EndFontMetrics:       strcpy(out, "afm_keyword_EndFontMetrics");       break;
   case afm_keyword_FontName:             strcpy(out, "afm_keyword_FontName");             break;
   case afm_keyword_FullName:             strcpy(out, "afm_keyword_FullName");             break;
   case afm_keyword_FamilyName:           strcpy(out, "afm_keyword_FamilyName");           break;

   case afm_keyword_Weight:               strcpy(out, "afm_keyword_Weight");               break;
   case afm_keyword_FontBBox:             strcpy(out, "afm_keyword_FontBBox");             break;
   case afm_keyword_Version:              strcpy(out, "afm_keyword_Version");              break;
   case afm_keyword_Notice:               strcpy(out, "afm_keyword_Notice");               break;
   case afm_keyword_EncodingScheme:       strcpy(out, "afm_keyword_EncodingScheme");       break;

   case afm_keyword_MappingScheme:        strcpy(out, "afm_keyword_MappingScheme");        break;
   case afm_keyword_EscChar:              strcpy(out, "afm_keyword_EscChar");              break;
   case afm_keyword_CharacterSet:         strcpy(out, "afm_keyword_CharacterSet");         break;
   case afm_keyword_Characters:           strcpy(out, "afm_keyword_Characters");           break;
   case afm_keyword_IsBaseFont:           strcpy(out, "afm_keyword_IsBaseFont");           break;

   case afm_keyword_VVector:              strcpy(out, "afm_keyword_VVector");              break;
   case afm_keyword_IsFixedV:             strcpy(out, "afm_keyword_IsFixedV");             break;
   case afm_keyword_CapHeight:            strcpy(out, "afm_keyword_CapHeight");            break;
   case afm_keyword_XHeight:              strcpy(out, "afm_keyword_XHeight");              break;
   case afm_keyword_Ascender:             strcpy(out, "afm_keyword_Ascender");             break;

   case afm_keyword_Descender:            strcpy(out, "afm_keyword_Descender");            break;
   case afm_keyword_Comment:              strcpy(out, "afm_keyword_Comment");              break;
   case afm_keyword_ItalicAngle:          strcpy(out, "afm_keyword_ItalicAngle");          break;
   case afm_keyword_IsFixedPitch:         strcpy(out, "afm_keyword_IsFixedPitch");         break;
   case afm_keyword_UnderlinePosition:    strcpy(out, "afm_keyword_UnderlinePosition");    break;

   case afm_keyword_UnderlineThickness:   strcpy(out, "afm_keyword_UnderlineThickness");   break;
   case afm_keyword_StartCharMetrics:     strcpy(out, "afm_keyword_StartCharMetrics");     break;
   case afm_keyword_EndCharMetrics:       strcpy(out, "afm_keyword_EndCharMetrics");       break;
   case afm_keyword_StartComposites:      strcpy(out, "afm_keyword_StartComposites");      break;
   case afm_keyword_EndComposites:        strcpy(out, "afm_keyword_EndComposites");        break;

   case afm_keyword_StartKernData:        strcpy(out, "afm_keyword_StartKernData");        break;
   case afm_keyword_EndKernData:          strcpy(out, "afm_keyword_EndKernData");          break;
   case afm_keyword_StartKernPairs:       strcpy(out, "afm_keyword_StartKernPairs");       break;
   case afm_keyword_EndKernPairs:         strcpy(out, "afm_keyword_EndKernPairs");         break;
   case afm_keyword_C:                    strcpy(out, "afm_keyword_C");                    break;

   case afm_keyword_CC:                   strcpy(out, "afm_keyword_CC");                   break;
   case afm_keyword_PCC:                  strcpy(out, "afm_keyword_PCC");                  break;
   case afm_keyword_KPX:                  strcpy(out, "afm_keyword_KPX");                  break;
   case afm_keyword_WX:                   strcpy(out, "afm_keyword_WX");                   break;
   case afm_keyword_N:                    strcpy(out, "afm_keyword_N");                    break;

   case afm_keyword_B:                    strcpy(out, "afm_keyword_B");                    break;
   case afm_keyword_L:                    strcpy(out, "afm_keyword_L");                    break;
   case no_afm_keyword:                   strcpy(out, "no_afm_keyword");                   break;

   default:
      strcpy(out, "(bad value)");
      break;

}   //  switch


return;

}


////////////////////////////////////////////////////////////////////////


