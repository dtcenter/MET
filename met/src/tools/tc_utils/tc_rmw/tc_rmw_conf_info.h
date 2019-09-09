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
#include "vx_data2d_factory.h"
#include "vx_data2d.h"
#include "vx_util.h"

////////////////////////////////////////////////////////////////////////

class TCRMWConfInfo {

    private:

        void init_from_scratch();

        // Number of data fields
        int n_data;

    public:

        // TCRMW configuration object
        MetConfig Conf;

        // Track filtering criteria
        ConcatString Model;     // Model name
        ConcatString Basin;     // Basin name
        ConcatString StormName; // Storm name
        ConcatString StormId;   // Storm id

        int Cyclone;  // Cyclone number

        // Timing information
        unixtime  InitTime;
        NumArray  LeadTimes;

        int n_range;
        int n_azimuth;
        double max_range_km;
        double delta_range_km;
        double rmw_scale;

        // Variable information
        VarInfo** data_info;

        // Config file version
        ConcatString Version;

        TCRMWConfInfo();
        ~TCRMWConfInfo();

        void clear();

        void read_config(const char *, const char *);
        void process_config(GrdFileType);

        int get_n_data() const;
};

////////////////////////////////////////////////////////////////////////

inline int TCRMWConfInfo::get_n_data() const {
    return n_data;
}

////////////////////////////////////////////////////////////////////////

#endif   /*  __TC_RMW_CONF_INFO_H__  */

////////////////////////////////////////////////////////////////////////
