import datetime as dt
import io
from typing import Any, Callable, List, Optional, TextIO, Union

import numpy as np
import pandas as pd

BDECK_DATE_FORMAT = "%Y%m%d%H"
DEFAULT_SEPARATOR = " *, *"


class ColumnDefinition:
    """Simple class to store information needed to parse a column.
    """
    def __init__(
            self,
            name: str,
            parse_function: Optional[Callable[[str], Any]] = None) -> None:
        self.name = name.strip().lower()
        self.parse_function = parse_function


def date_parse(text: str, date_format=BDECK_DATE_FORMAT) -> dt.datetime:
    return dt.datetime.strptime(text.strip(), date_format)


def minute_parse(text: str) -> int:
    stripped = text.strip()
    if len(stripped) == 0:
        return int(0)

    return int(stripped)


def text2int(text: str) -> int:
    if isinstance(text, str):
        text = text.strip()
    try:
        return int(text)
    except (AttributeError, ValueError, TypeError):
        return text


def parse_coord(text: str) -> float:
    try:
        text = text.strip().lower()
        coordinate = 0.1 * int(text.strip("nesw"))
        if text.endswith(("s", "w")):
            coordinate *= -1
        return coordinate
    except (AttributeError, ValueError):
        return np.NaN


def get_dataframe(filepath_or_buffer: Union[str, TextIO],
                  column_definitions: List[ColumnDefinition],
                  separator: str = DEFAULT_SEPARATOR) -> pd.DataFrame:
    column_names = []
    converters = {}
    for definition in column_definitions:
        column_names.append(definition.name)
        if definition.parse_function is not None:
            converters[definition.name] = definition.parse_function

    df = pd.read_csv(filepath_or_buffer,
                     header=None,
                     names=column_names,
                     converters=converters,
                     sep=separator,
                     engine="python",
                     index_col=False)

    return df


def get_filtered_buffer(filepath_or_buffer: Union[str, TextIO],
                        sub_string: str) -> io.StringIO:
    if hasattr(filepath_or_buffer, "read"):
        filtered_file = _filter(filepath_or_buffer, sub_string)
    else:
        with open(filepath_or_buffer, "r") as in_file:
            filtered_file = _filter(in_file, sub_string)

    return filtered_file


def _filter(buffer: TextIO, sub_string: str) -> io.StringIO:
    filtered_file = io.StringIO()
    for line in buffer:
        if sub_string in line:
            filtered_file.write(line)

    filtered_file.seek(0)
    return filtered_file
