"""Routines to help find overpasses."""
import dataclasses
import datetime as dt
from typing import Tuple

import numpy as np
import pandas as pd

EARTH_RADIUS = 6371


def filter_track_by_time_range(track: pd.DataFrame, start_time: dt.datetime,
                               end_time: dt.datetime) -> pd.DataFrame:
    indices = np.logical_and(track.index >= start_time,
                             track.index <= end_time)
    return track.loc[indices]


@dataclasses.dataclass(frozen=True)
class OverpassPoint:
    time: dt.datetime
    lon: float
    lat: float


@dataclasses.dataclass(frozen=True)
class Overpass:
    distance_km: float
    track_a_point: OverpassPoint
    track_b_point: OverpassPoint


def find_overpass(track_a: pd.DataFrame,
                  track_b: pd.DataFrame,
                  start_time: dt.datetime,
                  end_time: dt.datetime,
                  a_lon_name="lon",
                  b_lon_name="lon",
                  a_lat_name="lat",
                  b_lat_name="lat"):
    filtered_a = filter_track_by_time_range(track_a, start_time, end_time)
    filtered_b = filter_track_by_time_range(track_b, start_time, end_time)

    if len(filtered_a) <= 0 or len(filtered_b) <= 0:
        return None

    min_distance_km, min_a_index, min_b_index = _find_min_distance_indices(
        filtered_a[a_lon_name], filtered_a[a_lat_name], filtered_b[b_lon_name],
        filtered_b[b_lat_name])

    track_a_point = _get_overpass_point_at_index(filtered_a, min_a_index,
                                                 a_lon_name, a_lat_name)
    track_b_point = _get_overpass_point_at_index(filtered_b, min_b_index,
                                                 b_lon_name, b_lat_name)
    overpass = Overpass(min_distance_km, track_a_point, track_b_point)
    return overpass


def _find_min_distance_indices(lons_a: np.ndarray, lats_a: np.ndarray,
                               lons_b: np.ndarray,
                               lats_b: np.ndarray) -> Tuple[float, int, int]:
    min_distance_km = None
    min_a_index = None
    min_b_index = None
    for i, lon, lat in zip(range(len(lons_a)), lons_a, lats_a):
        distances_km = haversine_distance_km(lon, lat, lons_b, lats_b)
        track_b_index = np.argmin(distances_km)
        distance_km = distances_km[track_b_index]

        if min_distance_km is None or distance_km < min_distance_km:
            min_distance_km = distance_km
            min_a_index = i
            min_b_index = track_b_index

    return min_distance_km, min_a_index, min_b_index


def _get_overpass_point_at_index(
        filtered: pd.DataFrame, index: int, lon_name: str,
        lat_name: str) -> Tuple[dt.datetime, float, float]:
    time = filtered.index[index]
    lon = filtered[lon_name].iloc[index]
    lat = filtered[lat_name].iloc[index]
    point = OverpassPoint(time, lon, lat)
    return point


def haversine_distance_km(lons_a: np.ndarray, lats_a: np.ndarray,
                          lons_b: np.ndarray,
                          lats_b: np.ndarray) -> np.ndarray:
    lons_a = lons_a % 360
    lons_b = lons_b % 360

    d_lat = np.radians(lats_b - lats_a)
    d_lon = np.radians(lons_b - lons_a)
    a = np.sin(d_lat / 2.0) * np.sin(d_lat / 2) + \
        np.cos(np.radians(lats_a)) * \
        np.cos(np.radians(lats_b)) * np.sin(d_lon / 2.0) * np.sin(d_lon / 2.0)
    c = 2 * np.arctan2(np.sqrt(a), np.sqrt(1 - a))
    d = EARTH_RADIUS * c
    return d
