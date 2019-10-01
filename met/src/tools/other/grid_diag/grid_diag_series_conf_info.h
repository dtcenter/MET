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

        StringArray mask_grid; // Masking grid strings
        StringArray mask_poly; // Masking polyline strings
        StringArray mask_name; // Masking region names

        int get_n_mask() const;
};

////////////////////////////////////////////////////////////////////////

inline int GridDiagVxOpt::get_n_mask() const {
    return mask_name.n_elements();
}

////////////////////////////////////////////////////////////////////////

class GridDiagConfInfo {

    private:

        void init_from_scratch();

        // Number of data fields
        int n_data;

    public:

        // GridDiag configuration object
        MetConfig Conf;

        // Config file version
        ConcatString version;

        // Masking grid or polyline
        ConcatString mask_grid;
        ConcatString mask_poly;
        ConcatString mask_name;

        // Variable information
        VarInfo** data_info;

        GridDiagConfInfo();
        ~GridDiagConfInfo();

        void clear();

        void read_config(const char*, const char*);
        void process_config(GrdFileType);
        void process_mask(const Grid&);

        int get_n_data() const;
};

////////////////////////////////////////////////////////////////////////

inline int GridDiagConfInfo::get_n_data() const {
    return n_data;
}

////////////////////////////////////////////////////////////////////////

#endif  /*  __GRID_DIAG_CONF_INFO_H__  */

////////////////////////////////////////////////////////////////////////
