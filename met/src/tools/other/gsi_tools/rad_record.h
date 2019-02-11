// *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*
// ** Copyright UCAR (c) 1992 - 2019
// ** University Corporation for Atmospheric Research (UCAR)
// ** National Center for Atmospheric Research (NCAR)
// ** Research Applications Lab (RAL)
// ** P.O.Box 3000, Boulder, Colorado, 80307-3000, USA
// *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*



////////////////////////////////////////////////////////////////////////


#ifndef  __GSI_RAD_RECORD_H__
#define  __GSI_RAD_RECORD_H__


////////////////////////////////////////////////////////////////////////


#include "gsi_record.h"
#include "vx_cal.h"


////////////////////////////////////////////////////////////////////////


struct RadParams {

   char obstype [11];
   char dplat   [11];
   char isis    [21];

   int jiter;
   int nchanl;
   int npred;
   int idate;
   int ireal;
   int ipchan;
   int iextra;
   int jextra;

   int idiag;
   int angord;
   int iversion;
   int inewpc;

   RadParams();

   void dump(ostream &) const;

};


////////////////////////////////////////////////////////////////////////


struct ChannelParams {

   float freq;
   float plo;
   float wave;
   float varch;
   float tlap;

   int iuse;
   int nuchan;
   int ich;

   ChannelParams();

   void dump(ostream &) const;

};


////////////////////////////////////////////////////////////////////////


class RadRecord;   //  forward reference


////////////////////////////////////////////////////////////////////////


class RadFile {

   friend bool operator>>(RadFile &, RadRecord &);

   private:

      void init_from_scratch();

      ConcatString Filename;

      int Fd;   //  the file descriptor

      bool SwapEndian;

      int RecPadSize;

      RadParams R_params;

      ChannelParams * C_params;   //  allocated

      void read_rad_params();

      void read_channel(int);   //  zero-based

      unixtime Date;

      int Nchannels;

      int N1;
      int N2;

      int Ndiag;
      int Nrec;
      int Npair;

   public:

      RadFile();
     ~RadFile();

      bool open(const char * path, bool _swap_endian = true, int _rec_pad_size = 4);

      void close();

      void rewind();

         //
         //  set stuff
         //

         //
         //  get stuff
         //

      bool get_swap_endian() const;

      int get_rec_pad_size() const;

      bool has_extra() const;

      unixtime date() const;

      int n_channels() const;

      int channel_val(const int) const;

      int use_channel(const int) const;

      int n_diag() const;
      int n_rec()  const;
      int n_pair() const;

      int n1() const;
      int n2() const;

      int iextra() const;
      int jextra() const;

      int n12() const;

         //
         //  do stuff
         //

};


////////////////////////////////////////////////////////////////////////


inline bool RadFile::get_swap_endian() const { return ( SwapEndian ); }

inline int RadFile::get_rec_pad_size() const { return ( RecPadSize ); }

inline unixtime RadFile::date() const { return ( Date ); }

inline int RadFile::n_channels() const { return ( Nchannels ); }

inline int RadFile::n_diag() const { return ( Ndiag ); }
inline int RadFile::n_rec()  const { return ( Nrec  ); }
inline int RadFile::n_pair() const { return ( Npair ); }

inline int RadFile::n1() const { return ( N1 ); }
inline int RadFile::n2() const { return ( N2 ); }

inline int RadFile::n12() const { return ( N1*N2 ); }

inline int RadFile::iextra() const { return ( R_params.iextra ); }
inline int RadFile::jextra() const { return ( R_params.jextra ); }

inline bool RadFile::has_extra() const { return ( (R_params.iextra != 0) && (R_params.jextra != 0) ); }


////////////////////////////////////////////////////////////////////////


extern bool operator>>(RadFile &, RadRecord &);


////////////////////////////////////////////////////////////////////////


class RadRecord : public GsiRecord {

   friend bool operator>>(RadFile &, RadRecord &);

   protected:

      void rad_init_from_scratch();

      // void rad_assign(const GsiRecord &);

      float * diag;       //  not allocated
      float * diagchan;   //  not allocated
      float * extra;      //  not allocated

      int Ndiag;       //  # of floats, not bytes
      int Ndiagchan;   //  # of floats, not bytes
      int Nextra;      //  # of floats, not bytes

      int N1;
      int N2;

      int iextra;
      int jextra;

      unixtime Date;

   public:

      RadRecord();
     ~RadRecord();

      void rad_clear();

         //
         //  get stuff
         //

      int n1() const;
      int n2() const;

      bool has_extra() const;   //  is "extra" data present?

      unixtime date() const;

         //
         //  do stuff
         //

      double diag_data     (int)      const;   //  zero-based

      double diagchan_data (int, int) const;   //  zero-based

      double extra_data    (int, int) const;   //  zero-based ... will error out if no "extra" data exists

};


////////////////////////////////////////////////////////////////////////

inline int RadRecord::n1() const { return ( N1 ); }
inline int RadRecord::n2() const { return ( N2 ); }

inline bool RadRecord::has_extra() const { return ( extra != 0 ); }

inline unixtime RadRecord::date() const { return ( Date ); }


////////////////////////////////////////////////////////////////////////


#endif   /*  __GSI_RAD_RECORD_H__  */


////////////////////////////////////////////////////////////////////////


