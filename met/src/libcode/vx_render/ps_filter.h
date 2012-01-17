// *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*
// ** Copyright UCAR (c) 1992 - 2012
// ** University Corporation for Atmospheric Research (UCAR)
// ** National Center for Atmospheric Research (NCAR)
// ** Research Applications Lab (RAL)
// ** P.O.Box 3000, Boulder, Colorado, 80307-3000, USA
// *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*



////////////////////////////////////////////////////////////////////////


#ifndef  __PS_FILTER_H__
#define  __PS_FILTER_H__


////////////////////////////////////////////////////////////////////////


   //
   //  filter types
   //


static const int NoFilter         = -1;

static const int ASCII85Encode    =  1;

static const int HexEncode        =  2;

static const int RunLengthEncode  =  3;


////////////////////////////////////////////////////////////////////////


static const int max_filters = 10;


////////////////////////////////////////////////////////////////////////


class PSFilter {

   public:

      PSFilter();
      virtual ~PSFilter();

      PSFilter *next;

      virtual void eat(unsigned char);

      virtual void eod();

};


////////////////////////////////////////////////////////////////////////


#endif   //  __PS_FILTER_H__


////////////////////////////////////////////////////////////////////////


