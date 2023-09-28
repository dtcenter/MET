import datetime as dt
from typing import List, Optional, TextIO, Tuple, Union

import pandas as pd

from atcf_tools import parsing

CARQ_TECH_ID = "CARQ"

EXTRAPOLATION_EPOCH = dt.datetime(1970, 1, 1)

BDECK_COLUMN_DEFINITIONS = [
    parsing.ColumnDefinition("basin"),
    parsing.ColumnDefinition("cy", parsing.text2int),
    parsing.ColumnDefinition("yyyymmddhh", parsing.date_parse),
    parsing.ColumnDefinition("min", parsing.minute_parse),
    parsing.ColumnDefinition("tech"),
    parsing.ColumnDefinition("tau", parsing.text2int),
    parsing.ColumnDefinition("lat", parsing.parse_coord),
    parsing.ColumnDefinition("lon", parsing.parse_coord),
    parsing.ColumnDefinition("vmax", parsing.text2int),
    parsing.ColumnDefinition("mslp", parsing.text2int),
    parsing.ColumnDefinition("ty"),
    parsing.ColumnDefinition("rad", parsing.text2int),
    parsing.ColumnDefinition("windcode"),
    parsing.ColumnDefinition("rad1", parsing.text2int),
    parsing.ColumnDefinition("rad2", parsing.text2int),
    parsing.ColumnDefinition("rad3", parsing.text2int),
    parsing.ColumnDefinition("rad4", parsing.text2int),
    parsing.ColumnDefinition("radp", parsing.text2int),
    parsing.ColumnDefinition("rrp", parsing.text2int),
    parsing.ColumnDefinition("mrd", parsing.text2int),
    parsing.ColumnDefinition("gusts", parsing.text2int),
    parsing.ColumnDefinition("eye", parsing.text2int),
    parsing.ColumnDefinition("subregion"),
    parsing.ColumnDefinition("maxseas", parsing.text2int),
    parsing.ColumnDefinition("initials"),
    parsing.ColumnDefinition("dir", parsing.text2int),
    parsing.ColumnDefinition("speed", parsing.text2int),
    parsing.ColumnDefinition("stormname"),
    parsing.ColumnDefinition("depth"),
    parsing.ColumnDefinition("seas"),
    parsing.ColumnDefinition("seascode"),
    parsing.ColumnDefinition("seas1", parsing.text2int),
    parsing.ColumnDefinition("seas2", parsing.text2int),
    parsing.ColumnDefinition("seas3", parsing.text2int),
    parsing.ColumnDefinition("seas4", parsing.text2int),
    parsing.ColumnDefinition("userdefined")
]

ADECK_COLUMN_DEFINITIONS = list(BDECK_COLUMN_DEFINITIONS)
ADECK_TECHNUM_COLUMN = 3
ADECK_COLUMN_DEFINITIONS[ADECK_TECHNUM_COLUMN] = parsing.ColumnDefinition(
    "technum", parsing.text2int)


def add_bdeck_time_index(df: pd.DataFrame) -> None:
    times = []
    for base_time, minute in zip(df["yyyymmddhh"], df["min"]):
        new_time = base_time + dt.timedelta(minutes=minute)
        times.append(new_time)

    df["time"] = times
    df.set_index("time", inplace=True)


def add_carq_time_index(df: pd.DataFrame) -> None:
    df["time"] = df["yyyymmddhh"]
    df.set_index("time", inplace=True)


def make_lon_continuous(df: pd.DataFrame) -> None:
    lons = df["lon"]
    # Ensure 0-360 convention
    lons %= 360

    # Assumes that a track will never be longer than 180 degrees. If a
    # discontinuity is detected, it shifts the lons to -180 - 180 convention to
    # push the discontinuity to the other side of the planet.
    if max(abs(lons - lons[0])) > 180:
        lons[lons > 180] -= 360


def get_positions(
    df: pd.DataFrame,
    times: Optional[Union[dt.datetime, List[dt.datetime]]] = None
) -> Union[Tuple[float, float], Tuple[List[float], List[float]]]:
    if times is None:
        lons = df["lon"]
        lats = df["lat"]
    else:
        lons = df["lon"][times]
        lats = df["lat"][times]

    # Return the lons in 0-360 convention.
    lons = lons % 360
    return lons, lats


def remove_duplicate_times(df: pd.DataFrame) -> pd.DataFrame:
    return df[~df.index.duplicated(keep="first")]


def get_best_track(filepath_or_buffer: Union[str, TextIO]):
    df = parsing.get_dataframe(filepath_or_buffer, BDECK_COLUMN_DEFINITIONS)
    add_bdeck_time_index(df)
    return df


def get_carq_track(filepath_or_buffer: Union[str, TextIO]):
    return get_adeck_track(filepath_or_buffer, CARQ_TECH_ID)


def get_adeck_track(filepath_or_buffer: Union[str, TextIO], tech_id: str):
    filtered_buffer = parsing.get_filtered_buffer(filepath_or_buffer, tech_id)
    df = parsing.get_dataframe(filtered_buffer, ADECK_COLUMN_DEFINITIONS)
    add_carq_time_index(df)
    return df


def extrapolate(track: pd.DataFrame, column: str, t1: dt.datetime,
                t2: dt.datetime, target: dt.datetime) -> float:
    t1_value = track.loc[t1, column]
    t1_seconds = (t1 - EXTRAPOLATION_EPOCH).total_seconds()

    t2_value = track.loc[t2, column]
    t2_seconds = (t2 - EXTRAPOLATION_EPOCH).total_seconds()

    target_seconds = (target - EXTRAPOLATION_EPOCH).total_seconds()

    return t1_value + ((target_seconds - t1_seconds) /
                       (t2_seconds - t1_seconds)) * (t2_value - t1_value)


def extrapolate_lon(track: pd.DataFrame, t1: dt.datetime, t2: dt.datetime,
                    target: dt.datetime) -> float:
    # Just extrapolate and handle wrapping the lon.  Assumes that the lons have
    # already been made continuous.
    lon = extrapolate(track, "lon", t1, t2, target)
    lon %= 360
    if lon > 180:
        lon -= 360

    return lon


def sanitize_lons_for_cartopy(lons: List[float]) -> List[float]:
    new_list = []
    oldval = 0
    threshold = 180  # used to compare adjacent longitudes
    for index, lon in enumerate(lons):
        lon = lon % 360
        diff = oldval - lon
        if index > 0 and diff > threshold:
            lon = lon + 360
        oldval = lon
        new_list.append(lon)
    return new_list