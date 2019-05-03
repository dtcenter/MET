// *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*
// ** Copyright UCAR (c) 1992 - 2018
// ** University Corporation for Atmospheric Research (UCAR)
// ** National Center for Atmospheric Research (NCAR)
// ** Research Applications Lab (RAL)
// ** P.O.Box 3000, Boulder, Colorado, 80307-3000, USA
// *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*



////////////////////////////////////////////////////////////////////////


#ifndef  __CHECK_ENDIAN_H__
#define  __CHECK_ENDIAN_H__


////////////////////////////////////////////////////////////////////////


   //
   //  these constants will have the same
   //    numerical value on either
   //    big-endian or little-endian
   //    systems
   //


static const int    big_endian = 0x01010101;
static const int little_endian = 0x02020202;

#ifdef BIGENDIAN
   static const int native_endian = big_endian;
#else
   static const int native_endian = little_endian;
#endif

////////////////////////////////////////////////////////////////////////


inline void swap_uc(unsigned char & a, unsigned char & b)

{

unsigned char t;

t = a;

a = b;

b = t;

return;

}


////////////////////////////////////////////////////////////////////////


extern void shuffle_2(void *);

extern void shuffle_4(void *);

extern void shuffle_8(void *);


////////////////////////////////////////////////////////////////////////


#endif   //  __CHECK_ENDIAN_H__


////////////////////////////////////////////////////////////////////////


