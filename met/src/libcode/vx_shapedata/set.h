// *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*
// ** Copyright UCAR (c) 1992 - 2018
// ** University Corporation for Atmospheric Research (UCAR)
// ** National Center for Atmospheric Research (NCAR)
// ** Research Applications Lab (RAL)
// ** P.O.Box 3000, Boulder, Colorado, 80307-3000, USA
// *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*

///////////////////////////////////////////////////////////////////////////////
//
//   Filename:   set.h
//
//   Description:
//
//
//   Mod#   Date      Name           Description
//   ----   ----      ----           -----------
//   000    04-15-05  Halley Gotway
//
///////////////////////////////////////////////////////////////////////////////

#ifndef  __VX_WRFMODE_SET_H__
#define  __VX_WRFMODE_SET_H__

///////////////////////////////////////////////////////////////////////////////

static const int max_fcst_numbers = 100;
static const int max_obs_numbers  = 100;

///////////////////////////////////////////////////////////////////////////////

class FcstObsSet {

   public:

      int fcst_number[max_fcst_numbers];
      int obs_number[max_obs_numbers];

      int n_fcst;
      int n_obs;

      FcstObsSet();
     ~FcstObsSet();
      FcstObsSet(const FcstObsSet &);
      FcstObsSet & operator=(const FcstObsSet &);

      void add_pair(int fcst, int obs);

      int has_fcst(int) const;
      int has_obs(int) const;

      void add_fcst(int);
      void add_obs(int);

      void clear();
};

///////////////////////////////////////////////////////////////////////////////

extern FcstObsSet union_fcst_obs_sets(const FcstObsSet &, const FcstObsSet &);

extern int fcst_obs_sets_overlap(const FcstObsSet &, const FcstObsSet &);

extern ostream & operator<<(ostream &, const FcstObsSet &);

///////////////////////////////////////////////////////////////////////////////

static const int max_fcst_obs_sets = 300;

///////////////////////////////////////////////////////////////////////////////

class SetCollection {

   public:

      FcstObsSet set[max_fcst_obs_sets];

      int n_sets;

      SetCollection();
     ~SetCollection();
      SetCollection(const SetCollection &);
      SetCollection & operator=(const SetCollection &);

      int merge();

      int fcst_set_number(int fcst_number) const;
      int obs_set_number(int obs_number) const;

      int is_fcst_matched(int fcst_number) const;
      int is_obs_matched(int obs_number) const;

      void merge_two(int, int);

      void add_pair(int fsct, int obs);

      void clear_empty_sets();

      void clear();
};

///////////////////////////////////////////////////////////////////////////////

extern ostream & operator<<(ostream &, const SetCollection &);

///////////////////////////////////////////////////////////////////////////////

#endif   //  __VX_WRFMODE_SET_H__

///////////////////////////////////////////////////////////////////////////////
