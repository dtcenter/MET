"""Provides routines to handle grib input.
"""
import dataclasses
import logging
import pathlib
from typing import List, Optional, Tuple

import numpy as np
import pygrib
import xarray as xr

GribFile = pygrib._pygrib.open

LOGGER = logging.getLogger(__name__)
LOGGER.addHandler(logging.NullHandler())


@dataclasses.dataclass
class InputVarSpec:
    # The name of the variable
    var_name: str
    level_type: str
    # The name of the variable in the grib file.
    # This is distinct from the var_name to allow for renaming variables from
    # what they're called in the grib file. EX: "z" is "gh" in the grib file.
    grib_name: Optional[str] = None
    is_surface: bool = False
    level: int = 0
    # Determines whether just the name should be used to select the variable
    # from the grib file. Otherwise the level and level_type are used as well.
    # This argument is only used for surface variables since the level_type and
    # level are essential to select profile variables.  If set to True and
    # is_surface is False, this parameter will be ignored.
    select_name_only: bool = False

    def __post_init__(self):
        if self.grib_name is None:
            self.grib_name = self.var_name


def grib_to_dataset(
        grib_filename: pathlib.Path,
        nav_var_spec: InputVarSpec,
        levels_hPa: List[int],
        input_var_specs: List[InputVarSpec],
        grib_file=None) -> Tuple[xr.Dataset, np.ndarray, np.ndarray]:
    if grib_file is None:
        grib_file = pygrib.index(str(grib_filename), "shortName",
                                 "typeOfLevel", "level")
        name_only_grib_file = pygrib.index(str(grib_filename), "shortName")
    else:
        name_only_grib_file = grib_file

    lons, lats = get_grib_lons_lats(grib_file, nav_var_spec.grib_name,
                                    nav_var_spec.level,
                                    nav_var_spec.level_type)
    lon_coord = lons[0, :]
    lat_coord = lats[:, 0]

    ds_vars = {}
    for var_spec in input_var_specs:
        if var_spec.is_surface:
            dat_array = _surface_var_as_data_array(grib_file,
                                                   name_only_grib_file,
                                                   var_spec, lon_coord,
                                                   lat_coord)
        else:
            dat_array = _levels_as_data_array(grib_file, var_spec, levels_hPa,
                                              lon_coord, lat_coord)
        ds_vars[var_spec.var_name] = dat_array

    grib_ds = xr.Dataset(data_vars=ds_vars,
                         coords=dict(level=levels_hPa,
                                     lat=lat_coord,
                                     lon=lon_coord))

    grib_file.close()
    name_only_grib_file.close()
    return grib_ds, lons, lats


def _surface_var_as_data_array(grib_file: GribFile,
                               name_only_grib_file: GribFile,
                               var_spec: InputVarSpec, lon_coord: List[float],
                               lat_coord: List[float]) -> xr.DataArray:
    try:
        if var_spec.select_name_only:
            grib_record = name_only_grib_file.select(
                shortName=var_spec.grib_name)
        else:
            grib_record = grib_file.select(shortName=var_spec.grib_name,
                                           level=var_spec.level,
                                           typeOfLevel=var_spec.level_type)
    except ValueError:
        LOGGER.exception(f"Could not find grib entry for spec:{var_spec}")
        raise

    dat_array = xr.DataArray(data=grib_record[0].values,
                             dims=["lat", "lon"],
                             coords=dict(lat=lat_coord, lon=lon_coord))
    return dat_array


def _levels_as_data_array(grib_file: GribFile, var_spec: InputVarSpec,
                          levels_hPa: List[int], lon_coord: List[float],
                          lat_coord: List[float]) -> xr.DataArray:
    level_data = []
    for level in levels_hPa:
        grib_record = grib_file.select(shortName=var_spec.grib_name,
                                       level=level,
                                       typeOfLevel=var_spec.level_type)
        data = grib_record[0].values
        level_data.append(data)

    sounding_data = np.array(level_data)
    dat_array = xr.DataArray(data=sounding_data,
                             dims=["level", "lat", "lon"],
                             coords=dict(level=levels_hPa,
                                         lat=lat_coord,
                                         lon=lon_coord))
    return dat_array


def get_grib_lons_lats(grib_file: GribFile, var_name: str, level: int,
                       level_type: str):
    g = grib_file.select(shortName=var_name,
                         level=level,
                         typeOfLevel=level_type)
    lats, lons = g[0].latlons()
    return lons, lats
