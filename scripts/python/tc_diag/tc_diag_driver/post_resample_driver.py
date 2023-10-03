import argparse
import dataclasses
import datetime as dt
import inspect
import io
import pathlib
from typing import Any, Dict, List, Tuple

import numpy as np
import pandas as pd
import xarray as xr
import yaml
from atcf_tools import track_tools

from tc_diag_driver import computation_engine as ce
from tc_diag_driver import diag_vars
from tc_diag_driver import results as fcresults

LEVEL_EPSILON = 0.01
LEAD_TIME_TO_HRS = 10000
ATCF_DELIM_CHAR = ","
ATCF_TECH_ID_COL = 4

@dataclasses.dataclass
class DriverConfig:
    pressure_independent_computation_specs: List[Dict[str, Any]]
    sounding_computation_specs: List[Dict[str, Any]]

    land_lut_file: pathlib.Path
    in_forecast_time_name: str
    in_levels_name: str
    in_radii_name: str
    in_azimuth_name: str
    lon_input_name: str
    lat_input_name: str
    init_time_name: str
    init_time_format: str
    full_track_line_name: str

    def __post_init__(self):
        self.land_lut_file = pathlib.Path(self.land_lut_file)


def main():
    args = _get_args()

    config = config_from_file(args.config_file, DriverConfig)

    results = diag_calcs(
        config, args.data_file, suppress_exceptions=args.suppress_exceptions)
    if args.out_dir is not None:
        _dump_results(results, args.out_dir)


def config_from_file(filename: pathlib.Path, config_class: Any) -> Any:
    with open(filename, "r") as in_file:
        spec_dict = yaml.safe_load(in_file)

    sig = inspect.signature(config_class.__init__)
    args = [p.name for p in sig.parameters.values()]
    filtered = {k: spec_dict[k] for k in spec_dict.keys() if k in args}

    spec = config_class(**filtered)
    return spec


def _get_args() -> argparse.Namespace:
    parser = argparse.ArgumentParser(
        description=
        "Driver to perform diag computations from model data resampled to a cylindrical grid."
    )
    parser.add_argument(
        "config_file",
        type=pathlib.Path,
        help="YAML config file specifying how to process diag vars.")
    parser.add_argument(
        "data_file",
        type=pathlib.Path,
        help="NetCDF file containing model data resampled to cylindrical grid."
    )
    parser.add_argument(
        "-o",
        "--out_dir",
        type=pathlib.Path,
        default=None,
        help="Optional directory to write results to for debugging purposes.")
    parser.add_argument("-s", "--suppress_exceptions", action="store_true",
                        default=False, help="If this flag is set, then "
                        "exceptions encountered during diagnostic computations "
                        "will be logged and then ignored.")

    return parser.parse_args()


def populate_missing_results(
        config: DriverConfig, forecast_hour: int,
        levels_hPa: List[int]) -> fcresults.ForecastHourResults:
    _batches, results = _prep_diag_calculations(config, forecast_hour,
                                                levels_hPa)
    return results


def diag_calcs(
        config: DriverConfig,
        data_path: pathlib.Path,
        suppress_exceptions: bool = False) -> fcresults.ForecastHourResults:
    # Gather various data necessary to perform diagnostic calculations
    input_data = xr.load_dataset(data_path, engine="netcdf4")
    forecast_hour = _get_forecast_hour(config, input_data)
    levels_hPa = _get_pressure_levels(config, input_data)
    init_time = _get_init_time(config, input_data)
    land_lut = diag_vars.get_land_lut(config.land_lut_file)
    radii_1d = input_data[config.in_radii_name]
    azimuth_1d = input_data[config.in_azimuth_name]
    theta_2d, radii_2d = np.meshgrid(azimuth_1d, radii_1d)
    atcf_tech_id = _parse_atcf_id(input_data[config.full_track_line_name])
    track = _dataset_track_lines_to_track(
        input_data[config.full_track_line_name], atcf_tech_id)

    lon = input_data[config.lon_input_name][0]
    lat = input_data[config.lat_input_name][0]

    batches, results = _prep_diag_calculations(config, forecast_hour,
                                               levels_hPa)

    call_args = {
        "input_data": input_data,
        "lon": lon,
        "lat": lat,
        "radii_1d": radii_1d,
        "azimuth_1d": azimuth_1d,
        "radii_2d": radii_2d,
        "theta_2d": theta_2d,
        "land_lut": land_lut,
        "forecast_hour": forecast_hour,
        "init_time": init_time,
        "track": track,
        "results": results
    }

    for batch in batches:
        batch.add_to_results(
            results,
            call_args,
            forecast_hour,
            levels_hPa,
            suppress_computation_exceptions=suppress_exceptions)

    return results


def _prep_diag_calculations(
    config: DriverConfig, forecast_hour: int, levels_hPa: List[int]
) -> Tuple[List[ce.ComputationBatch], fcresults.ForecastHourResults]:
    pi_comps = ce.diag_computations_from_entry(
        config.pressure_independent_computation_specs)
    snd_comps = ce.diag_computations_from_entry(
        config.sounding_computation_specs)

    pi_result_names, snd_result_names = ce.get_all_result_names(
        pi_comps, snd_comps)
    results = fcresults.ForecastHourResults([forecast_hour], levels_hPa,
                                            pi_result_names, snd_result_names)
    batches = ce.get_computation_batches(pi_comps, snd_comps)

    return batches, results


def _dump_results(results: fcresults.ForecastHourResults,
                  out_dir: pathlib.Path) -> None:
    sounding_filename = out_dir / "sounding.nc"
    pressue_independent_filename = out_dir / "pressure_independent.nc"

    results.soundings.to_netcdf(sounding_filename)
    results.pressure_independent.to_netcdf(pressue_independent_filename)


def _get_forecast_hour(config: DriverConfig, input_data: xr.Dataset) -> int:
    return int(input_data[config.in_forecast_time_name][0]) // LEAD_TIME_TO_HRS


def _get_pressure_levels(config: DriverConfig,
                         input_data: xr.Dataset) -> List[int]:
    levels = input_data[config.in_levels_name]
    return [round(float(level + LEVEL_EPSILON)) for level in levels]


def _get_init_time(config: DriverConfig,
                   input_data: xr.Dataset) -> dt.datetime:
    init_time_var = input_data[config.init_time_name]
    init_time_str = str(init_time_var.values)

    return dt.datetime.strptime(init_time_str, config.init_time_format)


def _dataset_track_lines_to_track(track_lines: xr.DataArray,
                                  atcf_tech_id: str) -> pd.DataFrame:
    lines = []
    for line in track_lines:
        lines.append(str(line.values))
    lines_str = "\n".join(lines)
    line_buffer = io.StringIO(lines_str)
    track = track_tools.get_adeck_track(line_buffer, atcf_tech_id)
    return track

def _parse_atcf_id(track_lines: xr.DataArray) -> str:
    first_line = str(track_lines[0].values)
    split_line = first_line.split(ATCF_DELIM_CHAR)
    atcf_id = split_line[ATCF_TECH_ID_COL].strip()
    return atcf_id

if __name__ == "__main__":
    main()