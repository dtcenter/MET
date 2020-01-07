// *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*
// ** Copyright UCAR (c) 1992 - 2020
// ** University Corporation for Atmospheric Research (UCAR)
// ** National Center for Atmospheric Research (NCAR)
// ** Research Applications Lab (RAL)
// ** P.O.Box 3000, Boulder, Colorado, 80307-3000, USA
// *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*



////////////////////////////////////////////////////////////////////////


#ifndef  __MYSTUFF_AFM_H__
#define  __MYSTUFF_AFM_H__


////////////////////////////////////////////////////////////////////////


   //
   //  Reference:  Adobe Font Metrics File Format Specification v4.1
   //


////////////////////////////////////////////////////////////////////////


#include <iostream>

#include "afm_line.h"


////////////////////////////////////////////////////////////////////////


class AfmBBox {

   private:

      void init_from_scratch();

      void assign(const AfmBBox &);

   public:

      AfmBBox();
     ~AfmBBox();
      AfmBBox(const AfmBBox &);
      AfmBBox & operator=(const AfmBBox &);

      void clear();

      void dump(ostream &, int depth = 0) const;

      int L;
      int B;

      int R;
      int T;

};


////////////////////////////////////////////////////////////////////////


class LigatureInfo {

   private:

      void init_from_scratch();

      void assign(const LigatureInfo &);

   public:

      LigatureInfo();
     ~LigatureInfo();
      LigatureInfo(const LigatureInfo &);
      LigatureInfo & operator=(const LigatureInfo &);

      void clear();

      void dump(ostream &, int depth = 0) const;


      char * successor_name;

      int successor_index;   //  index into the cm array

      char * ligature_name;

      int ligature_index;   //  index into the cm array

};


////////////////////////////////////////////////////////////////////////


class AfmCharMetrics {

   private:

      void init_from_scratch();

      void assign(const AfmCharMetrics &);

   public:

      AfmCharMetrics();
     ~AfmCharMetrics();
      AfmCharMetrics(const AfmCharMetrics &);
      AfmCharMetrics & operator=(const AfmCharMetrics &);

      void clear();

      void dump(ostream &, int depth = 0) const;


      int ascii_code;

      char * name;

      int width;

      AfmBBox bbox;

      int n_ligatures;

      LigatureInfo * linfo;

};


////////////////////////////////////////////////////////////////////////


class PCC {

   private:

      void init_from_scratch();

      void assign(const PCC &);

   public:

      PCC();
     ~PCC();
      PCC(const PCC &);
      PCC & operator=(const PCC &);

      void clear();

      void dump(ostream &, int depth = 0) const;

      char * name;

      int delta_x;
      int delta_y;

};


////////////////////////////////////////////////////////////////////////


class AfmCompositeInfo {

   private:

      void init_from_scratch();

      void assign(const AfmCompositeInfo &);

   public:

      AfmCompositeInfo();
     ~AfmCompositeInfo();
      AfmCompositeInfo(const AfmCompositeInfo &);
      AfmCompositeInfo & operator=(const AfmCompositeInfo &);

      void clear();

      void dump(ostream &, int depth = 0) const;

      char * name;

      int n_parts;

      PCC * pcc;

};


////////////////////////////////////////////////////////////////////////


class KPX {

   private:

      void init_from_scratch();

      void assign(const KPX &);

   public:

      KPX();
     ~KPX();
      KPX(const KPX &);
      KPX & operator=(const KPX &);

      void clear();

      void dump(ostream &, int depth = 0) const;


      char * name1;

      char * name2;

      double dx;

};


////////////////////////////////////////////////////////////////////////


class Afm {

   private:

      void init_from_scratch();

      void assign(const Afm &);

      void do_startfontmetrics();
      void do_startcharmetrics();
      void do_startcomposites();
      void do_startkerndata();
      void do_startkernpairs();

      void do_c(AfmLine &, const int);
      void do_cc(AfmLine &, const int);

      void do_fontbbox(AfmLine &);

      ifstream * in;      //  used for reading
      int line_number;    //

      void patch_ligatures(LigatureInfo &);

   public:

      Afm();
     ~Afm();
      Afm(const Afm &);
      Afm & operator=(const Afm &);

      void clear();

      void dump(ostream &, int depth = 0) const;

      int read(const ConcatString&);

      int lookup_cm(int ascii_code)     const;
      int lookup_cm(const char * name)  const;

      int has_ligature  (int ascii_code_1, int ascii_code_2, LigatureInfo &) const;
      int has_kern_pair (int ascii_code_1, int ascii_code_2, KPX &) const;

         //
         //  data
         //

      char * FontName;
      char * FullName;
      char * FamilyName;
      char * Weight;
      char * Version;
      char * EncodingScheme;

      double ItalicAngle;

      int IsFixedPitch;

      double UnderlinePosition;
      double UnderlineThickness;

      AfmBBox FontBBox;

      double CapHeight;
      double XHeight;
      double Ascender;
      double Descender;


      int n_cms;

      AfmCharMetrics * cm;


      int n_composites;

      AfmCompositeInfo * compinfo;


      int n_kern_pairs;

      KPX * kpx;


};


////////////////////////////////////////////////////////////////////////


#endif   //  __MYSTUFF_AFM_H__


////////////////////////////////////////////////////////////////////////


