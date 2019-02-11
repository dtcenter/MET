// *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*
// ** Copyright UCAR (c) 1992 - 2019
// ** University Corporation for Atmospheric Research (UCAR)
// ** National Center for Atmospheric Research (NCAR)
// ** Research Applications Lab (RAL)
// ** P.O.Box 3000, Boulder, Colorado, 80307-3000, USA
// *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*



////////////////////////////////////////////////////////////////////////


#ifndef  __HISTOGRAM_H__
#define  __HISTOGRAM_H__


////////////////////////////////////////////////////////////////////////


#include <iostream>


////////////////////////////////////////////////////////////////////////


class Histogram {

   private:

      int * Count;

      int  Nbins;

      double  Bottom;
      double  Delta;

      double  MinValue;
      double  MaxValue;

      int  TooBigCount;
      int  TooSmallCount;

      int  is_empty;

      void init_from_scratch();

      void assign(const Histogram &);

   public:

         //
         //  canonical members
         //

      Histogram();
     ~Histogram();
      Histogram(const Histogram &);
      Histogram & operator=(const Histogram &);

         //
         //  misc members
         //

      void clear();

      void set_nbd(int n, double b, double d);  //  Nbins, Bottom, Delta

         //
         //  add data point
         //

      void add(double x);

         //
         //  constant members
         //

      int nbins  () const;

      double bottom () const;
      double delta  () const;

      double min_data_value() const;
      double max_data_value() const;

      int bin_count(int bin) const;

      int total_bin_count() const;

      int max_bin_count() const;
      int min_bin_count() const;

      int too_big_count() const;
      int too_small_count() const;

};


////////////////////////////////////////////////////////////////////////


inline int Histogram::nbins  () const { return ( Nbins  ); }

inline double Histogram::bottom () const { return ( Bottom ); }
inline double Histogram::delta  () const { return ( Delta  ); }

inline double Histogram::min_data_value  () const { return ( MinValue ); }
inline double Histogram::max_data_value  () const { return ( MaxValue ); }

inline int Histogram::too_big_count    () const { return ( TooBigCount ); }
inline int Histogram::too_small_count  () const { return ( TooSmallCount ); }


////////////////////////////////////////////////////////////////////////


extern ostream & operator<<(ostream &, const Histogram &);


////////////////////////////////////////////////////////////////////////


#endif   //  __HISTOGRAM_H__


////////////////////////////////////////////////////////////////////////


