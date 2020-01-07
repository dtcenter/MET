// *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*
// ** Copyright UCAR (c) 1992 - 2020
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


class TTContingencyTable;   //  forward reference


////////////////////////////////////////////////////////////////////////


   //
   //  general contingency table
   //


class ContingencyTable {

   protected:

      void init_from_scratch();

      void assign(const ContingencyTable &);

      int rc_to_n(int r, int c) const;

      vector<int> *E;   //  this is really a two-dimensional array

      int Nrows;
      int Ncols;

      ConcatString Name;

   public:

      ContingencyTable();
      virtual ~ContingencyTable();
      ContingencyTable(const ContingencyTable &);
      ContingencyTable & operator=(const ContingencyTable &);
      ContingencyTable & operator+=(const ContingencyTable &);

      void clear();

      void zero_out();

      virtual void dump(ostream & out, int depth = 0) const;

         //
         //  condition on an event
         //

      TTContingencyTable condition_on(int) const;

         //
         //  set attributes
         //

      virtual void set_size(int);
      virtual void set_size(int NR, int NC);

      void set_name(const char *);

         //
         //  get attributes
         //

      int nrows() const;
      int ncols() const;

      ConcatString name() const;

         //
         //  set counts
         //

      void set_entry(int row, int col, int value);

         //
         //  increment counts
         //

      void inc_entry(int row, int col);

         //
         //  get counts
         //

      int total() const;

      int row_total(int row) const;
      int col_total(int col) const;

      int entry(int row, int col) const;

      int  largest_entry() const;
      int smallest_entry() const;

         //
         //  statistics
         //

      virtual double gaccuracy () const;
      virtual double gheidke   () const;
      virtual double gkuiper   () const;
      virtual double gerrity   () const;

};


////////////////////////////////////////////////////////////////////////


inline int ContingencyTable::nrows() const { return ( Nrows ); }
inline int ContingencyTable::ncols() const { return ( Ncols ); }

inline ConcatString ContingencyTable::name() const { return ( Name ); }


////////////////////////////////////////////////////////////////////////


static const int nx2_event_column     = 0;
static const int nx2_nonevent_column  = 1;


////////////////////////////////////////////////////////////////////////


   //
   //  N x 2 contingency table
   //


class Nx2ContingencyTable : public ContingencyTable {

   private:

      double * Thresholds;   //  N + 1 count, increasing

      int value_to_row(double) const;

      void init_from_scratch();

      void assign(const Nx2ContingencyTable &);

   public:

      Nx2ContingencyTable();
      virtual ~Nx2ContingencyTable();
      Nx2ContingencyTable(const Nx2ContingencyTable &);
      Nx2ContingencyTable & operator=(const Nx2ContingencyTable &);

      void clear();

      void set_size(int NR);
      void set_size(int NR, int NC);   //  NC had better be 2

      void set_thresholds(const double *);

         //
         //  get thresholds
         //

      double threshold(int index) const;   //  0 <= index <= Nrows

         //
         //  increment counts
         //

      void inc_event    (double);
      void inc_nonevent (double);

         //
         //  get counts
         //

      int    event_count_by_thresh(double) const;
      int nonevent_count_by_thresh(double) const;

      int    event_count_by_row(int row) const;
      int nonevent_count_by_row(int row) const;

      int n() const;

         //
         //  column totals
         //

      int    event_col_total() const;
      int nonevent_col_total() const;

         //
         //  statistics
         //

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


inline int Nx2ContingencyTable::event_col_total    () const { return ( col_total(nx2_event_column) ); }
inline int Nx2ContingencyTable::nonevent_col_total () const { return ( col_total(nx2_nonevent_column) ); }

inline int Nx2ContingencyTable::event_count_by_row    (int row) const { return ( entry(row, nx2_event_column) ); }
inline int Nx2ContingencyTable::nonevent_count_by_row (int row) const { return ( entry(row, nx2_nonevent_column) ); }


////////////////////////////////////////////////////////////////////////


   //
   //  2 x 2 contingency table
   //


class TTContingencyTable : public ContingencyTable {

   public:

      TTContingencyTable();
      virtual ~TTContingencyTable();
      TTContingencyTable(const TTContingencyTable &);
      TTContingencyTable & operator=(const TTContingencyTable &);

      void set_size(int);
      void set_size(int NR, int NC);

         //
         //  set counts
         //

      void set_fn_on(int);
      void set_fy_on(int);

      void set_fn_oy(int);
      void set_fy_oy(int);

         //
         //  increment counts
         //

      void inc_fn_on();
      void inc_fy_on();

      void inc_fn_oy();
      void inc_fy_oy();

         //
         //  get counts
         //

      int fn_on() const;
      int fy_on() const;

      int fn_oy() const;
      int fy_oy() const;

      int on() const;
      int oy() const;

      int fn() const;
      int fy() const;

      int n() const;

         //
         //  FHO rates where:
         //  f_rate = FY/N
         //  h_rate = fy_oy/N
         //  o_rate = OY/N
         //

      double f_rate    () const;
      double h_rate    () const;
      double o_rate    () const;

         //
         //  Raw counts as proportions of the
         //  total count.
         //

      double fy_oy_tp  () const;
      double fy_on_tp  () const;
      double fn_oy_tp  () const;
      double fn_on_tp  () const;

      double fy_tp     () const;
      double fn_tp     () const;

      double oy_tp     () const;
      double on_tp     () const;

         //
         //  Raw counts as proportions of the
         //  total forecast yes count.
         //

      double fy_oy_fp () const;
      double fy_on_fp () const;
      double fn_oy_fp () const;
      double fn_on_fp () const;

         //
         //  Raw counts as proportions of the
         //  total observation yes count.
         //

      double fy_oy_op () const;
      double fy_on_op () const;
      double fn_oy_op () const;
      double fn_on_op () const;

         //
         //  Contingency Table Statistics with confidence intervals
         //  when applicable.
         //

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
   //  this is the layout on page 239 of
   //
   //   "Statistical Methods in the Atmospheric Sciences" (1st ed)
   //
   //     by Daniel S. Wilks
   //


static const int OY_col = 0;
static const int ON_col = 1;

static const int FY_row = 0;
static const int FN_row = 1;


////////////////////////////////////////////////////////////////////////


extern void calc_gerrity_scoring_matrix(int N, const double * p, double * s);


////////////////////////////////////////////////////////////////////////


#endif   //  __CONTINGENCY_TABLE_H__


////////////////////////////////////////////////////////////////////////


