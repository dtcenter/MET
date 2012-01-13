// *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*
// ** Copyright UCAR (c) 1992 - 2007
// ** University Corporation for Atmospheric Research (UCAR)
// ** National Center for Atmospheric Research (NCAR)
// ** Research Applications Lab (RAL)
// ** P.O.Box 3000, Boulder, Colorado, 80307-3000, USA
// *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*

///////////////////////////////////////////////////////////////////////////////
//
//   Filename:   filterbox.h
//
//   Description:
//      Contains the declaration of the FilterBox class.
//
//   Mod#   Date      Name            Description
//   ----   ----      ----            -----------
//   000    11-03-06  Halley Gotway
//
///////////////////////////////////////////////////////////////////////////////

#ifndef  __DATA2D_UTIL_FILTERBOX_H__
#define  __DATA2D_UTIL_FILTERBOX_H__

///////////////////////////////////////////////////////////////////////////////

class FilterBox {

   private:

      double *z;

      int *on;

      int xmin;
      int xmax;

      int ymin;
      int ymax;

      int nx;
      int ny;

      void assign(const FilterBox &);

      void clear();

      void alloc(int);

      int two_to_one(int x, int y) const;

   public:

      FilterBox();
      ~FilterBox();
      FilterBox(const FilterBox &);
      FilterBox & operator=(const FilterBox &);

      void set_cylinder_height(int radius, double height);
      void set_cylinder_volume(int radius, double volume);

      void set_cone(int radius, int height);

      void set_box_height(int x_half_side, int y_half_side, double height);
      void set_box_volume(int x_half_side, int y_half_side, double volume);

      int get_nx() const;
      int get_ny() const;

      int get_xmin() const;
      int get_xmax() const;

      int get_ymin() const;
      int get_ymax() const;

      double get_value(int x, int y) const;

      int is_on(int x, int y) const;
};

///////////////////////////////////////////////////////////////////////////////

inline int FilterBox::two_to_one(int x, int y) const {
   return ( (y - ymin)*nx + (x - xmin) ); }

inline int FilterBox::get_nx() const { return ( nx ); }
inline int FilterBox::get_ny() const { return ( ny ); }

inline int FilterBox::get_xmax() const { return ( xmax ); }
inline int FilterBox::get_xmin() const { return ( xmin ); }

inline int FilterBox::get_ymax() const { return ( ymax ); }
inline int FilterBox::get_ymin() const { return ( ymin ); }

inline double FilterBox::get_value(int x, int y) const {
   return (  z[two_to_one(x, y)] ); }

inline int FilterBox::is_on(int x, int y) const {
   return ( on[two_to_one(x, y)] ); }

///////////////////////////////////////////////////////////////////////////////

#endif   //  __DATA2D_UTIL_FILTERBOX_H__

///////////////////////////////////////////////////////////////////////////////
