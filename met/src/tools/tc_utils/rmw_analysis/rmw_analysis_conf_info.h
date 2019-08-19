// *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*
// ** Copyright UCAR (c) 1992 - 2019
// ** University Corporation for Atmospheric Research (UCAR)
// ** National Center for Atmospheric Research (NCAR)
// ** Research Applications Lab (RAL)
// ** P.O.Box 3000, Boulder, Colorado, 80307-3000, USA
// *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*

////////////////////////////////////////////////////////////////////////

#ifndef  __RMW_ANALYSIS_CONF_INFO_H__
#define  __RMW_ANALYSIS_CONF_INFO_H__

////////////////////////////////////////////////////////////////////////

using namespace std;

#include <iostream>

#include "vx_config.h"
#include "vx_data2d_factory.h"
#include "vx_data2d.h"
#include "vx_util.h"

////////////////////////////////////////////////////////////////////////

struct ConsensusInfo {
    ConcatString Name;
    StringArray  Members;
    NumArray     Required;
    int          MinReq;
};

////////////////////////////////////////////////////////////////////////

class RMWAnalysisConfInfo {

    private:

        void init_from_scratch();

        // Number of forecast fields
        int n_data;

    public:

        // RMWAnalysis configuration object
        MetConfig Conf;

        // Variable information
        VarInfo** data_info;

        // Config file version
        ConcatString Version;

        RMWAnalysisConfInfo();
        ~RMWAnalysisConfInfo();

        void clear();

        void read_config(const char *, const char *);
        void process_config();

        int get_n_data() const;
};

////////////////////////////////////////////////////////////////////////

inline int RMWAnalysisConfInfo::get_n_data() const {
    return n_data;
}

////////////////////////////////////////////////////////////////////////

#endif   /*  __RMW_ANALYSIS_CONF_INFO_H__  */

////////////////////////////////////////////////////////////////////////
