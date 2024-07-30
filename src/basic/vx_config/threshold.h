// *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*
// ** Copyright UCAR (c) 1992 - 2024
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
#include <map>
#include <vector>

#include "concat_string.h"
#include "vx_cal.h"
#include "is_bad_data.h"
#include "num_array.h"

////////////////////////////////////////////////////////////////////////

   //
   // Enumeration of thresholding operations
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

   no_thresh_type = -1
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

extern bool is_inclusive(ThreshType);

////////////////////////////////////////////////////////////////////////

   //
   // Enumeration of percentile threshold types
   //

enum PercThreshType {
   perc_thresh_user_specified    = 0,
   perc_thresh_sample_fcst       = 1,
   perc_thresh_sample_obs        = 2,
   perc_thresh_sample_fcst_climo = 3,
   perc_thresh_sample_obs_climo  = 4,
   perc_thresh_fcst_climo_dist   = 5,
   perc_thresh_obs_climo_dist    = 6,
   perc_thresh_freq_bias         = 7,

   no_perc_thresh_type = -1
};

extern bool is_climo_dist_type(PercThreshType);

struct PercThreshInfo {
   const std::string short_name;
   const std::string long_name;
};

static const std::map<PercThreshType,PercThreshInfo> perc_thresh_info_map = {
   { perc_thresh_user_specified,    { "USP",   "USER_SPECIFIED_PERC"    } },
   { perc_thresh_sample_fcst,       { "SFP",   "SAMPLE_FCST_PERC"       } },
   { perc_thresh_sample_obs,        { "SOP",   "SAMPLE_OBS_PERC"        } },
   { perc_thresh_sample_fcst_climo, { "SFCP",  "SAMPLE_FCST_CLIMO_PERC" } },
   { perc_thresh_sample_obs_climo,  { "SOCP",  "SAMPLE_OBS_CLIMO_PERC"  } },
   { perc_thresh_fcst_climo_dist,   { "FCDP",  "CLIMO_FCST_DIST_PERC"   } },
   { perc_thresh_obs_climo_dist,    { "OCDP",  "CLIMO_OBS_DIST_PERC"    } },
   { perc_thresh_freq_bias,         { "FBIAS", "FREQ_BIAS_PERC"         } },
};

static const int    perc_thresh_default_precision = 0;
static const double perc_thresh_default_tol = 0.05;

struct PC_info {
   PercThreshType ptype;
   double value;
};

extern bool parse_perc_thresh(const char *str, PC_info *info = nullptr);

struct ClimoPntInfo {
   ClimoPntInfo() { clear(); }
   ClimoPntInfo(double a, double b, double c, double d) :
                fcmn(a), fcsd(b), ocmn(c), ocsd(d) {}
   void set(double a, double b, double c, double d) { fcmn = a; fcsd = b; ocmn = c; ocsd = d; }
   void clear() { fcmn = fcsd = ocmn = ocsd = bad_data_double; }

   double fcmn;
   double fcsd;
   double ocmn;
   double ocsd;
};

////////////////////////////////////////////////////////////////////////

class SingleThresh;
class Simple_Node;

////////////////////////////////////////////////////////////////////////

class ThreshNode {

   protected:

      void threshnode_assign(const ThreshNode *);

   public:

      ThreshNode();
      virtual ~ThreshNode();

      virtual bool check(double, const ClimoPntInfo *cpi = nullptr) const = 0;

      virtual ThreshNode * copy() const = 0;

      virtual ThreshType type() const = 0;

      virtual double value() const = 0;

      virtual PercThreshType ptype() const = 0;

      virtual double pvalue() const = 0;

      virtual double obs_climo_prob() const = 0;

      virtual bool need_perc() const = 0;

      virtual void set_perc(const NumArray *, const NumArray *,
                            const NumArray *, const NumArray *,
                            const SingleThresh *fthr = nullptr,
                            const SingleThresh *othr = nullptr) = 0;

      virtual void multiply_by(const double) = 0;

      virtual void get_simple_nodes(std::vector<Simple_Node> &) const = 0;

      ConcatString s;
      ConcatString abbr_s;

};


////////////////////////////////////////////////////////////////////////


class Or_Node : public ThreshNode {

   public:

      Or_Node();
     ~Or_Node();

      bool check(double, const ClimoPntInfo *cpi = nullptr) const override;

      ThreshNode * copy() const override;

      ThreshType type() const override;

      double value() const override;

      PercThreshType ptype() const override;

      double pvalue() const override;

      double obs_climo_prob() const override;

      bool need_perc() const override;

      void set_perc(const NumArray *, const NumArray *,
                    const NumArray *, const NumArray *,
                    const SingleThresh *fthr = nullptr,
                    const SingleThresh *othr = nullptr) override;

      void multiply_by(const double) override;

      void get_simple_nodes(std::vector<Simple_Node> &) const override;

      ThreshNode * left_child;
      ThreshNode * right_child;

};


////////////////////////////////////////////////////////////////////////


inline ThreshType     Or_Node::type()   const { return thresh_complex      ; }
inline double         Or_Node::value()  const { return bad_data_double     ; }
inline PercThreshType Or_Node::ptype()  const { return no_perc_thresh_type ; }
inline double         Or_Node::pvalue() const { return bad_data_double     ; }


////////////////////////////////////////////////////////////////////////


class And_Node : public ThreshNode {

   public:

      And_Node();
     ~And_Node();

      bool check(double, const ClimoPntInfo *cpi = nullptr) const override;

      ThreshType type() const override;

      double value() const override;

      PercThreshType ptype() const override;

      double pvalue() const override;

      double obs_climo_prob() const override;

      bool need_perc() const override;

      void set_perc(const NumArray *, const NumArray *,
                    const NumArray *, const NumArray *,
                    const SingleThresh *fthr = nullptr,
                    const SingleThresh *othr = nullptr) override;

      void multiply_by(const double) override;

      void get_simple_nodes(std::vector<Simple_Node> &) const override;

      ThreshNode * copy() const override;

      ThreshNode * left_child;
      ThreshNode * right_child;

};


////////////////////////////////////////////////////////////////////////


inline ThreshType     And_Node::type()   const { return thresh_complex      ; }
inline double         And_Node::value()  const { return bad_data_double     ; }
inline PercThreshType And_Node::ptype()  const { return no_perc_thresh_type ; }
inline double         And_Node::pvalue() const { return bad_data_double     ; }


////////////////////////////////////////////////////////////////////////


class Not_Node : public ThreshNode {

   public:

      Not_Node();
     ~Not_Node();

      bool check(double, const ClimoPntInfo *cpi = nullptr) const override;

      ThreshType type() const override;

      double value() const override;

      PercThreshType ptype() const override;

      double pvalue() const override;

      double obs_climo_prob() const override;

      bool need_perc() const override;

      void set_perc(const NumArray *, const NumArray *,
                    const NumArray *, const NumArray *,
                    const SingleThresh *fthr = nullptr,
                    const SingleThresh *othr = nullptr) override;

      void multiply_by(const double) override;

      void get_simple_nodes(std::vector<Simple_Node> &) const override;

      ThreshNode * copy() const override;

      ThreshNode * child;

};


////////////////////////////////////////////////////////////////////////


inline ThreshType     Not_Node::type()   const { return thresh_complex      ; }
inline double         Not_Node::value()  const { return bad_data_double     ; }
inline PercThreshType Not_Node::ptype()  const { return no_perc_thresh_type ; }
inline double         Not_Node::pvalue() const { return bad_data_double     ; }


////////////////////////////////////////////////////////////////////////


class Simple_Node : public ThreshNode {

   public:

      Simple_Node();
     ~Simple_Node();

         //

      ThreshType op;

      double T;

      PercThreshType Ptype;

      double PT;

         //
         //  set stuff
         //

      void set_na();

      void set_perc(const NumArray *, const NumArray *,
                    const NumArray *, const NumArray *,
                    const SingleThresh *fthr = nullptr,
                    const SingleThresh *othr = nullptr) override;

         //
         //  get stuff
         //

      ThreshType type() const override;

      double value() const override;

      PercThreshType ptype() const override;

      double pvalue() const override;

      double obs_climo_prob() const override;

      bool need_perc() const override;

      void get_simple_nodes(std::vector<Simple_Node> &) const override;

         //
         //  do stuff
         //

      ThreshNode * copy() const override;

      bool check(double, const ClimoPntInfo *cpi = nullptr) const override;

      void multiply_by(const double) override;

};


////////////////////////////////////////////////////////////////////////


inline ThreshType     Simple_Node::type()   const { return op    ; }
inline double         Simple_Node::value()  const { return T     ; }
inline PercThreshType Simple_Node::ptype()  const { return Ptype ; }
inline double         Simple_Node::pvalue() const { return PT    ; }


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

      void dump(std::ostream &, int = 0) const;

      bool operator==(const SingleThresh &) const;

      ThreshNode * node;   //  allocated

      void           clear();

      void           set(double  t, ThreshType);
      void           set(double pt, ThreshType, PercThreshType, double t = bad_data_double);
      void           set(const ThreshNode *);
      void           set(const char *);

      bool           need_perc() const;
      void           set_perc(const NumArray *, const NumArray *,
                              const NumArray *, const NumArray *,
                              const SingleThresh *fthr = nullptr,
                              const SingleThresh *othr = nullptr);

      void           set_na();

      ThreshType     get_type() const;
      double         get_value() const;
      PercThreshType get_ptype() const;
      double         get_pvalue() const;
      double         get_obs_climo_prob() const;
      void           get_simple_nodes(std::vector<Simple_Node> &) const;

      void           multiply_by(const double);

      ConcatString   get_str(int precision = thresh_default_precision) const;
      ConcatString   get_abbr_str(int precision = thresh_default_precision) const;

      bool           check(double, const ClimoPntInfo *cpi = nullptr) const; 

};


////////////////////////////////////////////////////////////////////////


inline ThreshType     SingleThresh::get_type()           const { return ( node ? node->type()           : thresh_na           ); }
inline double         SingleThresh::get_value()          const { return ( node ? node->value()          : bad_data_double     ); }
inline PercThreshType SingleThresh::get_ptype()          const { return ( node ? node->ptype()          : no_perc_thresh_type ); }
inline double         SingleThresh::get_pvalue()         const { return ( node ? node->pvalue()         : bad_data_double     ); }
inline double         SingleThresh::get_obs_climo_prob() const { return ( node ? node->obs_climo_prob() : bad_data_double     ); }

////////////////////////////////////////////////////////////////////////


#endif   //  __THRESHOLD_H__

////////////////////////////////////////////////////////////////////////
