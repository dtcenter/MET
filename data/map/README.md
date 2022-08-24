Map Data
========

The data provided in this directory is used by the MET tools to add map outlines to plots. By default, the data files listed in **data/config/ConfigMapData** are plotted. However, users can override the **map_data** dictionary in the configuration files passed to the MET tools on the command line.

The map data provide here is derived from [Natural Earth](https://www.naturalearthdata.com/) shapefiles. It corresponds to the 1:110m and 1:10m scale shapefiles and was last updated using version 5.5.1 from May 2022.

The map data was reformatted using calls to the **make_mapfiles** development utility. Those calls are described below.

- **country_data** (~225K) contains 110m resolution global country outlines.
```
make_mapfiles \
110m_cultural/ne_110m_admin_0_countries.shp \
110m_cultural/ne_110m_admin_0_countries.shx \
110m_cultural/ne_110m_admin_0_countries.dbf NAME ADMIN
mv ne_110m_admin_0_countries_data country_data
```
- **major_lakes_data** (~244K) contains 110m resolution global major lake outlines.
```
make_mapfiles \
110m_physical/ne_110m_lakes.shp \
110m_physical/ne_110m_lakes.shx \
110m_physical/ne_110m_lakes.dbf name NA
mv 110m_cultural/110m_physical/ne_110m_lakes_data major_lakes_data
```
- **country_major_lakes_data** (~464K) contains 110m resolution global country and major lake outlines.
```
make_mapfiles \
110m_cultural/ne_110m_admin_0_countries_lakes.shp \
110m_cultural/ne_110m_admin_0_countries_lakes.shx \
110m_cultural/ne_110m_admin_0_countries_lakes.dbf NAME ADMIN
mv ne_110m_admin_0_countries_lakes_data country_major_lakes_data
```
- **usa_state_data** (~48K) contains 110m resolution administrative boundaries for the United States.
```
make_mapfiles \
110m_cultural/ne_110m_admin_1_states_provinces.shp \
110m_cultural/ne_110m_admin_1_states_provinces.shx \
110m_cultural/ne_110m_admin_1_states_provinces.dbf name NA
mv ne_110m_admin_1_states_provinces_data usa_state_data
```
- **country_major_lakes_detail_data** (~11M) contains 10m resolution global country and major lake outlines.
```
make_mapfiles \
10m_cultural/ne_10m_admin_0_countries_lakes.shp \
10m_cultural/ne_10m_admin_0_countries_lakes.shx \
10m_cultural/ne_10m_admin_0_countries_lakes.dbf NAME ADMIN
mv ne_10m_admin_0_countries_lakes_data country_major_lakes_detail_data
```
- **admin_by_country** (~26M) contains 10m resolution state/province data organized into separate data files for each country.
```
make_mapfiles -outdir admin_by_country -separate_files \
10m_cultural/ne_10m_admin_1_states_provinces.shp \
10m_cultural/ne_10m_admin_1_states_provinces.shx \
10m_cultural/ne_10m_admin_1_states_provinces.dbf admin name
for file in `ls admin_by_country/ne_10m_admin_1_*`; do
  mv $file `echo $file | sed 's/ne_10m_admin_1_states_provinces/admin/g'`
done
```
