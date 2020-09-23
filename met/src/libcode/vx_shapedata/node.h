// *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*
// ** Copyright UCAR (c) 1992 - 2020
// ** University Corporation for Atmospheric Research (UCAR)
// ** National Center for Atmospheric Research (NCAR)
// ** Research Applications Lab (RAL)
// ** P.O.Box 3000, Boulder, Colorado, 80307-3000, USA
// *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*

///////////////////////////////////////////////////////////////////////////////
//
//   Filename:   node.h
//
//   Description:
//
//
//   Mod#   Date      Name           Description
//   ----   ----      ----           -----------
//   000    11-03-06  Halley Gotway
//
///////////////////////////////////////////////////////////////////////////////

#ifndef  __DATA2D_UTIL_NODE_H__
#define  __DATA2D_UTIL_NODE_H__

///////////////////////////////////////////////////////////////////////////////

#include "polyline.h"
#include "vx_util.h"

///////////////////////////////////////////////////////////////////////////////

class Node {

   private:

      void assign(const Node &);
      void assign_tree(const Node *);

   public:

      Node();
     ~Node();
      Node(const Node &);
      Node & operator=(const Node &);

      void clear();

      /////////////////////////////////////////////////////////////////////////

      Polyline p;

      Node *child;

      Node *sibling;

      void add_child(const Polyline *);

      /////////////////////////////////////////////////////////////////////////

      int n_children() const;

      Node *get_child(int) const;

      int is_closed() const;

      void centroid(double &ubar, double &vbar) const;

      void translate(double du, double dv);

      double angle() const;

      void rotate(double deg);

      void rotate(double deg, double ubar, double vbar);

      double uv_signed_area() const;

      int is_inside(double u_test, double v_test) const;

      int is_polyline_point(double u_test, double v_test) const;

      void bounding_box(Box &) const;

      void sum_first_moments(double &, double &) const;

      void sum_second_moments(double, double,
                              double &, double &, double &) const;
};

///////////////////////////////////////////////////////////////////////////////

extern double node_dist(const Node &, const Node &);

extern double node_polyline_dist(const Node &, const Polyline &);

///////////////////////////////////////////////////////////////////////////////

#endif   //  __DATA2D_UTIL_NODE_H__

///////////////////////////////////////////////////////////////////////////////
