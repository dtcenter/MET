import glob
import pathlib
from typing import List
import datetime as dt

import pandas as pd
import numpy as np

from atcf_tools import track_tools


def get_active_tracks(glob_string: str,
                      search_time: dt.datetime,
                      search_reverse_hours: int,
                      search_recursive=False) -> List[pd.DataFrame]:
    if search_reverse_hours < 0:
        raise ValueError(
            f"search_reverse_hours must be > 0, given: {search_reverse_hours}")

    start_time = search_time - dt.timedelta(hours=search_reverse_hours)

    filenames = glob.glob(glob_string, recursive=search_recursive)
    active_tracks = []
    for filename in filenames:
        adeck_track = track_tools.get_carq_track(filename)
        if track_has_times_in_range(adeck_track, start_time, search_time):
            adeck_track["atcf_filename"] = filename
            active_tracks.append(adeck_track)

    return active_tracks


def track_has_times_in_range(track: pd.DataFrame, start_time: dt.timedelta,
                             end_time: dt.timedelta) -> bool:
    if end_time < start_time:
        raise ValueError(
            f"End time must be after start time given start: {start_time} end: {end_time}"
        )

    valid_indices = (track.index >= start_time) & (track.index <= end_time)
    is_track_active = np.any(valid_indices)
    return is_track_active
