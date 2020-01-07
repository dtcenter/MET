// *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*
// ** Copyright UCAR (c) 1992 - 2020
// ** University Corporation for Atmospheric Research (UCAR)
// ** National Center for Atmospheric Research (NCAR)
// ** Research Applications Lab (RAL)
// ** P.O.Box 3000, Boulder, Colorado, 80307-3000, USA
// *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*



////////////////////////////////////////////////////////////////////////


#ifndef  __PBM_H__
#define  __PBM_H__


////////////////////////////////////////////////////////////////////////


#include "pxm_base.h"


////////////////////////////////////////////////////////////////////////


class Pbm : public PxmBase {

   protected:

      void assign(const Pbm &);

      void init_from_scratch();


      int bytes_per_row() const;

      int total_data_bytes() const;

   public:

      Pbm();
      Pbm(const char *);
      virtual ~Pbm();
      Pbm(const Pbm &);
      Pbm & operator=(const Pbm &);


         //
         //  from base class
         //

      int read(const char *);

      int write(const char *) const;

      void clear();

      int n_data_bytes() const;


      void set_size_rc(int NR, int NC);
      void set_size_xy(int NX, int NY);

      Color getrc(int, int) const;
      Color getxy(int, int) const;

      void putrc(const Color &, int, int);
      void putxy(const Color &, int, int);


      void rotate(int);

      void autocrop();

      void gamma(double);

      void reverse_video();

      void all_black();

      void all_white();

      void dump(ostream &, int depth = 0) const;

};


////////////////////////////////////////////////////////////////////////


inline int Pbm::bytes_per_row() const { return ( (Ncols + 7)/8 ); }

inline int Pbm::total_data_bytes() const { return ( Nrows*bytes_per_row() ); }

inline int Pbm::n_data_bytes() const { return ( Nalloc ); }


////////////////////////////////////////////////////////////////////////


#endif   //  __PBM_H__


////////////////////////////////////////////////////////////////////////


