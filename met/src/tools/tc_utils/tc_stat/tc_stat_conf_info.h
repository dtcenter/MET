// *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*
// ** Copyright UCAR (c) 1992 - 2020
// ** University Corporation for Atmospheric Research (UCAR)
// ** National Center for Atmospheric Research (NCAR)
// ** Research Applications Lab (RAL)
// ** P.O.Box 3000, Boulder, Colorado, 80307-3000, USA
// *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*

////////////////////////////////////////////////////////////////////////

#ifndef  __TC_STAT_CONF_INFO_H__
#define  __TC_STAT_CONF_INFO_H__

////////////////////////////////////////////////////////////////////////

#include <iostream>

#include "tc_stat_job.h"

#include "mask_poly.h"

#include "vx_config.h"
#include "vx_util.h"

////////////////////////////////////////////////////////////////////////

class TCStatConfInfo {

   private:

      void init_from_scratch();

   public:

      // TCStat configuration object
      MetConfig Conf;

      // TCStatJob to store configuration file filtering options
      TCStatJob Filter;

      // Jobs array
      StringArray Jobs;

      // Config file version
      ConcatString Version;

      TCStatConfInfo();
     ~TCStatConfInfo();

      void clear();

      void read_config(const char *, const char *);
      void process_config();
};

////////////////////////////////////////////////////////////////////////

#endif   /*  __TC_STAT_CONF_INFO_H__  */

////////////////////////////////////////////////////////////////////////
