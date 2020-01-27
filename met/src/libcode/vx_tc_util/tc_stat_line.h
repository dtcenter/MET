// *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*
// ** Copyright UCAR (c) 1992 - 2020
// ** University Corporation for Atmospheric Research (UCAR)
// ** National Center for Atmospheric Research (NCAR)
// ** Research Applications Lab (RAL)
// ** P.O.Box 3000, Boulder, Colorado, 80307-3000, USA
// *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*

////////////////////////////////////////////////////////////////////////

#ifndef  __TC_STAT_LINE_H__
#define  __TC_STAT_LINE_H__

////////////////////////////////////////////////////////////////////////

#include <iostream>
#include <fstream>

#include "vx_util.h"
#include "vx_cal.h"

////////////////////////////////////////////////////////////////////////

// Enumerate all the possible line types
enum TCStatLineType {

   TCStatLineType_TCMPR,
   TCStatLineType_ProbRIRW,
   TCStatLineType_Header,
   NoTCStatLineType

};

extern TCStatLineType string_to_tcstatlinetype(const char *);
extern ConcatString   tcstatlinetype_to_string(const TCStatLineType);

////////////////////////////////////////////////////////////////////////

static const char TCStatLineType_TCMPR_Str[]    = "TCMPR";
static const char TCStatLineType_ProbRIRW_Str[] = "PROBRIRW";
static const char TCStatLineType_Header_Str[]   = "LINE_TYPE";

////////////////////////////////////////////////////////////////////////

class TCStatLine : public DataLine {

   private:

      TCStatLineType Type;

      const AsciiHeaderLine *HdrLine;   //  not allocated

      void init_from_scratch();

      void assign(const TCStatLine &);

   public:

      TCStatLine();
     ~TCStatLine();
      TCStatLine(const TCStatLine &);
      TCStatLine & operator=(const TCStatLine &);

      void clear();

      int read_line(LineDataFile *);   //  virtual from base class

      bool is_ok() const;              //  virtual from base class

      bool is_header() const;          //  virtual from base class

      //
      // Retrieve values of the header columns
      //

      ConcatString get          (const char *, bool check_na = true) const;
      const char * get_item     (const char *, bool check_na = true) const;
      const char * get_item     (int,          bool check_na = true) const;

      const char * version      () const;
      const char * amodel       () const;
      const char * bmodel       () const;
      const char * desc         () const;
      const char * storm_id     () const;
      const char * basin        () const;
      const char * cyclone      () const;
      const char * storm_name   () const;
      unixtime     init         () const;
      int          init_hour    () const; // init%sec_per_day
      int          lead         () const;
      unixtime     valid        () const;
      int          valid_hour   () const; // valid%sec_per_day
      const char * init_mask    () const;
      const char * valid_mask   () const;
      const char * initials     () const;
      const char * line_type    () const;
      ConcatString header       () const;

      TCStatLineType type       () const;
};

////////////////////////////////////////////////////////////////////////

inline  TCStatLineType TCStatLine::type() const { return(Type); }

////////////////////////////////////////////////////////////////////////

#endif   /*  __TC_STAT_LINE_H__  */

////////////////////////////////////////////////////////////////////////
