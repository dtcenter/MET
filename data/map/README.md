Map Data
========

The data provided in this directory is used by the MET tools to add map outlines to plots. By default, the data files listed in **data/config/ConfigMapData** are plotted. However, users can override the **map_data** dictionary in the configuration files passed to the MET tools on the command line.

The map data provide here is derived from [Natural Earth](https://www.naturalearthdata.com/) shapefiles. It corresponds to the 1:110m scale shapefiles and was last updated using version 5.5.1 from May 2022.

The map data was reformatted using calls to the **make_mapfiles** development utility. Those calls are described below.
