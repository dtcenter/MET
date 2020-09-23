// *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*
// ** Copyright UCAR (c) 1992 - 2020
// ** University Corporation for Atmospheric Research (UCAR)
// ** National Center for Atmospheric Research (NCAR)
// ** Research Applications Lab (RAL)
// ** P.O.Box 3000, Boulder, Colorado, 80307-3000, USA
// *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*



////////////////////////////////////////////////////////////////////////


#ifndef  __PS_FILTER_H__
#define  __PS_FILTER_H__


////////////////////////////////////////////////////////////////////////


#include "concat_string.h"


////////////////////////////////////////////////////////////////////////


   //
   //  filter types
   //


static const int NoFilter         = -1;

static const int ASCII85Encode    =  1;

static const int HexEncode        =  2;

static const int RunLengthEncode  =  3;

static const int FlateEncode      =  4;


////////////////////////////////////////////////////////////////////////


static const int max_filters = 10;


////////////////////////////////////////////////////////////////////////


class PSFilter {

   public:

      PSFilter();
      virtual ~PSFilter();

      PSFilter *next;

      int DecimalPlaces;

      char double_format[32];

      virtual void eat(unsigned char);

      virtual void eod();

      virtual void set_decimal_places(int);

      virtual PSFilter & operator<<(const char);
      virtual PSFilter & operator<<(const int);
      virtual PSFilter & operator<<(const double);
      virtual PSFilter & operator<<(const char *);
      virtual PSFilter & operator<<(const ConcatString &);

};


////////////////////////////////////////////////////////////////////////


inline PSFilter & PSFilter::operator<<(char c)

{

eat( (unsigned char) c );

return ( *this );

}


////////////////////////////////////////////////////////////////////////


#endif   //  __PS_FILTER_H__


////////////////////////////////////////////////////////////////////////


