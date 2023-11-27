import dataclasses
import datetime as dt
import pathlib
from cmath import isnan
from typing import List, Optional, TextIO, Union

import numpy as np
import xarray as xr

from tc_diag_driver import results as hr_results

STORM_HEADER = "                ------------------------------------------------------     STORM DATA     ----------------------------------------------------------\n"
SOUNDING_HEADER = "                ------------------------------------------------------     SOUNDING DATA     -------------------------------------------------------\n"
CUSTOM_HEADER = "                ------------------------------------------------------     CUSTOM DATA     ---------------------------------------------------------\n"
PRESS_FMT = "04d"
NLEV_FMT = "03d"
INT_VALUE_COLUMN_FMT = "5d"
FLOAT_VALUE_COLUMN_FMT = "5.1f"
VAR_NAME_FMT = "<8s"
VAR_UNIT_FMT = "<9s"

DIAG_MISSING_VALUE = 9999

DIAG_HEADER1_FMT = "               *   {model_id}  {model_time:%Y%m%d%H}   *\n"
DIAG_HEADER2_FMT = "               *   {basin}{storm_number:02d}  {storm_name}       *\n"

STORM_TIME_HEADER_FMT = "NTIME {n_hours:03d}   DELTAT {delta_t:03d}\n"
CUSTOM_N_VARS_HEADER_FMT = "NVAR {n_vars:03d}\n"

OUTPUT_TYPES = ("sounding", "surface", "storm", "custom")


@dataclasses.dataclass
class DiagVarOutputSpec:
    var_name: str
    units: str
    output_type: str
    scale_factor: float = 1
    output_float: bool = False

    def __post_init__(self):
        self.output_type = self.output_type.strip().lower()
        if self.output_type not in OUTPUT_TYPES:
            raise ValueError(f"Output type must be one of {OUTPUT_TYPES}, "
                             f"received:{self.output_type}")


@dataclasses.dataclass
class DiagHeaderInfo:
    model_id: str
    model_time: dt.datetime
    basin: str
    storm_number: int
    storm_name: str

    def as_header_lines(self) -> List[str]:
        line1 = DIAG_HEADER1_FMT.format(model_id=self.model_id.upper(),
                                        model_time=self.model_time)
        line2 = DIAG_HEADER2_FMT.format(basin=self.basin.upper(),
                                        storm_number=self.storm_number,
                                        storm_name=self.storm_name.upper())
        return [line1, line2]


def diag_filename(filename_format: str, output_dir: pathlib.Path,
                  model_time: dt.datetime, atcf_id: str,
                  atcf_tech_id: str) -> pathlib.Path:
    basename = filename_format.format(model_time=model_time,
                                      atcf_id=atcf_id,
                                      atcf_tech_id=atcf_tech_id).lower()
    outpath = output_dir / basename
    return outpath


def to_diag_file(filepath_or_buffer: Union[pathlib.Path, TextIO],
                 results: hr_results.ForecastHourResults,
                 output_var_specs: List[DiagVarOutputSpec],
                 diag_header_info: DiagHeaderInfo,
                 missing_value: int = DIAG_MISSING_VALUE) -> None:
    if hasattr(filepath_or_buffer, "write"):
        _write_to_diag_buffer(filepath_or_buffer, results, output_var_specs,
                              diag_header_info, missing_value)
    else:
        with open(filepath_or_buffer, "w") as out_file:
            _write_to_diag_buffer(out_file, results, output_var_specs,
                                  diag_header_info, missing_value)


def _write_to_diag_buffer(out_buffer: TextIO,
                          results: hr_results.ForecastHourResults,
                          output_var_specs: List[DiagVarOutputSpec],
                          diag_header_info: DiagHeaderInfo,
                          missing_value: int) -> None:
    header_lines = diag_header_info.as_header_lines()
    out_buffer.writelines(header_lines)
    out_buffer.write("\n")

    storm_lines = _gen_storm_lines(results, output_var_specs, missing_value)
    out_buffer.writelines(storm_lines)
    out_buffer.write("\n")

    sounding_lines = _gen_sounding_lines(results, output_var_specs,
                                         missing_value)
    out_buffer.writelines(sounding_lines)
    out_buffer.write("\n")

    custom_lines = _gen_custom_lines(results, output_var_specs, missing_value)
    out_buffer.writelines(custom_lines)


def _gen_storm_lines(results: hr_results.ForecastHourResults,
                     output_var_specs: List[DiagVarOutputSpec],
                     missing_value: int) -> List[str]:
    storm_time_header = _storm_time_header(results.pressure_independent)

    sorted_hours = _sorted_hours(results.pressure_independent)
    hours_header = _hours_header(sorted_hours)

    storm_lines = [STORM_HEADER, "\n", storm_time_header, hours_header]
    storm_var_specs = _get_var_specs_of_type(output_var_specs, "storm")

    storm_var_rows = _pressure_independent_rows(results.pressure_independent,
                                                storm_var_specs, sorted_hours,
                                                missing_value)
    storm_lines.extend(storm_var_rows)
    return storm_lines


def _gen_sounding_lines(results: hr_results.ForecastHourResults,
                        output_var_specs: List[DiagVarOutputSpec],
                        missing_value: int) -> List[str]:
    sorted_levels = _sorted_levels(results.soundings)
    levels_header = _levels_header(sorted_levels)

    sorted_hours = _sorted_hours(results.soundings)
    hours_header = _hours_header(sorted_hours)

    sounding_lines = [SOUNDING_HEADER, "\n", levels_header, hours_header]

    surface_var_specs = _get_var_specs_of_type(output_var_specs, "surface")
    surface_rows = _pressure_independent_rows(results.pressure_independent,
                                              surface_var_specs, sorted_hours,
                                              missing_value)
    sounding_lines.extend(surface_rows)

    sounding_var_specs = _get_var_specs_of_type(output_var_specs, "sounding")
    sounding_rows = _sounding_rows(results.soundings, sounding_var_specs,
                                   sorted_hours, sorted_levels, missing_value)
    sounding_lines.extend(sounding_rows)

    return sounding_lines


def _gen_custom_lines(results: hr_results.ForecastHourResults,
                      output_var_specs: List[DiagVarOutputSpec],
                      missing_value: int) -> List[str]:
    custom_var_specs = _get_var_specs_of_type(output_var_specs, "custom")
    n_vars = len(custom_var_specs)
    n_vars_header = CUSTOM_N_VARS_HEADER_FMT.format(n_vars=n_vars)
    custom_lines = [CUSTOM_HEADER, "\n", n_vars_header]

    sorted_hours = _sorted_hours(results.soundings)

    custom_var_lines = _pressure_independent_rows(results.pressure_independent,
                                                  custom_var_specs,
                                                  sorted_hours, missing_value)
    custom_lines.extend(custom_var_lines)
    return custom_lines


def _get_var_specs_of_type(output_var_specs: List[DiagVarOutputSpec],
                           output_type: str) -> List[str]:
    return [s for s in output_var_specs if s.output_type == output_type]


def _sorted_levels(soundings: xr.Dataset) -> List[int]:
    # Need levels from lowest in atmosphere to highest
    return np.sort(soundings.coords["level_hPa"].data)[::-1]


def _sorted_hours(dataset: xr.Dataset) -> List[int]:
    return np.sort(dataset.coords["forecast_hour"].data)


def _storm_time_header(dataset: xr.Dataset) -> str:
    hours = dataset.coords["forecast_hour"].data
    td = hours[1] - hours[0]
    n_hours = len(hours)
    return STORM_TIME_HEADER_FMT.format(n_hours=n_hours, delta_t=td)


def _levels_header(sorted_levels: List[int]) -> str:
    n_levels = len(sorted_levels) + 1
    levels_strings = [f"{lev:{PRESS_FMT}}" for lev in sorted_levels]
    joined_levels = " ".join(levels_strings)
    levels_header = f"NLEV {n_levels:{NLEV_FMT}} SURF {joined_levels}\n"
    return levels_header


def _hours_header(hours: List[int]) -> str:
    hours_strings = [f"{h:{INT_VALUE_COLUMN_FMT}}" for h in hours]
    joined_hours = " ".join(hours_strings)
    hours_header = f"TIME    (HR)     {joined_hours}\n"
    return hours_header


def _pressure_independent_rows(dataset: xr.Dataset,
                               var_specs: List[DiagVarOutputSpec],
                               sorted_hours: np.ndarray,
                               missing_value: int) -> List[str]:
    rows = []
    for var_spec in var_specs:
        row = _format_var_row(var_spec, dataset, sorted_hours, missing_value)
        rows.append(row)
    return rows


def _format_var_row(var_spec: DiagVarOutputSpec,
                    dataset: xr.Dataset,
                    sorted_hours: np.ndarray,
                    missing_value: int,
                    level: Optional[int] = None) -> List[str]:
    var_strings = []
    for hour in sorted_hours:
        if level is None:
            val = dataset[var_spec.var_name].loc[hour]
        else:
            val = dataset[var_spec.var_name].loc[hour, level]

        if isnan(val):
            out_val = missing_value
        else:
            out_val = val * var_spec.scale_factor

        if var_spec.output_float:
            column_format = FLOAT_VALUE_COLUMN_FMT
            out_val = float(out_val)
        else:
            out_val = int(out_val)
            column_format = INT_VALUE_COLUMN_FMT

        out_str = f"{out_val:{column_format}}"
        var_strings.append(out_str)
    joined_vals = " ".join(var_strings)

    if level is None:
        out_name = f"{var_spec.var_name.upper()}"
    else:
        out_name = f"{var_spec.var_name.upper()}_{level:{PRESS_FMT}}"
    units_str = f"({var_spec.units.upper()})"
    row = f"{out_name:{VAR_NAME_FMT}}{units_str:{VAR_UNIT_FMT}}{joined_vals}\n"
    return row


def _sounding_rows(soundings: xr.Dataset,
                   sounding_var_specs: List[DiagVarOutputSpec],
                   sorted_hours: np.ndarray, sorted_levels: np.ndarray,
                   missing_value: int) -> List[str]:
    rows = []
    for level in sorted_levels:
        for var_spec in sounding_var_specs:
            row = _format_var_row(var_spec, soundings, sorted_hours,
                                  missing_value, level)
            rows.append(row)

    return rows
