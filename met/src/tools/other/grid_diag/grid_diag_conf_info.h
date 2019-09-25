////////////////////////////////////////////////////////////////////////

#ifndef  __GRID_DIAG_CONF_INFO_H__
#define  __GRID_DIAG_CONF_INFO_H__

using namespace std;

#include <iostream>

#include "vx_config.h"
#include "vx_data2d_factory.h"
#include "vx_data2d.h"
#include "vx_util.h"

////////////////////////////////////////////////////////////////////////

class GridDiagVxOpt {

    private:

        void init_from_scratch();

    public:

        GridDiagVxOpt();
        ~GridDiagVxOpt();

};

////////////////////////////////////////////////////////////////////////

class GridDiagConfInfo {

    private:

        void init_from_scratch();

    public:

        // GridDiag configuration object
        MetConfig Conf;

        // Config file version
        ConcatString Version;

        // Variable information
        VarInfo** data_info;

        GridDiagConfInfo();
        ~GridDiagConfInfo();

        void clear();

        void read_config(const char*, const char*);
        void process_config();

        int get_n_data() const;
};

#endif  /*  __GRID_DIAG_CONF_INFO_H__  */

////////////////////////////////////////////////////////////////////////
