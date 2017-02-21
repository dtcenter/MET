
// *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*
// ** Copyright UCAR (c) 1992 - 2017
// ** University Corporation for Atmospheric Research (UCAR)
// ** National Center for Atmospheric Research (NCAR)
// ** Research Applications Lab (RAL)
// ** P.O.Box 3000, Boulder, Colorado, 80307-3000, USA
// *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*


////////////////////////////////////////////////////////////////////////


using namespace std;


////////////////////////////////////////////////////////////////////////


#include "calipso_5km.h"


////////////////////////////////////////////////////////////////////////


   //
   //  Code for struct Calipso_5km_data
   //


////////////////////////////////////////////////////////////////////////


Calipso_5km_data::Calipso_5km_data()

{

clear();

}


////////////////////////////////////////////////////////////////////////


void Calipso_5km_data::clear()

{

hdf_sd_id = -1;

lat_info.clear();
lon_info.clear();
time_info.clear();

top_layer_info.clear();
top_pressure_info.clear();

base_layer_info.clear();
base_pressure_info.clear();

opacity_flag_info.clear();

cad_score_info.clear();

num_layers_info.clear();


      //

return;

};


////////////////////////////////////////////////////////////////////////


void Calipso_5km_data::get_var_info(const int _hdf_sd_id)

{

clear();

hdf_sd_id = _hdf_sd_id;

get_hdf_var_info(hdf_sd_id, hdf_lat_name,   lat_info);
get_hdf_var_info(hdf_sd_id, hdf_lon_name,   lon_info);
get_hdf_var_info(hdf_sd_id, hdf_time_name, time_info);

get_hdf_var_info(hdf_sd_id, hdf_base_pressure_name,  base_pressure_info);
get_hdf_var_info(hdf_sd_id, hdf_top_pressure_name,    top_pressure_info);
get_hdf_var_info(hdf_sd_id, hdf_base_layer_name,        base_layer_info);
get_hdf_var_info(hdf_sd_id, hdf_top_layer_name,          top_layer_info);
get_hdf_var_info(hdf_sd_id, hdf_opacity_flag_name,    opacity_flag_info);
get_hdf_var_info(hdf_sd_id, hdf_cad_score_name,          cad_score_info);
get_hdf_var_info(hdf_sd_id, hdf_num_layers_name,        num_layers_info);
// get_hdf_var_info(hdf_sd_id, hdf_fclass_name,                fclass_info);


      //

return;

}


////////////////////////////////////////////////////////////////////////



