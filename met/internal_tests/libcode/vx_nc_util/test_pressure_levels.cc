// *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*
// ** Copyright UCAR (c) 1992 - 2020
// ** University Corporation for Atmospheric Research (UCAR)
// ** National Center for Atmospheric Research (NCAR)
// ** Research Applications Lab (RAL)
// ** P.O.Box 3000, Boulder, Colorado, 80307-3000, USA
// *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*
////////////////////////////////////////////////////////////////////////

using namespace std;

#include <iostream>
#include <fstream>
#include <unistd.h>
#include <stdlib.h>
#include <cmath>

#include "vx_util.h"
#include "vx_tc_nc_util.h"

////////////////////////////////////////////////////////////////////////

static ConcatString program_name;

////////////////////////////////////////////////////////////////////////

int main(int argc, char *argv[]) {

    program_name = get_short_name(argv[0]);

    map<string, vector<string> > variable_levels;

    vector<string> u_levels;
    u_levels.push_back("P1000");
    u_levels.push_back("P800");
    u_levels.push_back("P500");
    variable_levels["U"] = u_levels;

    vector<string> v_levels;
    v_levels.push_back("P1000");
    v_levels.push_back("P900");
    v_levels.push_back("P700");
    v_levels.push_back("P500");
    v_levels.push_back("P300");
    v_levels.push_back("P100");
    v_levels.push_back("P50");
    v_levels.push_back("P10");
    variable_levels["V"] = v_levels;

    for (map<string, vector<string> >::iterator i = variable_levels.begin();
        i != variable_levels.end(); ++i) {
        cout << i->first << ": ";
        vector<string> levels = variable_levels[i->first];
        for (int j = 0; j < levels.size(); j++) {
            cout << levels[j] << " ";
        }
        cout << endl;
    }
    cout << endl;

    // set<double> pressure_levels
    //     = get_pressure_levels(variable_levels);

    // for (set<double>::iterator i = pressure_levels.begin();
    //     i != pressure_levels.end(); ++i) {
    //         cout << *i << " ";
    // }
    // cout << endl;

    set<string> pressure_level_strings
        = get_pressure_level_strings(variable_levels);

    for (set<string>::iterator i = pressure_level_strings.begin();
        i != pressure_level_strings.end(); ++i) {
            cout << *i << " ";
    }
    cout << endl << endl;

    set<double> pressure_levels
        = get_pressure_levels(pressure_level_strings);

    for (set<double>::iterator i = pressure_levels.begin();
        i != pressure_levels.end(); ++i) {
            cout << *i << " ";
    }
    cout << endl << endl;

    map<double, int> pressure_level_indices
        = get_pressure_level_indices(pressure_levels);

    for (set<double>::iterator i = pressure_levels.begin();
        i != pressure_levels.end(); ++i) {
            cout << *i << " : " << pressure_level_indices[*i] << endl;
    }
    cout << endl;

    map<string, int> pressure_level_indices_string_key
        = get_pressure_level_indices(pressure_level_strings, pressure_levels);

    for (set<string>::iterator i = pressure_level_strings.begin();
        i != pressure_level_strings.end(); ++i) {
            cout << *i << " : " << pressure_level_indices_string_key[*i] << endl;
    }
    cout << endl;

    return 0;
}

////////////////////////////////////////////////////////////////////////
