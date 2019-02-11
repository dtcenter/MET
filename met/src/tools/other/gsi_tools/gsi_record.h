// *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*
// ** Copyright UCAR (c) 1992 - 2019
// ** University Corporation for Atmospheric Research (UCAR)
// ** National Center for Atmospheric Research (NCAR)
// ** Research Applications Lab (RAL)
// ** P.O.Box 3000, Boulder, Colorado, 80307-3000, USA
// *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*



////////////////////////////////////////////////////////////////////////


#ifndef  __GSI_RECORD_H__
#define  __GSI_RECORD_H__


////////////////////////////////////////////////////////////////////////


   //
   //  Base class for the other record types
   //


class GsiRecord {

   protected:

      void gsi_init_from_scratch();

      void gsi_assign(const GsiRecord &);

      unsigned char * Buf;   //  allocated

      int Nalloc;

      void extend(int);

      bool Shuffle;

      int RecPadLength;

   public:

      GsiRecord();
      virtual ~GsiRecord();

      void gsi_clear();

         //
         //  set stuff
         //

      bool set_shuffle(bool);

      bool set_record_pad_length(int);


         //
         //  get stuff
         //

      bool shuffle       () const;
      int rec_pad_length () const;

         //
         //  do stuff
         //


};


////////////////////////////////////////////////////////////////////////


inline bool GsiRecord::shuffle        () const { return ( Shuffle ); }
inline int  GsiRecord::rec_pad_length () const { return ( RecPadLength ); }


////////////////////////////////////////////////////////////////////////


extern bool operator>>(const int, GsiRecord &);


////////////////////////////////////////////////////////////////////////


#endif   /*  __GSI_RECORD_H__  */


////////////////////////////////////////////////////////////////////////


