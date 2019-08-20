////////////////////////////////////////////////////////////////////////

#ifndef  __MODE_MULTIVAR_CONF_INFO_H__
#define  __MODE_MULTIVAR_CONF_INFO_H__

using namespace std;

#include <iostream>

#include "vx_config.h"
#include "vx_util.h"

////////////////////////////////////////////////////////////////////////

class ModeMultivarConfInfo {

    private:

        void init_from_scratch();

    public:

        // ModeMultivar configuration object
        MetConfig Conf;

        // Config file version
        ConcatString Version;

        ModeMultivarConfInfo();
        ~ModeMultivarConfInfo();

        void clear();

        void read_config(const char*, const char*);
        void process_config();
};

#endif  /*  __MODE_MULTIVAR_CONF_INFO_H__  */

////////////////////////////////////////////////////////////////////////
