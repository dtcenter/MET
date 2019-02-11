// *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*
// ** Copyright UCAR (c) 1992 - 2019
// ** University Corporation for Atmospheric Research (UCAR)
// ** National Center for Atmospheric Research (NCAR)
// ** Research Applications Lab (RAL)
// ** P.O.Box 3000, Boulder, Colorado, 80307-3000, USA
// *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*



////////////////////////////////////////////////////////////////////////


#ifndef __RENDERINFO_H__
#define __RENDERINFO_H__


////////////////////////////////////////////////////////////////////////


#include "ps_filter.h"


////////////////////////////////////////////////////////////////////////


class RenderInfo {

   private:

      void init_from_scratch();

      void assign(const RenderInfo &);

      double X_ll;
      double Y_ll;

      double X_mag;
      double Y_mag;

      bool BW;     //  false = color, true = grayscale

      int Filter[max_filters];

      int Nfilters;

   public:

      RenderInfo();
     ~RenderInfo();
      RenderInfo(const RenderInfo &);
      RenderInfo &operator=(const RenderInfo &);

      void clear();

         //
         //  set stuff
         //

     void set_ll  (double _x_ll, double _y_ll);

     void set_mag (double);

     void set_mag (double _x_mag, double _y_mag);

     void set_color(bool = true);

         //
         //  get stuff
         //

     double x_ll () const;
     double y_ll () const;

     double x_mag() const;
     double y_mag() const;

     int n_filters() const;

     int filter(int) const;

     bool is_color() const;
     bool is_bw   () const;

         //
         //  do stuff
         //

      void add_filter(int);

};


////////////////////////////////////////////////////////////////////////


inline double RenderInfo::x_ll () const { return ( X_ll ); }
inline double RenderInfo::y_ll () const { return ( Y_ll ); }

inline double RenderInfo::x_mag() const { return ( X_mag ); }
inline double RenderInfo::y_mag() const { return ( Y_mag ); }

inline int RenderInfo::n_filters() const { return ( Nfilters ); }

inline bool RenderInfo::is_bw    () const { return (   BW ); }
inline bool RenderInfo::is_color () const { return ( ! BW ); }


////////////////////////////////////////////////////////////////////////


#endif   //  __RENDERINFO_H__


////////////////////////////////////////////////////////////////////////


