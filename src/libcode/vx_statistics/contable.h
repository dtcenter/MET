// *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*
// ** Copyright UCAR (c) 1992 - 2024
// ** University Corporation for Atmospheric Research (UCAR)
// ** National Center for Atmospheric Research (NCAR)
// ** Research Applications Lab (RAL)
// ** P.O.Box 3000, Boulder, Colorado, 80307-3000, USA
// *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*

////////////////////////////////////////////////////////////////////////

#ifndef  __CONTINGENCY_TABLE_H__
#define  __CONTINGENCY_TABLE_H__

////////////////////////////////////////////////////////////////////////

#include <iostream>

#include "vx_util.h"
#include "vx_math.h"

////////////////////////////////////////////////////////////////////////

// Forward reference
class TTContingencyTable;

////////////////////////////////////////////////////////////////////////
//
// General contingency table
//
////////////////////////////////////////////////////////////////////////

class ContingencyTable {

   protected:

      void init_from_scratch();

      void assign(const ContingencyTable &);

      int rc_to_n(int r, int c) const;

      // This is really a two-dimensional array (Nrows, Ncols)
      std::vector<double> E;

      int Nrows;
      int Ncols;

      int Npairs;
      double ECvalue;

      ConcatString Name;

   public:

      ContingencyTable();
      virtual ~ContingencyTable();
      ContingencyTable(const ContingencyTable &);
      ContingencyTable & operator=(const ContingencyTable &);
      ContingencyTable & operator+=(const ContingencyTable &);

      void clear();

      void zero_out();

      virtual void dump(std::ostream & out, int depth = 0) const;

      // Set attributes
      virtual void set_size(int);
      virtual void set_size(int NR, int NC);

      void set_n_pairs(int);
      void set_ec_value(double);
      void set_name(const char *);

      // Get attributes
      int nrows() const;
      int ncols() const;

      int n_pairs() const;
      double ec_value() const;
      ConcatString name() const;

      // Set table entries
      void set_entry(int row, int col, double value);

      // Increment table entries
      void inc_entry(int row, int col, double weight=1.0);

      // Get values
      double total() const;

      double row_total(int row) const;
      double col_total(int col) const;

      double entry(int row, int col) const;

      double max() const;
      double min() const;

      bool is_integer() const;

      // Statistics
      virtual double gaccuracy () const;
      virtual double gheidke   () const;
      virtual double gheidke_ec(double) const;
      virtual double gkuiper   () const;
      virtual double gerrity   () const;
};


////////////////////////////////////////////////////////////////////////

inline int ContingencyTable::nrows() const { return Nrows; }
inline int ContingencyTable::ncols() const { return Ncols; }

inline int          ContingencyTable::n_pairs()  const { return Npairs;  }
inline double       ContingencyTable::ec_value() const { return ECvalue; }
inline ConcatString ContingencyTable::name()     const { return Name;    }

////////////////////////////////////////////////////////////////////////

static const int nx2_event_column     = 0;
static const int nx2_nonevent_column  = 1;

////////////////////////////////////////////////////////////////////////
//
// N x 2 contingency table
//
////////////////////////////////////////////////////////////////////////

class Nx2ContingencyTable : public ContingencyTable {

   private:

      // N + 1 count, parametrically increasing or decreasing
      std::vector<double> Thresholds;

      int value_to_row(double) const;

      void init_from_scratch();

      void assign(const Nx2ContingencyTable &);

   public:

      Nx2ContingencyTable();
      virtual ~Nx2ContingencyTable();
      Nx2ContingencyTable(const Nx2ContingencyTable &);
      Nx2ContingencyTable & operator=(const Nx2ContingencyTable &);

      void clear();

      void set_size(int NR) override;
      void set_size(int NR, int NC) override; // NC must be 2

      void set_thresholds(const std::vector<double> &);

      // Get thresholds
      double threshold(int index) const; // 0 <= index <= Nrows

      // Increment table entries
      void inc_event    (double value, double weight=1.0);
      void inc_nonevent (double value, double weight=1.0);

      // Get table entries
      double    event_total_by_thresh(double) const;
      double nonevent_total_by_thresh(double) const;

      double    event_total_by_row(int row) const;
      double nonevent_total_by_row(int row) const;

      // Set counts
      void set_event(int row, double);
      void set_nonevent(int row, double);

      // Column totals
      double    event_col_total() const;
      double nonevent_col_total() const;

      // Statistics
      double baser        () const;
      double baser_ci     (double alpha, double &cl, double &cu) const;
      double brier_score  () const;
      double brier_ci_halfwidth(double alpha) const;

      double reliability  () const;
      double resolution   () const;
      double uncertainty  () const;
      double bss_smpl     () const;

      double row_obar  (int row) const;
      double     obar  ()        const;
      double row_proby (int row) const;

      double row_calibration (int row) const;
      double row_refinement  (int row) const;

      double row_event_likelihood    (int row) const;
      double row_nonevent_likelihood (int row) const;

      TTContingencyTable ctc_by_row  (int row) const;
      double             roc_auc() const;
};

////////////////////////////////////////////////////////////////////////

inline double Nx2ContingencyTable::event_total_by_row    (int row) const { return entry(row, nx2_event_column); }
inline double Nx2ContingencyTable::nonevent_total_by_row (int row) const { return entry(row, nx2_nonevent_column); }

inline double Nx2ContingencyTable::event_col_total    () const { return col_total(nx2_event_column); }
inline double Nx2ContingencyTable::nonevent_col_total () const { return col_total(nx2_nonevent_column); }

////////////////////////////////////////////////////////////////////////
//
// 2 x 2 contingency table
//
////////////////////////////////////////////////////////////////////////

class TTContingencyTable : public ContingencyTable {

   public:

      TTContingencyTable();
      virtual ~TTContingencyTable();
      TTContingencyTable(const TTContingencyTable &);
      TTContingencyTable & operator=(const TTContingencyTable &);

      void set_size(int) override;
      void set_size(int NR, int NC) override;

      // Set table entries
      void set_fn_on(double);
      void set_fy_on(double);

      void set_fn_oy(double);
      void set_fy_oy(double);

      // Increment table entries
      void inc_fn_on(double weight=1.0);
      void inc_fy_on(double weight=1.0);

      void inc_fn_oy(double weight=1.0);
      void inc_fy_oy(double weight=1.0);

      //  Get table entries
      double fn_on() const;
      double fy_on() const;

      double fn_oy() const;
      double fy_oy() const;

      double on() const;
      double oy() const;

      double fn() const;
      double fy() const;

      //  FHO rates where:
      //     f_rate = FY/N
      //     h_rate = fy_oy/N
      //     o_rate = OY/N
      double f_rate() const;
      double h_rate() const;
      double o_rate() const;

      // Proportions of the total
      double fy_oy_tp  () const;
      double fy_on_tp  () const;
      double fn_oy_tp  () const;
      double fn_on_tp  () const;

      double fy_tp     () const;
      double fn_tp     () const;

      double oy_tp     () const;
      double on_tp     () const;

      // Proportions of forecast
      double fy_oy_fp () const;
      double fy_on_fp () const;
      double fn_oy_fp () const;
      double fn_on_fp () const;

      // Proportions of observation
      double fy_oy_op () const;
      double fy_on_op () const;
      double fn_oy_op () const;
      double fn_on_op () const;

      // Contingency Table Statistics and confidence intervals
      double baser      () const;
      double baser_ci   (double alpha, double &cl, double &cu) const;
      double fmean      () const;
      double fmean_ci   (double alpha, double &cl, double &cu) const;
      double accuracy   () const;
      double accuracy_ci(double alpha, double &cl, double &cu) const;
      double fbias      () const;
      double pod_yes    () const;
      double pod_yes_ci (double alpha, double &cl, double &cu) const;
      double pod_no     () const;
      double pod_no_ci  (double alpha, double &cl, double &cu) const;
      double pofd       () const;
      double pofd_ci    (double alpha, double &cl, double &cu) const;
      double far        () const;
      double far_ci     (double alpha, double &cl, double &cu) const;
      double csi        () const;
      double csi_ci     (double alpha, double &cl, double &cu) const;
      double gss        () const;
      double bagss      () const;
      double hk         () const;
      double hk_ci      (double alpha, double &cl, double &cu) const;
      double hss        () const;
      double odds       () const;
      double odds_ci    (double alpha, double &cl, double &cu) const;
      double lodds      () const;
      double slor2      () const;
      double lodds_ci   (double alpha, double &cl, double &cu) const;
      double orss       () const;
      double orss_ci    (double alpha, double &cl, double &cu) const;
      double eds        () const;
      double eds_ci     (double alpha, double &cl, double &cu) const;
      double seds       () const;
      double seds_ci    (double alpha, double &cl, double &cu) const;
      double edi        () const;
      double edi_ci     (double alpha, double &cl, double &cu) const;
      double sedi       () const;
      double sedi_ci    (double alpha, double &cl, double &cu) const;
      double cost_loss  (double) const;
};

////////////////////////////////////////////////////////////////////////

extern TTContingencyTable finley();

extern TTContingencyTable finley_always_no();

////////////////////////////////////////////////////////////////////////
//
//  Reference page 239 of
//     "Statistical Methods in the Atmospheric Sciences" (1st ed)
//     by Daniel S. Wilks
//
////////////////////////////////////////////////////////////////////////

static const int OY_col = 0;
static const int ON_col = 1;

static const int FY_row = 0;
static const int FN_row = 1;

////////////////////////////////////////////////////////////////////////

extern void calc_gerrity_scoring_matrix(int N, const std::vector<double> &p,
                                                     std::vector<double> &s);

extern double compute_proportion(double, double);

////////////////////////////////////////////////////////////////////////

#endif   //  __CONTINGENCY_TABLE_H__

////////////////////////////////////////////////////////////////////////
