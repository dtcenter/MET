// *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*
// ** Copyright UCAR (c) 1992 - 2016
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
   TCStatLineType_ProbRI,
   TCStatLineType_Header,
   NoTCStatLineType

};

extern TCStatLineType string_to_tcstatlinetype(const char *);
extern ConcatString   tcstatlinetype_to_string(const TCStatLineType);

////////////////////////////////////////////////////////////////////////

static const char *TCStatLineType_TCMPR_Str  = "TCMPR";
static const char *TCStatLineType_ProbRI_Str = "PROBRI";
static const char *TCStatLineType_Header_Str = "LINE_TYPE";

////////////////////////////////////////////////////////////////////////

class TCStatLine : public DataLine {

   private:

      void assign(const TCStatLine &);

      void determine_line_type();

      TCStatLineType Type;

   public:

      TCStatLine();
     ~TCStatLine();
      TCStatLine(const TCStatLine &);
      TCStatLine & operator=(const TCStatLine &);

      int read_line(LineDataFile *);   //  virtual from base class

      int is_ok() const;               //  virtual from base class

      int is_header() const;           //  virtual from base class

      //
      // Retrieve values of the header columns
      //

      const char * get_item     (int, bool check_na = true) const;
      const char * get_item     (const char *, bool check_na = true) const;

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

extern int determine_column_offset(const TCStatLine &, const char *,
                                   bool error_out = true);

////////////////////////////////////////////////////////////////////////

#endif   /*  __TC_STAT_LINE_H__  */

////////////////////////////////////////////////////////////////////////
