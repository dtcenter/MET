// *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*
// ** Copyright UCAR (c) 1992 - 2020
// ** University Corporation for Atmospheric Research (UCAR)
// ** National Center for Atmospheric Research (NCAR)
// ** Research Applications Lab (RAL)
// ** P.O.Box 3000, Boulder, Colorado, 80307-3000, USA
// *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*



////////////////////////////////////////////////////////////////////////


#ifndef  __PCM_H__
#define  __PCM_H__


////////////////////////////////////////////////////////////////////////


#include "pxm_base.h"


////////////////////////////////////////////////////////////////////////


class Pcm : public PxmBase {

   private:

      Color * Colormap;

      int Ncolors;

      void assign(const Pcm &);

      void init_from_scratch();

   public:

      Pcm();
      Pcm(const char *);
      virtual ~Pcm();
      Pcm(const Pcm &);
      Pcm & operator=(const Pcm &);

         //
         //  from base class
         //

      int  read(const char *);

      int write(const char *) const;

      void clear();

      int n_data_bytes() const;

      void set_size_rc(int NR, int NC);
      void set_size_xy(int NX, int NY);

      Color getrc(int row, int col) const;
      Color getxy(int   x, int   y) const;

      unsigned char data_getrc(int row, int col) const;
      unsigned char data_getxy(int   x, int   y) const;

      void putrc(const Color &, int row, int col);
      void putxy(const Color &, int   x, int   y);


      void rotate(int);

      void autocrop();

      void gamma(double);

      void reverse_video();

      void dump(ostream &, int depth = 0) const;

      void all_black();   //  does nothing
      void all_white();   //  does nothing

         //
         //  not from base class
         //

      int colormap_index(const Color &) const;

      Color colormap(int) const;

      int n_colors() const;

      void set_colormap(const Color *, int n);

      void data_putrc(unsigned char, int row, int col);
      void data_putxy(unsigned char, int   x, int   y);

};


////////////////////////////////////////////////////////////////////////


inline int Pcm::n_colors() const { return ( Ncolors ); }

inline int Pcm::n_data_bytes() const { return ( Nalloc ); }

////////////////////////////////////////////////////////////////////////


#endif   //  __PCM_H__


////////////////////////////////////////////////////////////////////////


