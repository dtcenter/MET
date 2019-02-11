// *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*
// ** Copyright UCAR (c) 1992 - 2019
// ** University Corporation for Atmospheric Research (UCAR)
// ** National Center for Atmospheric Research (NCAR)
// ** Research Applications Lab (RAL)
// ** P.O.Box 3000, Boulder, Colorado, 80307-3000, USA
// *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*

////////////////////////////////////////////////////////////////////////

#ifndef  __TC_STAT_FILES_H__
#define  __TC_STAT_FILES_H__

////////////////////////////////////////////////////////////////////////

#include <iostream>
#include <map>

#include "vx_tc_util.h"
#include "vx_util.h"
#include "vx_cal.h"
#include "vx_log.h"

////////////////////////////////////////////////////////////////////////

class TCStatFiles {

   private:

      void init_from_scratch();

      void assign(const TCStatFiles &);

      StringArray FileList;

      int CurFile;

      LineDataFile CurLDF;

   public:

      TCStatFiles();
      virtual ~TCStatFiles();
      TCStatFiles(const TCStatFiles &);
      TCStatFiles & operator=(const TCStatFiles &);

      void clear();

      void add_files(const StringArray &);

      void rewind();

      bool operator>>(TrackPairInfo    &);
      bool operator>>(ProbRIRWPairInfo &);
      bool operator>>(TCStatLine       &);

};

////////////////////////////////////////////////////////////////////////

#endif   /*  __TC_STAT_FILES_H__  */

////////////////////////////////////////////////////////////////////////
