import datetime as dt
from typing import Any, Dict, Optional, Tuple

import numpy as np
import pandas as pd
import xarray as xr

from tc_diag_driver import diag_vars, met_post_process
from tc_diag_driver import results as fcresults

SHEAR_THETA_UNITS = "degrees"

STORM_R_UNITS = "kt"
STORM_THETA_UNITS = "degrees"

DIV_VORT_SCALE = 10000000
DIV_VORT_UNITS = "/s"

TGRD_SCALE = 10000000
TGRD_UNITS = "10^7c/m"


def kwarg_lookup(lookup_name: str, **kwargs: Dict[str, Any]) -> float:
    return kwargs[lookup_name]


def dataset_lookup(
    input_data: xr.Dataset, var_name: str, **_kwargs: Dict[str, Any]
) -> float:
    var = input_data[var_name][0]
    return var


def _with_units(data: float, units: str) -> xr.DataArray:
    return xr.DataArray(data=data, attrs={"units": units})


def average_rmw(
    input_data: xr.Dataset,
    radii_1d: xr.DataArray,
    u_surface_name: str,
    v_surface_name: str,
    radius_km: float,
    **_kwargs: Dict[str, Any]
) -> xr.DataArray:
    usurf = input_data[u_surface_name][0]
    vsurf = input_data[v_surface_name][0]

    # The function expects usurf and vsurf to be azimuth x radii, but the NC
    # files use the opposite convention, so transposing the arrays is necessary.
    avg_rmw = diag_vars.post_cyl_avg_rmw(
        usurf.data.T, vsurf.data.T, radius_km, radii_1d.data
    )
    units = radii_1d.attrs["units"]
    da_avg_rmw = _with_units(avg_rmw, units)
    return da_avg_rmw


def area_average(
    input_data: xr.Dataset,
    radii_2d: np.ndarray,
    min_radius_km: float,
    max_radius_km: float,
    var_name: str,
    level_hPa: Optional[int] = None,
    levels_dimen: Optional[str] = None,
    **_kwargs: Dict[str, Any]
) -> xr.DataArray:
    if level_hPa is None:
        var = input_data[var_name][0]
    else:
        var = input_data[var_name].sel(**{levels_dimen: level_hPa})[0]

    avg = diag_vars.area_average(var.data, radii_2d, min_radius_km, max_radius_km)
    units = var.attrs["units"]
    da_avg = _with_units(avg, units)
    return da_avg


def distance_to_land_lookup(
    lon: float, lat: float, land_lut: diag_vars.LandLUT, **_kwargs: Dict[str, Any]
) -> xr.DataArray:
    distance_km = land_lut.distance(lon, lat)
    units = land_lut.UNITS
    da_distance = _with_units(distance_km, units)
    return da_distance


def storm_r_theta(
    track: pd.DataFrame,
    forecast_hour: int,
    init_time: dt.datetime,
    **_kwargs: Dict[str, Any]
) -> Tuple[xr.DataArray, xr.DataArray]:
    storm_r, storm_theta = diag_vars.storm_r_theta(track, forecast_hour, init_time)
    da_r = _with_units(storm_r, STORM_R_UNITS)
    da_theta = _with_units(storm_theta, STORM_THETA_UNITS)
    return da_r, da_theta


def radial_and_tangential_area_average(
    input_data: xr.Dataset,
    radii_1d: xr.DataArray,
    theta_2d: np.ndarray,
    u_name: str,
    v_name: str,
    level_hPa: int,
    min_radius_km: float,
    max_radius_km: float,
    **_kwargs: Dict[str, Any]
) -> Tuple[xr.DataArray, xr.DataArray]:
    u = input_data[u_name]
    v = input_data[v_name]
    rad_azavg, tan_azavg = _rad_tan(level_hPa, u, v, theta_2d)

    radial_avg = diag_vars.area_average(
        rad_azavg, radii_1d.values, min_radius_km, max_radius_km
    )
    tan_avg = diag_vars.area_average(
        tan_azavg, radii_1d.values, min_radius_km, max_radius_km
    )
    units = u.attrs["units"]
    da_radial = _with_units(radial_avg, units)
    da_tan = _with_units(tan_avg, units)

    return da_radial, da_tan


def _rad_tan(
    level_hPa: int, u: xr.DataArray, v: xr.DataArray, theta_2d: np.ndarray
) -> Tuple[np.ndarray, np.ndarray]:
    u_cyl = u.interp(pressure=level_hPa).values[0, :, :]
    v_cyl = v.interp(pressure=level_hPa).values[0, :, :]

    rad_azavg, tan_azavg = diag_vars._rad_tan_azavg(u_cyl, v_cyl, theta_2d, 1)
    return rad_azavg, tan_azavg


def div_vort(
    input_data: xr.Dataset,
    level_hPa: int,
    u_name: xr.DataArray,
    v_name: xr.DataArray,
    theta_2d: np.ndarray,
    radii_1d: xr.DataArray,
    radius_km: float,
    **_kwargs: Dict[str, Any]
) -> Tuple[xr.DataArray, xr.DataArray]:
    u = input_data[u_name]
    v = input_data[v_name]
    rad_azavg, tan_azavg = _rad_tan(level_hPa, u, v, theta_2d)
    divergence, vorticity = diag_vars._div_vort(
        radii_1d, radius_km, rad_azavg, tan_azavg
    )

    divergence *= DIV_VORT_SCALE
    vorticity *= DIV_VORT_SCALE

    da_div = _with_units(divergence, DIV_VORT_UNITS)
    da_vort = _with_units(vorticity, DIV_VORT_UNITS)

    return da_div, da_vort


def shear(
    forecast_hour: int,
    results: fcresults.ForecastHourResults,
    u_name: str,
    v_name: str,
    bottom_hPa: int,
    top_hPa: int,
    should_convert_uv_from_10kt_to_mps=True,
    **_kwargs: Dict[str, Any]
) -> Tuple[xr.DataArray, xr.DataArray]:
    converter = None
    if should_convert_uv_from_10kt_to_mps:
        converter = met_post_process.ten_kt_to_mps
    shear_r, shear_theta = diag_vars.shear(
        forecast_hour,
        results,
        u_name,
        v_name,
        bottom_hPa,
        top_hPa,
        uv_converter=converter,
    )
    r_units = results.soundings[u_name].attrs["units"]

    da_r = _with_units(shear_r, r_units)
    da_theta = _with_units(shear_theta, SHEAR_THETA_UNITS)

    return da_r, da_theta


def temperature_gradient(
    forecast_hour: int,
    results: fcresults.ForecastHourResults,
    u_name: str,
    v_name: str,
    bottom_hPa: int,
    top_hPa: int,
    lat: float,
    should_convert_uv_from_10kt_to_mps=True,
    **_kwargs: Dict[str, Any]
) -> xr.DataArray:
    converter = None
    if should_convert_uv_from_10kt_to_mps:
        converter = met_post_process.ten_kt_to_mps

    grad = diag_vars.temperature_gradient(
        forecast_hour,
        results,
        u_name,
        v_name,
        bottom_hPa,
        top_hPa,
        lat,
        uv_converter=converter,
    )
    grad *= TGRD_SCALE

    da_grad = _with_units(grad, TGRD_UNITS)
    return da_grad
