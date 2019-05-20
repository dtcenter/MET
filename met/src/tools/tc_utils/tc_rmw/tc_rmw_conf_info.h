// *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*
// ** Copyright UCAR (c) 1992 - 2019
// ** University Corporation for Atmospheric Research (UCAR)
// ** National Center for Atmospheric Research (NCAR)
// ** Research Applications Lab (RAL)
// ** P.O.Box 3000, Boulder, Colorado, 80307-3000, USA
// *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*

////////////////////////////////////////////////////////////////////////

#ifndef  __TC_RMW_CONF_INFO_H__
#define  __TC_RMW_CONF_INFO_H__

////////////////////////////////////////////////////////////////////////

#include <iostream>

#include "vx_config.h"
#include "vx_util.h"

////////////////////////////////////////////////////////////////////////

struct ConsensusInfo {
    ConcatString Name;
    StringArray  Members;
    NumArray     Required;
    int          MinReq;
};

////////////////////////////////////////////////////////////////////////

class TCRMWConfInfo {

    private:

        void init_from_scratch();

    public:

        // TCRMW configuration object
        MetConfig Conf;

        // User-defined description
        ConcatString Desc;

        // Track filtering criteria
        StringArray Model;     // List of model names
        StringArray StormId;   // List of storm ids
        StringArray Basin;     // List of basin names
        StringArray Cyclone;   // List of cyclone numbers
        StringArray StormName; // List of storm names

        // Timing information
        unixtime  InitBeg, InitEnd;
        TimeArray InitInc;
        TimeArray InitExc;
        NumArray  InitHour;
        NumArray  LeadReq;
        unixtime  ValidBeg, ValidEnd;

        // Track datasets
        TrackType Track;

        bool CheckDup;

        int n_range;
        int n_azimuth;
        double max_range;
        double delta_range;

        // Config file version
        ConcatString Version;

        TCRMWConfInfo();
        ~TCRMWConfInfo();

        void clear();

        void read_config(const char *, const char *);
        void process_config();
};

////////////////////////////////////////////////////////////////////////

#endif   /*  __TC_RMW_CONF_INFO_H__  */

////////////////////////////////////////////////////////////////////////
