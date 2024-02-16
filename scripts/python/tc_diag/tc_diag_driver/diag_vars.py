"""Contains routines for performing diagnositc variable calculations.

The routines are intended to be used with the "Computation Engine", but can
be used independently as well. In order to work with the computation engine,
they must accept kwargs even if they're not used, and they must return at least
one float.
"""
import dataclasses
import datetime as dt
import logging
import pathlib
import sys
from typing import Callable, Optional, TextIO, Tuple

import numpy as np
import pandas as pd
import xarray as xr
from diag_lib import cylindrical_grid as cg

from tc_diag_driver import computation_engine as ce
from tc_diag_driver import results as hr_results

LOGGER = logging.getLogger(__name__)
LOGGER.addHandler(logging.NullHandler())

DEG_TO_NMI = 60.0


def debug_cyl_grid_dump(
    hour: int,
    grib_dataset: xr.Dataset,
    cylindrical_grid_interpolator: cg.CylindricalGridInterpolator,
    output_filename: str,
    grib_var_name: str,
    level_hPa: Optional[int] = None,
    unit_converter: str = None,
    **_kwargs,
) -> float:
    LOGGER.info(
        "Started debug dump of %s converted to cylindrical grid in file: %s",
        grib_var_name,
        output_filename,
    )
    data = _obtain_data_at_level(grib_dataset, grib_var_name, level_hPa)
    data = _convert(data, unit_converter)

    lerp = cylindrical_grid_interpolator
    data_on_grid = lerp(data)

    out_fname = output_filename.format(
        grib_var_name=grib_var_name, level_hPa=level_hPa, hour=hour
    )

    data_vars = {grib_var_name: (["theta_radians", "radius_km"], data_on_grid)}
    theta_radians = lerp.theta_2d_radians[:, 0]
    radius_km = lerp.rad_2d_km[0, :]
    ds = xr.Dataset(
        data_vars=data_vars,
        coords=dict(theta_radians=theta_radians, radius_km=radius_km),
    )
    ds.to_netcdf(path=out_fname)

    LOGGER.info(
        "Finished debug dump of %s converted to cylindrical grid in file: %s",
        grib_var_name,
        output_filename,
    )
    # Have to return a float.
    return np.nan


def _obtain_data_at_level(
    grib_dataset: xr.Dataset, grib_var_name: str, level_hPa: Optional[int] = None
) -> np.ndarray:
    if level_hPa is None:
        data = grib_dataset[grib_var_name].values
    else:
        data = grib_dataset[grib_var_name].sel(level=level_hPa).values
    return data


def _convert(data: np.ndarray, unit_converter: Optional[str] = None) -> np.ndarray:
    if unit_converter is not None:
        uc = ce.get_callable_from_import_path(unit_converter)
        data = uc(data)

    return data


def mean_in_radius_range(
    grib_dataset: xr.Dataset,
    cylindrical_grid_interpolator: cg.CylindricalGridInterpolator,
    min_radius_km: float,
    max_radius_km: float,
    grib_var_name: str,
    level_hPa: Optional[int] = None,
    unit_converter: str = None,
    **_kwargs,
) -> float:
    LOGGER.info("Started mean in radius average: %s %s", grib_var_name, str(level_hPa))

    data = _obtain_data_at_level(grib_dataset, grib_var_name, level_hPa)
    data = _convert(data, unit_converter)

    lerp = cylindrical_grid_interpolator
    data_on_grid = lerp(data)
    mean = area_average(data_on_grid, lerp.rad_2d_km, min_radius_km, max_radius_km)
    LOGGER.info("Finished mean in radius average: %s %s", grib_var_name, str(level_hPa))
    return mean


# TODO: Add unit tests, possible move to diag_lib
def area_average(
    data_on_grid: np.ndarray,
    radii_2d_km: np.ndarray,
    min_radius_km: float,
    max_radius_km: float,
) -> float:
    mask = (radii_2d_km >= min_radius_km) & (radii_2d_km <= max_radius_km)
    numerator = np.sum(data_on_grid[mask] * radii_2d_km[mask])
    denominator = np.sum(radii_2d_km[mask])
    return numerator / denominator


def track_row_lookup(
    track_row: pd.DataFrame, column_name: str, convert_to_0_360: bool = False, **_kwargs
):
    LOGGER.info(
        "Started track row lookup: %s convert_to_0_360: %d",
        column_name,
        convert_to_0_360,
    )
    val = track_row[column_name][0]
    if convert_to_0_360:
        val %= 360
    LOGGER.info(
        "Finished track row lookup: %s convert_to_0_360: %d val: %f",
        column_name,
        convert_to_0_360,
        val,
    )
    return val


def shear(
    hour: int,
    results: hr_results.ForecastHourResults,
    u_name: str,
    v_name: str,
    bottom_hPa: float,
    top_hPa: float,
    uv_converter: Optional[Callable[[float], float]] = None,
    **_kwargs,
) -> Tuple[float, float]:
    LOGGER.info(
        "Started shear calculations hour: %d u_name: %s v_name: %s bottom_hPa: %f top_hPa: %f",
        hour,
        u_name,
        v_name,
        bottom_hPa,
        top_hPa,
    )
    u = _shear_component(hour, results, u_name, bottom_hPa, top_hPa)
    v = _shear_component(hour, results, v_name, bottom_hPa, top_hPa)
    if uv_converter is not None:
        u = uv_converter(u)
        v = uv_converter(v)

    r, theta = _u_v_to_r_theta(u, v)
    LOGGER.info(
        "Finished shear calculations hour: %d u_name: %s v_name: %s bottom_hPa: %f top_hPa: %f",
        hour,
        u_name,
        v_name,
        bottom_hPa,
        top_hPa,
    )
    return r, theta


def _shear_component(
    hour: int,
    results: hr_results.ForecastHourResults,
    var_name: str,
    bottom_hPa: float,
    top_hPa: float,
) -> float:
    at_all_levels = results.soundings[var_name].sel(forecast_hour=hour)
    at_bottom = at_all_levels.interp(level_hPa=bottom_hPa)
    at_top = at_all_levels.interp(level_hPa=top_hPa)
    shear_val = at_top - at_bottom
    return shear_val


def _u_v_to_r_theta(u: float, v: float) -> Tuple[float, float]:
    r = np.sqrt(u**2 + v**2)
    if not np.isfinite(r) or r < sys.float_info.epsilon:
        return r, np.nan

    theta = np.degrees(np.arccos(u / r))
    if v < 0:
        theta = 360 - theta

    # Convert to heading
    theta = 90 - theta
    if theta < 0:
        theta += 360

    return r, theta


def temperature_gradient(
    hour: int,
    results: hr_results.ForecastHourResults,
    u_name: str,
    v_name: str,
    bottom_hPa: int,
    top_hPa: int,
    tc_lat: float,
    uv_converter: Optional[Callable[[float], float]] = None,
    **_kwargs,
) -> float:
    bottom_u, top_u = _component_at_levels(results, u_name, hour, bottom_hPa, top_hPa)
    bottom_v, top_v = _component_at_levels(results, v_name, hour, bottom_hPa, top_hPa)

    if uv_converter is not None:
        bottom_u = uv_converter(bottom_u)
        top_u = uv_converter(top_u)
        bottom_v = uv_converter(bottom_v)
        top_v = uv_converter(top_v)

    f = 2.0 * 7.292e-5 * np.sin(tc_lat * 0.017453)
    r = 287.0
    plog = np.log(bottom_hPa / top_hPa)
    cfac = f / (r * plog)

    dtdx = -cfac * (bottom_v - top_v)
    dtdy = cfac * (bottom_u - top_u)
    tgrd = np.sqrt(dtdx**2 + dtdy**2)

    return tgrd


def _component_at_levels(
    results: hr_results.ForecastHourResults,
    var_name: str,
    hour: int,
    bottom_hPa: int,
    top_hPa: int,
) -> Tuple[float, float]:
    bottom = results.soundings[var_name].sel(forecast_hour=hour, level_hPa=bottom_hPa)
    top = results.soundings[var_name].sel(forecast_hour=hour, level_hPa=top_hPa)

    return bottom, top


def always_missing(**_kwargs) -> float:
    """Diag var function that will always return missing."""
    LOGGER.info("Started always_missing.")
    LOGGER.info("Finished always_missing.")
    return np.nan


@dataclasses.dataclass
class LUTExtents:
    """Class that stores distance to land LUT meta-data."""

    ll_lon: float
    ll_lat: float
    ur_lon: float
    ur_lat: float
    nx: int
    ny: int
    lon_spacing: float
    lat_spacing: float

    def width(self):
        return self.ur_lon - self.ll_lon

    def height(self):
        return self.ur_lat - self.ll_lat


class LandLUT:
    """Provides a lookup table of distances to land in km given lon, lat point.

    Bilinear interpolation will be performed on the values from the LUT.  The
    interpolation will be performed across the date-line if necessary.
    """

    UNITS = "km"

    def __init__(self, distances: np.ndarray, extents: LUTExtents):
        self.distances = distances
        self.extents = extents

    def distance(self, lon: float, lat: float) -> float:
        lon = lon % 360

        # Obtain the lower-left grid indices used for the bilinear interpolation
        i = int((lon - self.extents.ll_lon) / self.extents.lon_spacing)
        j = int((lat - self.extents.ll_lat) / self.extents.lat_spacing)

        # The grid cell to the right will be wrapped around the dateline if it
        # extends past the width of the distances grid.
        i_right = i + 1
        wrap_dx = False
        if i >= self.distances.shape[1]:
            i = 0
            wrap_dx = True
        # The upper grid cell will be clamped at the edge if it extents past
        # the height of the distances grid.
        j_up = j + 1
        if j >= self.distances.shape[0]:
            j_up = j

        w_0_0 = self._inverse_distance(lon, i, lat, j)
        w_0_1 = self._inverse_distance(lon, i_right, lat, j, wrap_dx=wrap_dx)
        w_1_0 = self._inverse_distance(lon, i, lat, j_up)
        w_1_1 = self._inverse_distance(lon, i_right, lat, j_up, wrap_dx=wrap_dx)

        weight_total = w_0_0 + w_0_1 + w_1_0 + w_1_1
        w_0_0 /= weight_total
        w_0_1 /= weight_total
        w_1_0 /= weight_total
        w_1_1 /= weight_total

        d_0_0 = self.distances[j, i]
        d_0_1 = self.distances[j, i_right]
        d_1_0 = self.distances[j_up, i]
        d_1_1 = self.distances[j_up, i_right]

        lerp_dist = w_0_0 * d_0_0 + w_0_1 * d_0_1 + w_1_0 * d_1_0 + w_1_1 * d_1_1

        return lerp_dist

    def _inverse_distance(self, lon: float, i: int, lat: float, j: int, wrap_dx=False):
        if wrap_dx:
            dx = lon - (self.extents.ur_lon + self.extents.lon_spacing)
        else:
            dx = lon - (self.extents.lon_spacing * i)

        dy = lat - (self.extents.lat_spacing * j)

        if np.abs(dx) < sys.float_info.epsilon:
            dx = sys.float_info.epsilon

        if np.abs(dy) < sys.float_info.epsilon:
            dy = sys.float_info.epsilon

        return 1 / np.sqrt(dx**2 + dy**2)


def read_land_lut_file(lut_filename: pathlib.Path) -> Tuple[np.ndarray, LUTExtents]:
    """Reads a simple ascii LUT of distances from land on a global grid.

    Args:
        lut_filename (pathlib.Path): The LUT file to read.

    Returns:
        Tuple[np.ndarray, LUTExtents]: Returns an array of distances and the
            LUTExtents containing LUT meta-data.
    """
    with open(lut_filename, "r") as in_file:
        first_line = in_file.readline()
        extents = _get_header_info(first_line)
        data = _read_data_into_array(in_file, extents.nx, extents.ny)

    return data, extents


def _get_header_info(header_line: str) -> LUTExtents:
    header_tokens = header_line.split()
    ll_lon, ur_lon, lon_spacing = map(float, header_tokens[0:3])
    ll_lat, ur_lat, lat_spacing = map(float, header_tokens[4:7])
    nx = int(header_tokens[3])
    ny = int(header_tokens[7])
    extents = LUTExtents(
        ll_lon, ll_lat, ur_lon, ur_lat, nx, ny, lon_spacing, lat_spacing
    )
    return extents


def _read_data_into_array(in_file: TextIO, nx: int, ny: int) -> np.ndarray:
    data = np.zeros((ny, nx), dtype=np.float32)

    j = 0
    i = 0
    for line in in_file:
        line_tokens = line.split()
        tokens_as_float = map(float, line_tokens)
        for data_val in tokens_as_float:
            data[j, i] = data_val
            i += 1
            if i == nx:
                i = 0
                j += 1

    return data


def get_land_lut(land_lut_filename: pathlib.Path) -> LandLUT:
    """Convenience function to generate the distance to land LUT from a file.

    Args:
        land_lut_filename (pathlib.Path): The path to the land LUT file.

    Returns:
        LandLUT: The distance to land LUT.
    """
    distances, extents = read_land_lut_file(land_lut_filename)
    return LandLUT(distances, extents)


def distance_to_land_lookup(
    land_lut: LandLUT,
    hour: int,
    track_row: pd.DataFrame,
    lon_column_name: str = "lon",
    lat_column_name: str = "lat",
    **_kwargs,
) -> float:
    LOGGER.info(
        "Started distance to land lookup for hour: %d using column names: %s, %s.",
        hour,
        lon_column_name,
        lat_column_name,
    )
    lon = track_row_lookup(track_row, lon_column_name, convert_to_0_360=True)
    lat = track_row_lookup(track_row, lat_column_name)

    distance = land_lut.distance(lon, lat)
    LOGGER.info(
        "Finished distance to land lookup: %f for hour: %d using column names: %s, %s.",
        distance,
        hour,
        lon_column_name,
        lat_column_name,
    )
    return distance


def storm_r_theta(
    model_track: pd.DataFrame,
    hour: int,
    model_time: dt.datetime,
    time_delta_hours: int = 6,
    lon_column_name: str = "lon",
    lat_column_name: str = "lat",
    tau_column_name: str = "tau",
    time_column_name: str = "yyyymmddhh",
    **_kwargs,
) -> Tuple[float, float]:
    LOGGER.info(
        "Started storm r, theta calculation hour:%d, model_time:%s, "
        "time_delta_hours:%d, lon_column_name:%s, lat_column_name:%s, "
        "tau_column_name:%s, time_column_name:%s",
        hour,
        model_time,
        time_delta_hours,
        lon_column_name,
        lat_column_name,
        tau_column_name,
        time_column_name,
    )
    mt_rows = model_track.loc[model_track[time_column_name] == model_time]
    min_tau = min(mt_rows[tau_column_name])
    max_tau = max(mt_rows[tau_column_name])

    # Centered difference by default
    lower_offset = -time_delta_hours
    upper_offset = time_delta_hours
    delta_t = time_delta_hours * 2
    # Forward difference if it's the first forecast hour
    if hour == min_tau:
        lower_offset = 0
        upper_offset = time_delta_hours
        delta_t = time_delta_hours
    # Backward difference if it's the last forecast hour
    elif hour == max_tau:
        lower_offset = -time_delta_hours
        upper_offset = 0
        delta_t = time_delta_hours

    lower_row = mt_rows.loc[mt_rows[tau_column_name] == hour + lower_offset].iloc[0]
    upper_row = mt_rows.loc[mt_rows[tau_column_name] == hour + upper_offset].iloc[0]

    lower_lon = lower_row[lon_column_name] % 360
    lower_lat = lower_row[lat_column_name]
    upper_lon = upper_row[lon_column_name] % 360
    upper_lat = upper_row[lat_column_name]

    dlon = upper_lon - lower_lon
    dlat = upper_lat - lower_lat
    avg_lat = (upper_lat + lower_lat) / 2.0
    cfac = np.cos(np.deg2rad(avg_lat))

    v = DEG_TO_NMI * dlat / delta_t
    u = cfac * DEG_TO_NMI * dlon / delta_t
    r, theta = _u_v_to_r_theta(u, v)

    LOGGER.info(
        "Finished storm r:%f, theta:%f calculation hour:%d, model_time:%s, "
        "time_delta_hours:%d, lon_column_name:%s, lat_column_name:%s, "
        "tau_column_name:%s, time_column_name:%s",
        r,
        theta,
        hour,
        model_time,
        time_delta_hours,
        lon_column_name,
        lat_column_name,
        tau_column_name,
        time_column_name,
    )
    return r, theta


def radial_and_tangential_area_average(
    grib_dataset: xr.Dataset,
    cylindrical_grid_interpolator: cg.CylindricalGridInterpolator,
    u_name: str,
    v_name: str,
    level_hPa: int,
    min_radius_km: float,
    max_radius_km: float,
    **_kwargs,
) -> Tuple[float, float]:
    """Computes area average of azimuthally averaged radial & tangential winds."""
    radial_azavg, tan_azavg, radii = _gen_azimuthal_avg_of_radial_tan_winds(
        grib_dataset, cylindrical_grid_interpolator, u_name, v_name, level_hPa
    )

    # Area average of the azimuthal averages
    radial_radius_avg = area_average(radial_azavg, radii, min_radius_km, max_radius_km)
    tan_radius_avg = area_average(tan_azavg, radii, min_radius_km, max_radius_km)

    return radial_radius_avg, tan_radius_avg


def divergence_vorticity(
    grib_dataset: xr.Dataset,
    cylindrical_grid_interpolator: cg.CylindricalGridInterpolator,
    u_name: str,
    v_name: str,
    level_hPa: int,
    radius_km: float,
    **_kwargs,
) -> Tuple[float]:
    LOGGER.info(
        "Started computing divergence and vorticity u_name:%s, v_name:%s, level_hPa:%d, radius_km:%f",
        u_name,
        v_name,
        level_hPa,
        radius_km,
    )
    radial_azavg, tan_azavg, radii = _gen_azimuthal_avg_of_radial_tan_winds(
        grib_dataset, cylindrical_grid_interpolator, u_name, v_name, level_hPa
    )

    divergence, vorticity = _div_vort(radii, radius_km, radial_azavg, tan_azavg)

    LOGGER.info(
        "Finished computing divergence:%f and vorticity:%f u_name:%s, v_name:%s, level_hPa:%d, radius_km:%f",
        divergence,
        vorticity,
        u_name,
        v_name,
        level_hPa,
        radius_km,
    )
    return divergence, vorticity


def _div_vort(
    radii: np.ndarray, radius: float, radial_azavg: np.ndarray, tan_azavg: np.ndarray
) -> Tuple[float, float]:
    closest_radius_km, radius_index = _nearest_radius(radii, radius)
    radius_m = closest_radius_km * 1000

    # Divergence computation
    divergence = 2 * (radial_azavg[radius_index]) / radius_m
    vorticity = 2 * (tan_azavg[radius_index]) / radius_m
    return divergence, vorticity


def _nearest_radius(radii: np.ndarray, desired_radius: float) -> Tuple[float, int]:
    if np.nanmax(radii) < desired_radius:
        raise ValueError(
            f"Desired radius: {desired_radius} larger than max radius: {np.max(radii)}"
        )

    absdiff = np.abs(radii - desired_radius)
    closest_index = np.nanargmin(absdiff)
    closest_value = radii[closest_index]
    return closest_value, closest_index


def _gen_azimuthal_avg_of_radial_tan_winds(
    grib_dataset: xr.Dataset,
    cylindrical_grid_interpolator: cg.CylindricalGridInterpolator,
    u_name: str,
    v_name: str,
    level_hPa: int,
) -> Tuple[np.ndarray, np.ndarray, np.ndarray]:
    """Produces azimuthally averaged radial & tangential winds from grib u,v"""
    # Interpolate u, v to the desired pressure level
    u = grib_dataset[u_name].interp(level=level_hPa).values
    v = grib_dataset[v_name].interp(level=level_hPa).values

    # Interpolate cartesian u, v to cylindrical grid points
    lerp = cylindrical_grid_interpolator
    u_cyl = lerp(u)
    v_cyl = lerp(v)

    radial_azavg, tan_azavg = _rad_tan_azavg(u_cyl, v_cyl, lerp.theta_2d_radians, 0)

    # Get the 1D array of radii corresponding to the azimuthally averaged data.
    radii = lerp.rad_2d_km[0, :]

    return radial_azavg, tan_azavg, radii


def _rad_tan_azavg(
    u_cyl: np.ndarray, v_cyl: np.ndarray, theta_2d_radians: np.ndarray, avg_axis: int
) -> Tuple[np.ndarray, np.ndarray]:
    # Convert to radial and tangential wind
    radial = u_cyl * np.cos(theta_2d_radians) + v_cyl * np.sin(theta_2d_radians)
    tangential = -u_cyl * np.sin(theta_2d_radians) + v_cyl * np.cos(theta_2d_radians)

    # Compute the azimuthal averages
    radial_azavg = cg.azimuthal_average(radial, axis=avg_axis)
    tan_azavg = cg.azimuthal_average(tangential, axis=avg_axis)

    return radial_azavg, tan_azavg


def average_rmw(
    grib_dataset: xr.Dataset,
    cylindrical_grid_interpolator: cg.CylindricalGridInterpolator,
    u_surface_name: str,
    v_surface_name: str,
    radius_km: float,
    **_kwargs,
) -> float:
    LOGGER.info(
        "Starting calculation of average rmw u_surface_name:%s "
        "v_surface_name:%s radius_km:%f",
        u_surface_name,
        v_surface_name,
        radius_km,
    )
    u = grib_dataset[u_surface_name].values
    v = grib_dataset[v_surface_name].values

    lerp = cylindrical_grid_interpolator
    u_cyl = lerp(u)
    v_cyl = lerp(v)

    avg_rmw_km = post_cyl_avg_rmw(u_cyl, v_cyl, radius_km, lerp.rad_2d_km[0, :])

    LOGGER.info(
        "Finished calculation of average rmw:%f u_surface_name:%s "
        "v_surface_name:%s radius_km:%f",
        avg_rmw_km,
        u_surface_name,
        v_surface_name,
        radius_km,
    )
    return avg_rmw_km


def post_cyl_avg_rmw(
    u_cyl: np.ndarray, v_cyl: np.ndarray, radius_km: float, radii_1d_km: np.ndarray
) -> float:
    speed = np.sqrt(u_cyl**2 + v_cyl**2)

    # Figure out the index of the largest radius <= radius_km
    selected_i = 0
    for i, rad in enumerate(radii_1d_km):
        if rad > radius_km:
            break

        selected_i = i

    # Slice the speed array so that it no longer includes data for radii > radius_km
    speed = speed[:, 0 : selected_i + 1]

    # Get the radius of the max speed at each azimuth
    max_wind_indices = np.nanargmax(speed, axis=1)
    rmw_at_each_azimuth = radii_1d_km[max_wind_indices]

    avg_rmw_km = np.nanmean(rmw_at_each_azimuth)
    return avg_rmw_km
