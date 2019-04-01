// *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*
// ** Copyright UCAR (c) 1992 - 2019
// ** University Corporation for Atmospheric Research (UCAR)
// ** National Center for Atmospheric Research (NCAR)
// ** Research Applications Lab (RAL)
// ** P.O.Box 3000, Boulder, Colorado, 80307-3000, USA
// *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*



////////////////////////////////////////////////////////////////////////


#ifndef  __GSI_CONV_RECORD_H__
#define  __GSI_CONV_RECORD_H__


////////////////////////////////////////////////////////////////////////


#include "gsi_record.h"
#include "vx_cal.h"


////////////////////////////////////////////////////////////////////////


class ConvRecord;   //  forward reference


////////////////////////////////////////////////////////////////////////


class ConvFile {

   friend bool operator>>(ConvFile &, ConvRecord &);

   private:

      void init_from_scratch();

      ConcatString Filename;

      int Fd;   //  the file descriptor

      unixtime Date;

      bool SwapEndian;

      int RecPadSize;

      int Nrec;
      int Npair;

   public:

      ConvFile();
     ~ConvFile();

      bool open(const char * path, bool _swap_endian = true, int _rec_pad_size = 4);

      void close();

         //
         //  set stuff
         //

         //
         //  get stuff
         //

      bool get_swap_endian() const;

      int get_rec_pad_size() const;

      unixtime date() const;

      int n_rec() const;
      int n_pair() const;

         //
         //  do stuff
         //


};


////////////////////////////////////////////////////////////////////////


inline bool ConvFile::get_swap_endian() const { return ( SwapEndian ); }

inline int ConvFile::get_rec_pad_size() const { return ( RecPadSize ); }

inline unixtime ConvFile::date() const { return ( Date ); }

inline int ConvFile::n_rec() const { return ( Nrec ); }

inline int ConvFile::n_pair() const { return ( Npair ); }


////////////////////////////////////////////////////////////////////////


extern bool operator>>(ConvFile &, ConvRecord &);


////////////////////////////////////////////////////////////////////////


class ConvRecord : public GsiRecord {

   friend bool operator>>(ConvFile &, ConvRecord &);

   protected:

      void conv_init_from_scratch();

      // void conv_assign(const GsiRecord &);

   public:

      ConvRecord();
     ~ConvRecord();

      void conv_clear();

         //
         //  do stuff
         //

      ConcatString variable;

      int nchar;
      int nreal;
      int ii;
      int mtype;

      int n_rdiag;

      int cdiag_bytes;
      int rdiag_bytes;

      unixtime date;

      char  * cdiag;   //  not allocated
      float * rdiag;   //  not allocated

      ConcatString date_string () const;

      ConcatString station_name (int index) const;             //  zero-based

      double rdiag_get_2d(int data_index, int station) const;  //  zero-based

      double rdiag_get_guess(int station) const;               //  zero-based

      double rdiag_get_guess_v(int station) const;             //  zero-based

};


////////////////////////////////////////////////////////////////////////


#endif   /*  __GSI_CONV_RECORD_H__  */


////////////////////////////////////////////////////////////////////////


