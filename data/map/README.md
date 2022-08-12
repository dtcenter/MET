Map Data
========

The data provided in this directory is used by the MET tools to add map outlines to plots. By default, the data files listed in **data/config/ConfigMapData** are plotted. However, users can override the **map_data** dictionary in the configuration files passed to the MET tools on the command line.

The map data provide here is derived from [Natural Earth](https://www.naturalearthdata.com/) shapefiles. It corresponds to the 1:110m and 1:10m scale shapefiles and was last updated using version 5.5.1 from May 2022.

The map data was reformatted using calls to the **make_mapfiles** development utility. Those calls are described below.

- **country_data** contains 110m resolution country outlines.
```
make_mapfiles \
110m_cultural/ne_110m_admin_0_countries.shp \
110m_cultural/ne_110m_admin_0_countries.shx \
110m_cultural/ne_110m_admin_0_countries.dbf NAME ADMIN
mv ne_110m_admin_0_countries_data country_data
```
- **usa_state_data** contains 110m resolution state/province data for United States and Canada.
```
make_mapfiles \
110m_cultural/ne_110m_admin_1_states_provinces.shp \
110m_cultural/ne_110m_admin_1_states_provinces.shx \
110m_cultural/ne_110m_admin_1_states_provinces.dbf name NA
mv ne_110m_admin_1_states_provinces_data usa_state_data
``` 
- **admin_data** contains 10m resolution state/province data for all countries.
```
make_mapfiles \
10m_cultural/ne_10m_admin_1_states_provinces.shp \
10m_cultural/ne_10m_admin_1_states_provinces.shx \
10m_cultural/ne_10m_admin_1_states_provinces.dbf name admin
mv ne_10m_admin_1_states_provinces_data admin_data
```
- **admin_by_country** contains 10m resolution state/province organized into separate data files for each country.
```
make_mapfiles -outdir admin_by_country -separate_files \
10m_cultural/ne_10m_admin_1_states_provinces.shp \
10m_cultural/ne_10m_admin_1_states_provinces.shx \
10m_cultural/ne_10m_admin_1_states_provinces.dbf admin name
for file in `ls admin_by_country/ne_10m_admin_1_*`; do
  mv $file `echo $file | sed 's/ne_10m_admin_1_states_provinces/admin/g'`
done
```

TODO:
- [x] admin_by_country (307586 lines)
- [x] admin_data (307586 lines)
- [x] country_data
- [ ] country_major_lakes_data (23440 lines)
- [ ] major_lakes_data (12567 lines)
- [x] usa_state_data
