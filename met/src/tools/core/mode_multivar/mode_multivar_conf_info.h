////////////////////////////////////////////////////////////////////////

#ifndef  __MODE_MULTIVAR_CONF_INFO_H__
#define  __MODE_MULTIVAR_CONF_INFO_H__

using namespace std;

#include <iostream>

#include "vx_config.h"
#include "vx_util.h"

////////////////////////////////////////////////////////////////////////

class ModeMultiVarConfInfo {

    private:

        void init_from_scratch();

    public:

        // ModeMultiVar configuration object
        MetConfig Conf;

        // Config file version
        ConcatString Version;

        ModeMultiVarConfInfo();
        ~ModeMultiVarConfInfo();

        void clear();

        void read_config(const char*, const char*);
        void process_config();
};

#endif  /*  __MODE_MULTIVAR_CONF_INFO_H__  */

////////////////////////////////////////////////////////////////////////
