// *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*
// ** Copyright UCAR (c) 1992 - 2026
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


#include <vector>
#include <memory>
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

      void dump(std::ostream &, int depth = 0) const;

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

      void dump(std::ostream &, int depth = 0) const;


      ConcatString successor_name;

      int successor_index;   //  index into the cm array

      ConcatString ligature_name;

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

      void dump(std::ostream &, int depth = 0) const;


      int ascii_code;

      ConcatString name;

      int width;

      AfmBBox bbox;

      int n_ligatures;

      std::vector<LigatureInfo> linfo;

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

      void dump(std::ostream &, int depth = 0) const;

      ConcatString name;

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

      void dump(std::ostream &, int depth = 0) const;

      ConcatString name;

      int n_parts;

      std::vector<PCC> pcc;

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

      void dump(std::ostream &, int depth = 0) const;


      ConcatString name1;

      ConcatString name2;

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

      std::unique_ptr<std::ifstream> in;      //  used for reading
      int line_number;    //

      void patch_ligatures(LigatureInfo &);

   public:

      Afm();
     ~Afm();
      Afm(const Afm &);
      Afm & operator=(const Afm &);

      void clear();

      void dump(std::ostream &, int depth = 0) const;

      int read(const ConcatString&);

      int lookup_cm(int ascii_code)     const;
      int lookup_cm(const char * name)  const;

      int has_ligature  (int ascii_code_1, int ascii_code_2, LigatureInfo &) const;
      int has_kern_pair (int ascii_code_1, int ascii_code_2, KPX &) const;

         //
         //  data
         //

      ConcatString FontName;
      ConcatString FullName;
      ConcatString FamilyName;
      ConcatString Weight;
      ConcatString Version;
      ConcatString EncodingScheme;

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

      std::vector<AfmCharMetrics> cm;


      int n_composites;

      std::vector<AfmCompositeInfo> compinfo;


      int n_kern_pairs;

      std::vector<KPX> kpx;


};


////////////////////////////////////////////////////////////////////////


#endif   //  __MYSTUFF_AFM_H__


////////////////////////////////////////////////////////////////////////


