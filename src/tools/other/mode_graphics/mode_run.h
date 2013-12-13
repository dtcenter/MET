

////////////////////////////////////////////////////////////////////////


#ifndef  __MODE_RUN_H__
#define  __MODE_RUN_H__


////////////////////////////////////////////////////////////////////////


#include <iostream>

#include "vx_cal.h"

#include "met_int_array.h"
#include "mode_line_array.h"
#include "fo_pair_array.h"


////////////////////////////////////////////////////////////////////////


class ModeRun {

      friend int operator>>(istream &, ModeRun &);

   private:

      void init_from_scratch();

      void assign(const ModeRun &);

      int two_to_one(int fnum, int onum, int Nf, int No) const;


      ModeLineArray a;

      unixtime FcstValid;
      unixtime ObsValid;

      int FcstLead;    //  seconds
      int ObsLead;     //  seconds

      int FcstAccum;   //  seconds
      int ObsAccum;    //  seconds

      ConcatString Model;

      ConcatString Filename;

      int LineFirst;   //  line numbers start at one
      int LineLast;    //  line numbers start at one

      int          FcstRadius;
      ConcatString FcstThresh;
      ConcatString FcstVariable;
      ConcatString FcstLevel;

      int          ObsRadius;
      ConcatString ObsThresh;
      ConcatString ObsVariable;
      ConcatString ObsLevel;

      int N_FcstSimpleObjs;
      int N_ObsSimpleObjs;

      int N_FcstCompositeObjs;
      int N_ObsCompositeObjs;

      int N_SimplePairs;
      int N_CompositePairs;

      MetIntArray fcst_simple_single_indices;
      MetIntArray  obs_simple_single_indices;

      MetIntArray fcst_composite_single_indices;
      MetIntArray  obs_composite_single_indices;

      FO_PairArray    simple_pair_indices;
      FO_PairArray composite_pair_indices;

   public:

      ModeRun();
     ~ModeRun();
      ModeRun(const ModeRun &);
      ModeRun & operator=(const ModeRun &);


      void clear();

      void dump(ostream &, int = 0) const;

         //
         //  set stuff
         //

      void set_filename(const char *);

         //
         //  get stuff
         //

      unixtime fcst_valid() const;
      unixtime  obs_valid() const;

      int fcst_lead() const;    //  seconds
      int  obs_lead() const;    //  seconds

      int fcst_accum() const;   //  seconds
      int  obs_accum() const;   //  seconds

      int fcst_radius() const;
      int  obs_radius() const;

      ConcatString fcst_thresh() const;
      ConcatString  obs_thresh() const;

      ConcatString fcst_variable() const;
      ConcatString  obs_variable() const;

      ConcatString fcst_level() const;
      ConcatString  obs_level() const;

      ConcatString filename() const;
      ConcatString short_filename() const;

      ConcatString model() const;

      int line_number_first() const;
      int line_number_last () const;

      int n_lines() const;

      int n_fcst_simple_objs    () const;
      int n_obs_simple_objs     () const;

      int n_fcst_composite_objs () const;
      int n_obs_composite_objs  () const;

      int n_simple_pairs        () const;
      int n_composite_pairs     () const;

      ModeLine get_fcst_simple_single    (int) const;
      ModeLine get_obs_simple_single     (int) const;

      ModeLine get_fcst_composite_single (int) const;
      ModeLine get_obs_composite_single  (int) const;

      ModeLine get_simple_pair      (int fnum, int onum) const;

      ModeLine get_composite_pair   (int fnum, int onum) const;

      MetIntArray fcst_composite_obj_numbers(int fnum) const;
      MetIntArray  obs_composite_obj_numbers(int onum) const;

      ModeLineArray get_fcst_composite(int fnum) const;
      ModeLineArray  get_obs_composite(int onum) const;

      FO_Pair    simple_pair (int n) const;
      FO_Pair composite_pair (int n) const;

      bool has_simple_pair    (int fnum, int onum) const;
      bool has_composite_pair (int fnum, int onum) const;

      bool has_simple_pair    (int fnum, int onum, int & index) const;
      bool has_composite_pair (int fnum, int onum, int & index) const;

      ModeLine operator[](int) const;   //  returns lines from "a"

         //
         //  do stuff
         //

      void add(const ModeLine &);

      void patch_up();   //  do bookkeeping, patch up indices, etc.

      bool is_same_run(const ModeLine &) const;

      void calc_mmi (double & fcst_mmi, double & obs_mmi, double & fo_mmi) const;

};


////////////////////////////////////////////////////////////////////////


inline unixtime ModeRun::fcst_valid () const { return ( FcstValid ); }
inline unixtime ModeRun::obs_valid  () const { return (  ObsValid ); }

inline int ModeRun::fcst_lead () const { return ( FcstLead ); }
inline int ModeRun::obs_lead  () const { return (  ObsLead ); }

inline int ModeRun::fcst_radius() const { return ( FcstRadius ); }
inline int  ModeRun::obs_radius() const { return (  ObsRadius ); }

inline ConcatString ModeRun::fcst_thresh() const { return ( FcstThresh ); }
inline ConcatString  ModeRun::obs_thresh() const { return (  ObsThresh ); }

inline int ModeRun::fcst_accum () const { return ( FcstAccum ); }
inline int ModeRun::obs_accum  () const { return (  ObsAccum ); }

inline ConcatString ModeRun::fcst_variable () const { return ( FcstVariable ); }
inline ConcatString ModeRun::obs_variable  () const { return (  ObsVariable ); }

inline ConcatString ModeRun::fcst_level () const { return ( FcstLevel ); }
inline ConcatString ModeRun::obs_level  () const { return (  ObsLevel ); }

inline ConcatString ModeRun::filename () const { return ( Filename ); }

inline ConcatString ModeRun::model () const { return ( Model ); }

inline int ModeRun::line_number_first () const { return ( LineFirst ); }
inline int ModeRun::line_number_last  () const { return ( LineLast  ); }

inline int ModeRun::n_lines () const { return ( a.n_elements() ); }

inline int ModeRun::n_fcst_simple_objs    () const { return ( N_FcstSimpleObjs ); }
inline int ModeRun::n_obs_simple_objs     () const { return ( N_ObsSimpleObjs  ); }

inline int ModeRun::n_fcst_composite_objs () const { return ( N_FcstCompositeObjs ); }
inline int ModeRun::n_obs_composite_objs  () const { return ( N_ObsCompositeObjs  ); }

inline int ModeRun::n_simple_pairs () const { return ( N_SimplePairs ); }
inline int ModeRun::n_composite_pairs () const { return ( N_CompositePairs ); }

inline void ModeRun::set_filename(const char * path) { Filename = path;  return; }

inline ModeLine ModeRun::operator[](int n) const { return ( a[n] ); }


////////////////////////////////////////////////////////////////////////


// extern int operator>>(istream &, ModeRun &);  //  this is now a member of the 
                                                 //  ModeOutputFile class


////////////////////////////////////////////////////////////////////////


#endif   /*  __MODE_RUN_H__  */


////////////////////////////////////////////////////////////////////////


