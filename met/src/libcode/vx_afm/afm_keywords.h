// *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*
// ** Copyright UCAR (c) 1992 - 2020
// ** University Corporation for Atmospheric Research (UCAR)
// ** National Center for Atmospheric Research (NCAR)
// ** Research Applications Lab (RAL)
// ** P.O.Box 3000, Boulder, Colorado, 80307-3000, USA
// *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*



////////////////////////////////////////////////////////////////////////


#ifndef  __AFM_KEYWORDS_H__
#define  __AFM_KEYWORDS_H__

#include "concat_string.h"
////////////////////////////////////////////////////////////////////////


enum AfmKeyword {

   afm_keyword_StartFontMetrics,
   afm_keyword_EndFontMetrics,
   afm_keyword_FontName,
   afm_keyword_FullName,
   afm_keyword_FamilyName,
   afm_keyword_Weight,
   afm_keyword_FontBBox,
   afm_keyword_Version,
   afm_keyword_Notice,
   afm_keyword_EncodingScheme,
   afm_keyword_MappingScheme,
   afm_keyword_EscChar,
   afm_keyword_CharacterSet,
   afm_keyword_Characters,
   afm_keyword_IsBaseFont,
   afm_keyword_VVector,
   afm_keyword_IsFixedV,
   afm_keyword_CapHeight,
   afm_keyword_XHeight,
   afm_keyword_Ascender,
   afm_keyword_Descender,
   afm_keyword_Comment,
   afm_keyword_ItalicAngle,
   afm_keyword_IsFixedPitch,
   afm_keyword_UnderlinePosition,
   afm_keyword_UnderlineThickness,
   afm_keyword_StartCharMetrics,
   afm_keyword_EndCharMetrics,
   afm_keyword_StartComposites,
   afm_keyword_EndComposites,
   afm_keyword_StartKernData,
   afm_keyword_EndKernData,
   afm_keyword_StartKernPairs,
   afm_keyword_EndKernPairs,
   afm_keyword_C,
   afm_keyword_CC,
   afm_keyword_PCC,
   afm_keyword_KPX,
   afm_keyword_WX,
   afm_keyword_N,
   afm_keyword_B,
   afm_keyword_L,

   no_afm_keyword

};


////////////////////////////////////////////////////////////////////////


struct AfmKeywordInfo {

   const char * text;

   AfmKeyword key;

};


////////////////////////////////////////////////////////////////////////


static const AfmKeywordInfo kw_info[42] = {

   { "StartFontMetrics",   afm_keyword_StartFontMetrics   },
   { "EndFontMetrics",     afm_keyword_EndFontMetrics     },
   { "FontName",           afm_keyword_FontName           },
   { "FullName",           afm_keyword_FullName           },
   { "FamilyName",         afm_keyword_FamilyName         },
   { "Weight",             afm_keyword_Weight             },
   { "FontBBox",           afm_keyword_FontBBox           },
   { "Version",            afm_keyword_Version            },
   { "Notice",             afm_keyword_Notice             },
   { "EncodingScheme",     afm_keyword_EncodingScheme     },
   { "MappingScheme",      afm_keyword_MappingScheme      },
   { "EscChar",            afm_keyword_EscChar            },
   { "CharacterSet",       afm_keyword_CharacterSet       },
   { "Characters",         afm_keyword_Characters         },
   { "IsBaseFont",         afm_keyword_IsBaseFont         },
   { "VVector",            afm_keyword_VVector            },
   { "IsFixedV",           afm_keyword_IsFixedV           },
   { "CapHeight",          afm_keyword_CapHeight          },
   { "XHeight",            afm_keyword_XHeight            },
   { "Ascender",           afm_keyword_Ascender           },
   { "Descender",          afm_keyword_Descender          },
   { "Comment",            afm_keyword_Comment            },
   { "ItalicAngle",        afm_keyword_ItalicAngle        },
   { "IsFixedPitch",       afm_keyword_IsFixedPitch       },
   { "UnderlinePosition",  afm_keyword_UnderlinePosition  },
   { "UnderlineThickness", afm_keyword_UnderlineThickness },
   { "StartCharMetrics",   afm_keyword_StartCharMetrics   },
   { "EndCharMetrics",     afm_keyword_EndCharMetrics     },
   { "StartComposites",    afm_keyword_StartComposites    },
   { "EndComposites",      afm_keyword_EndComposites      },
   { "StartKernData",      afm_keyword_StartKernData      },
   { "EndKernData",        afm_keyword_EndKernData        },
   { "StartKernPairs",     afm_keyword_StartKernPairs     },
   { "EndKernPairs",       afm_keyword_EndKernPairs       },
   { "C",                  afm_keyword_C                  },
   { "CC",                 afm_keyword_CC                 },
   { "PCC",                afm_keyword_PCC                },
   { "KPX",                afm_keyword_KPX                },
   { "WX",                 afm_keyword_WX                 },
   { "N",                  afm_keyword_N                  },
   { "B",                  afm_keyword_B                  },
   { "L",                  afm_keyword_L                  }

};


////////////////////////////////////////////////////////////////////////


static const int n_kw_infos = 42;


////////////////////////////////////////////////////////////////////////


extern int is_afm_keyword (ConcatString text, AfmKeyword &);


////////////////////////////////////////////////////////////////////////


#endif   //  __AFM_KEYWORDS_H__


////////////////////////////////////////////////////////////////////////


