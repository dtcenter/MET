// *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*
// ** Copyright UCAR (c) 1992 - 2020
// ** University Corporation for Atmospheric Research (UCAR)
// ** National Center for Atmospheric Research (NCAR)
// ** Research Applications Lab (RAL)
// ** P.O.Box 3000, Boulder, Colorado, 80307-3000, USA
// *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*



////////////////////////////////////////////////////////////////////////


#ifndef  __PXM_BASE_H__
#define  __PXM_BASE_H__


////////////////////////////////////////////////////////////////////////


#include <iostream>

#include "vx_color.h"


////////////////////////////////////////////////////////////////////////


static const int max_comments       =  16;

static const int max_comment_length = 256;


////////////////////////////////////////////////////////////////////////


class PxmBase {

   protected:

      unsigned char * data;

      int Nalloc;

      char * Name;

      int Nrows;
      int Ncols;

      char * Comment [max_comments];

      int Ncomments;

      int rc_to_n(int r, int c) const;

      void n_to_rc(int n, int & r, int & c) const;

      virtual void copy_common(const PxmBase &);

      virtual void clear_common();

      virtual void init_from_scratch();

   public:

      PxmBase();
      virtual ~PxmBase();

         //
         //  abstract member functions
         //

      virtual  int    read  (const char *) = 0;

      virtual  int    write (const char *) const = 0;


      virtual  void   clear () = 0;

      virtual  int    n_data_bytes () const = 0;

      virtual  void   set_size_rc (int NR, int NC) = 0;
      virtual  void   set_size_xy (int NX, int NY) = 0;

      virtual  Color  getrc (int row, int col) const = 0;
      virtual  Color  getxy (int   x, int   y) const = 0;

      virtual  void   putrc (const Color &, int row, int col) = 0;
      virtual  void   putxy (const Color &, int   x, int   y) = 0;


      virtual  void   rotate (int) = 0;

      virtual  void   autocrop () = 0;

      virtual  void   gamma (double) = 0;

      virtual  void   reverse_video () = 0;

      virtual  void   all_black() = 0;

      virtual  void   all_white() = 0;

      virtual  void   dump (ostream &, int depth = 0) const = 0;

         //
         //  non-abstract member functions
         //

      virtual const char * name () const;

      virtual const char * short_name () const;

      virtual  int nrows () const;
      virtual  int ncols () const;

      virtual  int nx () const;
      virtual  int ny () const;

      virtual int ok () const;


      virtual const char * comment (int) const;

      virtual int n_comments () const;

      virtual void add_comment (const char *);

      virtual void clear_comments ();

      virtual void copy_data    (unsigned char * out) const;
      virtual void copy_data_32 (unsigned char * out, const bool swap_endian = false) const;

};


////////////////////////////////////////////////////////////////////////


inline const char * PxmBase::name() const { return ( Name ); }

inline int PxmBase::nrows() const { return ( Nrows ); }
inline int PxmBase::ncols() const { return ( Ncols ); }

inline int PxmBase::ny() const { return ( Nrows ); }
inline int PxmBase::nx() const { return ( Ncols ); }

inline int PxmBase::n_comments() const { return ( Ncomments ); }

inline int PxmBase::ok() const { return ( data ? 1 : 0 ); }


////////////////////////////////////////////////////////////////////////


#endif   //  __PXM_BASE_H__


////////////////////////////////////////////////////////////////////////


