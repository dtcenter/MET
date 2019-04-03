// *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*
// ** Copyright UCAR (c) 1992 - 2019
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


static const int fcst_obs_set_alloc_inc  = 50;


///////////////////////////////////////////////////////////////////////////////


class FcstObsSet {

   protected:

      void init_from_scratch();

      void assign(const FcstObsSet &);

      void extend(int * &, int & n_alloc, const int n_new);

   public:

         //
         //  data
         //

      int * fcst_number;   //  allocated
      int * obs_number;    //  allocated

      int n_fcst;
      int n_obs;

      int n_fcst_alloc;
      int n_obs_alloc;

         //
         //  functions
         //

      void     clear();
      void all_clear();

      void extend_fcst (int);
      void extend_obs  (int);

      FcstObsSet();
     ~FcstObsSet();
      FcstObsSet(const FcstObsSet &);
      FcstObsSet & operator=(const FcstObsSet &);

      void add_pair(int fcst, int obs);

      int has_fcst(int) const;
      int has_obs(int) const;

      void add_fcst(int);
      void add_obs(int);


};

///////////////////////////////////////////////////////////////////////////////


extern FcstObsSet union_fcst_obs_sets(const FcstObsSet &, const FcstObsSet &);

extern int fcst_obs_sets_overlap(const FcstObsSet &, const FcstObsSet &);

extern ostream & operator<<(ostream &, const FcstObsSet &);


///////////////////////////////////////////////////////////////////////////////


// static const int max_fcst_obs_sets = 300;

static const int set_alloc_inc = 50;


///////////////////////////////////////////////////////////////////////////////


class SetCollection {

      void init_from_scratch();

      void assign(const SetCollection &);

      void extend(int);

   public:

         //
         //  data
         //

      FcstObsSet * set;   //  allocated

      int n_sets;

      int n_alloc;


         //
         //  functions
         //

      SetCollection();
     ~SetCollection();
      SetCollection(const SetCollection &);
      SetCollection & operator=(const SetCollection &);

      void     clear();
      void all_clear();

      int merge();

      int fcst_set_number (int fcst_number) const;
      int  obs_set_number (int obs_number)  const;

      int is_fcst_matched (int fcst_number) const;
      int  is_obs_matched (int obs_number)  const;

      void merge_two(int, int);

      void add_pair(int fsct, int obs);

      void clear_empty_sets();


      void make_room(const int = 1);

};


///////////////////////////////////////////////////////////////////////////////


extern ostream & operator<<(ostream &, const SetCollection &);


inline void SetCollection::make_room(const int __n)  { if ( __n > 0 )  extend(n_sets + __n);  return; }


///////////////////////////////////////////////////////////////////////////////

#endif   //  __VX_WRFMODE_SET_H__

///////////////////////////////////////////////////////////////////////////////
