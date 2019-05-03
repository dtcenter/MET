// *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*
// ** Copyright UCAR (c) 1992 - 2015
// ** University Corporation for Atmospheric Research (UCAR)
// ** National Center for Atmospheric Research (NCAR)
// ** Research Applications Lab (RAL)
// ** P.O.Box 3000, Boulder, Colorado, 80307-3000, USA
// *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*
////////////////////////////////////////////////////////////////////////

#ifndef  __THRESHOLD_H__
#define  __THRESHOLD_H__

////////////////////////////////////////////////////////////////////////

#include <iostream>

#include "concat_string.h"
#include "vx_cal.h"

////////////////////////////////////////////////////////////////////////

//
// Enumeration to indicate the different ways of thresholding a field
//
enum ThreshType {
   thresh_na = 0,
   thresh_lt = 1,
   thresh_le = 2,
   thresh_eq = 3,
   thresh_ne = 4,
   thresh_gt = 5,
   thresh_ge = 6,

   thresh_complex = -2,

   no_thresh_type = -1,

};

static const int n_thresh_type = 7;
static const char * const thresh_type_str[n_thresh_type] = {
   "na", "<", "<=", "==", "!=", ">", ">="
};
static const char * const thresh_abbr_str[n_thresh_type] = {
   "na", "lt", "le", "eq", "ne", "gt", "ge"
};

static const int  thresh_default_precision = 3;
static const char thresh_default_sep[]     = ",";

////////////////////////////////////////////////////////////////////////


class ThreshNode {

   protected:

      void threshnode_assign(const ThreshNode *);

   public:

      ThreshNode();
      virtual ~ThreshNode();

      virtual bool check(double) const = 0;

      virtual ThreshNode * copy() const = 0;

      virtual ThreshType type() const = 0;

      virtual double value() const = 0;

      ConcatString s;
      ConcatString abbr_s;

};


////////////////////////////////////////////////////////////////////////


class Or_Node : public ThreshNode {

   public:

      Or_Node();
     ~Or_Node();

      bool check(double) const;

      ThreshNode * copy() const;

      ThreshType type() const;

      double value() const;

      ThreshNode * left_child;
      ThreshNode * right_child;

};


////////////////////////////////////////////////////////////////////////


inline ThreshType Or_Node::type()  const { return ( thresh_complex  ); }
inline double     Or_Node::value() const { return ( bad_data_double ); }


////////////////////////////////////////////////////////////////////////


class And_Node : public ThreshNode {

   public:

      And_Node();
     ~And_Node();

      bool check(double) const;

      ThreshType type() const;

      double value() const;

      ThreshNode * copy() const;

      ThreshNode * left_child;
      ThreshNode * right_child;

};


////////////////////////////////////////////////////////////////////////


inline ThreshType And_Node::type()  const { return ( thresh_complex  ); }
inline double     And_Node::value() const { return ( bad_data_double ); }

////////////////////////////////////////////////////////////////////////


class Not_Node : public ThreshNode {

   public:

      Not_Node();
     ~Not_Node();

      bool check(double) const;

      ThreshType type() const;

      double value() const;

      ThreshNode * copy() const;

      ThreshNode * child;

};


////////////////////////////////////////////////////////////////////////


inline ThreshType Not_Node::type()  const { return ( thresh_complex  ); }
inline double     Not_Node::value() const { return ( bad_data_double ); }


////////////////////////////////////////////////////////////////////////


class Simple_Node : public ThreshNode {

   public:

      Simple_Node();
     ~Simple_Node();


      void set_na();

      ThreshType type() const;

      double value() const;

      bool check(double) const;

      ThreshNode * copy() const;

      double T;

      ThreshType op;

};


////////////////////////////////////////////////////////////////////////


inline ThreshType Simple_Node::type()  const { return ( op ); }
inline double     Simple_Node::value() const { return ( T  ); }


////////////////////////////////////////////////////////////////////////
//
// Class to store a threshold value and type
//
////////////////////////////////////////////////////////////////////////


class SingleThresh {

   private:
      void init_from_scratch();
      void assign(const SingleThresh &);

   public:

      SingleThresh();
      ~SingleThresh();
      SingleThresh(const SingleThresh &);
      SingleThresh(const char *);
      SingleThresh & operator=(const SingleThresh &);

      void dump(ostream &, int = 0) const;

      bool operator==(const SingleThresh &) const;

      ThreshNode * node;   //  allocated

      void         clear();

      void         set(double, ThreshType);
      void         set(const ThreshNode *);
      void         set(const char *);

      void         set_na();

      ThreshType   get_type() const;
      double       get_value() const;

      ConcatString get_str(int precision = thresh_default_precision) const;
      ConcatString get_abbr_str(int precision = thresh_default_precision) const;
      bool         check(double) const;
};

////////////////////////////////////////////////////////////////////////


inline ThreshType SingleThresh::get_type()          const { return ( node ? node->type()       : thresh_na       ); }
inline double     SingleThresh::get_value()         const { return ( node ? node->value()      : bad_data_double ); }
inline bool       SingleThresh::check(double __x__) const { return ( node ? node->check(__x__) : true            ); }


////////////////////////////////////////////////////////////////////////


extern bool check_threshold(double, double, int);


////////////////////////////////////////////////////////////////////////

#endif   //  __THRESHOLD_H__

////////////////////////////////////////////////////////////////////////
