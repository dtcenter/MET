import datetime as dt
from typing import List, Tuple

import numpy as np
import pandas as pd

from atcf_tools import track_tools


def positions_at_times(
        track: pd.DataFrame,
        times: List[dt.datetime],
        method="pchip",
        union_times: bool = False) -> Tuple[pd.Series, pd.Series]:
    lons, lats = track_tools.get_positions(track)
    lons = track_tools.remove_duplicate_times(lons)
    lats = track_tools.remove_duplicate_times(lats)
    interp_lats = lats.copy()

    sin_lons = np.sin(np.radians(lons))
    cos_lons = np.cos(np.radians(lons))

    for time in times:
        if time not in lons.index:
            sin_lons.loc[time] = np.nan
            cos_lons.loc[time] = np.nan
            interp_lats.loc[time] = np.nan

    sin_lons = sin_lons.sort_index()
    cos_lons = cos_lons.sort_index()
    interp_lats = interp_lats.sort_index()

    sin_lons.interpolate(method=method, inplace=True)
    cos_lons.interpolate(method=method, inplace=True)
    interp_lats.interpolate(method=method, inplace=True)

    interp_lons = np.degrees(np.arctan2(sin_lons, cos_lons))

    if not union_times:
        # pylint: disable=no-member
        interp_lons = interp_lons.loc[times]
        interp_lats = interp_lats.loc[times]

    return interp_lons, interp_lats


def field_at_times(track: pd.DataFrame,
                   field_name: str,
                   times: List[dt.datetime],
                   method="linear",
                   union_times: bool = False) -> pd.Series:
    column = track[field_name].copy(deep=True)
    column = track_tools.remove_duplicate_times(column)

    for time in times:
        if time not in column.index:
            column.loc[time] = np.nan

    column.sort_index(inplace=True)
    column.interpolate(method=method, inplace=True)

    if not union_times:
        column = column.loc[times]

    return column
