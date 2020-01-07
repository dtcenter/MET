// *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*
// ** Copyright UCAR (c) 1992 - 2020
// ** University Corporation for Atmospheric Research (UCAR)
// ** National Center for Atmospheric Research (NCAR)
// ** Research Applications Lab (RAL)
// ** P.O.Box 3000, Boulder, Colorado, 80307-3000, USA
// *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*


////////////////////////////////////////////////////////////////////////


#ifndef   __CGRAPH_LOADED_FONTS_H__
#define   __CGRAPH_LOADED_FONTS_H__


////////////////////////////////////////////////////////////////////////


#include <iostream>

#include "ft2build.h"
#include FT_FREETYPE_H
#include FT_OUTLINE_H
#include FT_ERRORS_H

#include "vx_util.h"

#include "afm.h"


////////////////////////////////////////////////////////////////////////


class CgFont {

   private:

      void init_from_scratch();

      void assign(const CgFont &);


   public:

      CgFont();
     ~CgFont();
      CgFont(const CgFont &);
      CgFont & operator=(const CgFont &);

      void clear();

      void dump(ostream &, int = 0) const;

      void set_by_number(int);   //  for builtin fonts


      ConcatString gs_font_dir;   //  filled in by init_from_scratch

      ConcatString full_pfb_name;
      ConcatString short_pfb_name;

      ConcatString full_afm_name;
      ConcatString short_afm_name;

      ConcatString ps_name;

      double orig_ps_size;
      double scaled_ps_size;


      int ps_font_number;   //  if builtin font, -1 else

      FT_Face face;

      Afm * afm;   //  allocated

};


////////////////////////////////////////////////////////////////////////


class CgFontCollection {

   private:

      void init_from_scratch();

      void assign(const CgFontCollection &);

      void extend(int);

      bool have_it(const CgFont &) const;


      int Nelements;

      int Nalloc;

      CgFont ** e;

   public:

      CgFontCollection();
     ~CgFontCollection();
      CgFontCollection(const CgFontCollection &);
      CgFontCollection & operator=(const CgFontCollection &);

      void clear();

      void dump(ostream &, int = 0) const;

      int n_fonts() const;


      void add           (const CgFont &);
      void add_no_repeat (const CgFont &);

      CgFont * lookup_by_ps_name(const char *) const;

      CgFont * lookup_by_ps_font_number(int) const;    //  for builtin fonts


      CgFont * operator[](int) const;

};


////////////////////////////////////////////////////////////////////////


inline int CgFontCollection::n_fonts() const { return ( Nelements ); }


////////////////////////////////////////////////////////////////////////


#endif   /*  __CGRAPH_LOADED_FONTS_H__  */


////////////////////////////////////////////////////////////////////////


