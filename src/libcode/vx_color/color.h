// *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*
// ** Copyright UCAR (c) 1992 - 2019
// ** University Corporation for Atmospheric Research (UCAR)
// ** National Center for Atmospheric Research (NCAR)
// ** Research Applications Lab (RAL)
// ** P.O.Box 3000, Boulder, Colorado, 80307-3000, USA
// *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*



////////////////////////////////////////////////////////////////////////


#ifndef  __COLOR_H__
#define  __COLOR_H__


////////////////////////////////////////////////////////////////////////


#include <iostream>

#include "color_list.h"


////////////////////////////////////////////////////////////////////////


class Color {

      friend int operator==(const Color &, const Color &);
      friend int operator!=(const Color &, const Color &);

   private:

      unsigned char R;
      unsigned char G;
      unsigned char B;

      void init_from_scratch();

      void assign(const Color &);

   public:

      Color();
      Color(unsigned char);
      Color(unsigned char, unsigned char, unsigned char);
     ~Color();
      Color(const Color &);
      Color & operator=(const Color &);

      unsigned char red   () const;
      unsigned char green () const;
      unsigned char blue  () const;

      void set_rgb(unsigned char, unsigned char, unsigned char);
      void set_hsv(double, double, double);

      void set_gray(unsigned char);

      void tint(const Color &, double t);

      void clear();   //  set to black

      void to_gray();

      void reverse_video();

      void dump(ostream &, int depth = 0) const;

      bool is_gray () const;

};


////////////////////////////////////////////////////////////////////////


inline unsigned char Color::red   () const { return ( R ); }
inline unsigned char Color::green () const { return ( G ); }
inline unsigned char Color::blue  () const { return ( B ); }

inline bool Color::is_gray() const { return ( (R == G) && (G == B) ); }


////////////////////////////////////////////////////////////////////////


   //
   //  converting color to grayscale
   //

extern unsigned char color_to_gray(const Color & c);

   //
   //  rgb coordinates integer types (unsigned chars, actually) that
   //      range from 0 to 255 inclusive.
   //
   //  hsv coordinates are floating-point types (doubles) that
   //      range from 0.0 to 1.0 inclusive
   //

extern void rgb_to_hsv(unsigned char r, unsigned char g, unsigned char b,
                       double & h, double & s, double & v);


extern void hsv_to_rgb(double h, double s, double v,
                       unsigned char & r, unsigned char & g, unsigned char & b);


   //
   //  "double" versions of the rgb/hsv conversions.
   //
   //   all input & output values are between 0 and 1 inclusive
   //

extern void drgb_to_dhsv(double r, double g, double b, double & h, double & s, double & v);

extern void dhsv_to_drgb(double h, double s, double v, double & r, double & g, double & b);


   //
   //  color mixing
   //

extern Color blend_colors(const Color & color0, const Color & color1, double t);


////////////////////////////////////////////////////////////////////////


extern int operator==(const Color &, const Color &);

extern int operator!=(const Color &, const Color &);


////////////////////////////////////////////////////////////////////////


class CtableEntry {

   private:

      double ValueLo;
      double ValueHi;

      Color C;

      void assign(const CtableEntry &);

      void init_from_scratch();

   public:

      CtableEntry();
     ~CtableEntry();
      CtableEntry(const CtableEntry &);
      CtableEntry & operator=(const CtableEntry &);

      void set_value(double);
      void set_values(double, double);

      void set_color(const Color &);

      void clear();



      const Color & color() const;

      double value_low  () const;
      double value_high () const;

      void dump(ostream &, int depth = 0) const;

};


////////////////////////////////////////////////////////////////////////


inline void CtableEntry::set_color(const Color & c) { C = c;  return; }

inline const Color & CtableEntry::color() const { return ( C ); }

inline double CtableEntry::value_low()  const { return ( ValueLo ); }
inline double CtableEntry::value_high() const { return ( ValueHi ); }


////////////////////////////////////////////////////////////////////////


extern istream & operator>>(istream &, CtableEntry &);

extern ostream & operator<<(ostream &, const CtableEntry &);


////////////////////////////////////////////////////////////////////////


static const int ctable_alloc_inc = 30;


////////////////////////////////////////////////////////////////////////


class ColorTable {

   private:

      void assign(const ColorTable &);

      void init_from_scratch();

      void fudge_color(Color &) const;

      void extend(int);


      CtableEntry * Entry;

      int Nentries;

      int Nalloc;

      double Gamma;

      unsigned char fudge[256];


   public:


      ColorTable();
     ~ColorTable();
      ColorTable(const ColorTable &);
      ColorTable & operator=(const ColorTable &);


      void clear();

      int read(const char *);

      int write(const char *);

      void set_gamma(double);

      void set_gray();

      void tint(const Color &, double t);

      void add_entry(const CtableEntry &);

      void sort();   //  sort in increasing order of data value


      Color nearest(double) const;

      Color interp(double) const;

      int n_entries() const;

      double gamma() const;

      void dump(ostream &, int depth = 0) const;

      CtableEntry operator[](int) const;

      double data_min() const;
      double data_min(double) const;
      double data_max() const;
      double data_max(double) const;

      void rescale(double, double, double);

      int rescale_flag;
};


////////////////////////////////////////////////////////////////////////


inline int     ColorTable::n_entries()   const { return ( Nentries ); }

inline double  ColorTable::gamma()       const { return ( Gamma ); }


////////////////////////////////////////////////////////////////////////


#endif   //  __COLOR_H__


////////////////////////////////////////////////////////////////////////


