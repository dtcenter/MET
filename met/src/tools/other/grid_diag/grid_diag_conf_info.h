////////////////////////////////////////////////////////////////////////

#ifndef  __GRID_DIAG_CONF_INFO_H__
#define  __GRID_DIAG_CONF_INFO_H__

using namespace std;

#include <iostream>

#include "vx_config.h"
#include "vx_util.h"

////////////////////////////////////////////////////////////////////////

class GridDiagConfInfo {

    private:

        void init_from_scratch();

    public:

        // GridDiag configuration object
        MetConfig Conf;

        // Config file version
        ConcatString Version;

        GridDiagConfInfo();
        ~GridDiagConfInfo();

        void clear();

        void read_config(const char*, const char*);
        void process_config();
};

#endif  /*  __GRID_DIAG_CONF_INFO_H__  */

////////////////////////////////////////////////////////////////////////
