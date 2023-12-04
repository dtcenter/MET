import argparse
import dataclasses
import datetime as dt
import logging
import pathlib
import sys
from typing import Any, Dict, List, Tuple

import numpy as np
import pandas as pd
import yaml
from atcf_tools import filenames as atcf_id_parse
from atcf_tools import track_tools
from diag_lib import cylindrical_grid as cg

import tc_diag_driver
from tc_diag_driver import computation_engine as ce
from tc_diag_driver import diag_vars, grib, model_files, output
from tc_diag_driver import results as hr_results

LOGGER = logging.getLogger(__name__)
LOGGER.addHandler(logging.NullHandler())
DEFAULT_LOG_FORMAT = "%(asctime)s;%(levelname)s;%(name)s;%(lineno)d;%(message)s"

INVEST = "INVEST"


@dataclasses.dataclass
class ModelEntry:
    model_spec: model_files.ModelSpec
    atcf_id: str
    atcf_filename: pathlib.Path
    model_time: dt.datetime
    output_dir: pathlib.Path


def main():
    LOGGER.info("******Started execution with version: %s******",
                tc_diag_driver.__version__)
    args = _get_args()
    entries = model_entries_from_file(args.model_entry_file)
    # The land LUT only needs to be read once and shared among all processing
    # So we'll read it now and pass it to the lower-level code.
    land_lut = diag_vars.get_land_lut(args.land_lut_file)
    run(entries, land_lut)
    LOGGER.info("******Finished execution with version: %s******",
                tc_diag_driver.__version__)


def _get_args():
    parser = argparse.ArgumentParser(
        description="Driver to generate diagnostic variables.")
    parser.add_argument(
        "model_entry_file",
        type=pathlib.Path,
        help="YAML file containing a list of model times to process.")
    parser.add_argument(
        "land_lut_file",
        type=pathlib.Path,
        help="A file containing a lookup table of distances to land.")
    args = parser.parse_args()
    return args


def model_entries_from_file(filename: pathlib.Path) -> List[ModelEntry]:
    with open(filename, "r") as in_file:
        entry_specs = yaml.safe_load(in_file)["model_entries"]

    entries = []
    for entry_spec in entry_specs:
        model_spec_filename = pathlib.Path(entry_spec["model_spec"])
        model_spec = model_files.model_spec_from_file(model_spec_filename)

        atcf_filename = pathlib.Path(entry_spec["atcf_file"])
        output_dir = pathlib.Path(entry_spec["output_dir"])

        entry = ModelEntry(model_spec, entry_spec["atcf_id"], atcf_filename,
                           entry_spec["model_time"], output_dir)
        entries.append(entry)
    return entries


def run(model_entries: List[ModelEntry],
        land_lut: diag_vars.LandLUT,
        suppress_exceptions=False) -> None:
    for entry in model_entries:
        try:
            model_spec = entry.model_spec

            input_vars = specs_from_config(model_spec.input_var_specs,
                                           grib.InputVarSpec)

            pi_comps = ce.diag_computations_from_entry(
                model_spec.pressure_independent_computation_specs)
            snd_comps = ce.diag_computations_from_entry(
                model_spec.sounding_computation_specs)

            var_output_specs = specs_from_config(model_spec.output_specs,
                                                 output.DiagVarOutputSpec)

            track = track_tools.get_adeck_track(entry.atcf_filename,
                                                model_spec.atcf_tech_id)

            results = process_model_entry(
                entry,
                land_lut,
                track,
                input_vars,
                pi_comps,
                snd_comps,
                suppress_exceptions=suppress_exceptions)
            _write_results(entry, model_spec, results, var_output_specs, track)
        except KeyboardInterrupt:
            raise
        except:
            LOGGER.exception(f"Encountered exception processing entry:{entry}")
            if not suppress_exceptions:
                raise


def specs_from_config(config_entry: List[Dict[str, Any]],
                      spec_class: type) -> List[Any]:
    specs = [spec_class(**spec_args) for spec_args in config_entry]
    return specs


def _write_results(entry: ModelEntry, model_spec: model_files.ModelSpec,
                   results: hr_results.ForecastHourResults,
                   var_output_specs: List[output.DiagVarOutputSpec],
                   track: pd.DataFrame) -> None:
    output_filename = output.diag_filename(model_spec.output_file_format,
                                           entry.output_dir, entry.model_time,
                                           entry.atcf_id,
                                           model_spec.atcf_tech_id)

    basin, storm_num = _get_atcf_info(entry.atcf_id)
    storm_name = _get_storm_name(track, basin, storm_num)
    header_info = output.DiagHeaderInfo(model_spec.atcf_tech_id,
                                        entry.model_time, basin, storm_num,
                                        storm_name)

    output.to_diag_file(output_filename, results, var_output_specs,
                        header_info)


#TODO: Might want to move this to atcf_tools
def _get_atcf_info(atcf_id: str) -> Tuple[str, int]:
    normalized = atcf_id.strip().lower()
    m = atcf_id_parse.ATCF_ID_REGEX.match(normalized)
    if m is None:
        raise ValueError(f"Can not parse atcf id:{atcf_id}")

    gd = m.groupdict()
    basin = gd["basin"]
    storm_num = int(gd["storm_number"])

    return basin, storm_num


#TODO: Might want to move this to atcf_tools
def _get_storm_name(track: pd.DataFrame, basin: str, storm_num: int) -> str:
    fallback_name = f"{basin.strip().upper()}{storm_num:02d}"
    name_col = track["stormname"]
    not_nan = ~np.isnan(name_col)
    if not np.any(not_nan):
        return fallback_name

    mask = not_nan & (~name_col == INVEST)
    if not np.any(mask):
        return INVEST

    rows_with_name = name_col[mask]
    if len(rows_with_name <= 0):
        LOGGER.warning(
            "Given atcf file with storm name column that has columens that are"\
            " not nan, INVEST, or storm name."
        )
        return fallback_name

    return rows_with_name[0]


def setup_logging(level: str, log_format=DEFAULT_LOG_FORMAT):
    levels = {
        "debug": logging.DEBUG,
        "info": logging.INFO,
        "warning": logging.WARNING,
        "error": logging.WARNING,
        "critical": logging.CRITICAL
    }
    selected_level = levels[level.lower().strip()]

    console_handler = logging.StreamHandler(sys.stdout)

    logging.basicConfig(level=selected_level,
                        format=log_format,
                        handlers=[console_handler])


def process_model_entry(
        entry: ModelEntry,
        land_lut: diag_vars.LandLUT,
        track: pd.DataFrame,
        input_vars: List[grib.InputVarSpec],
        pressure_independent_computations: List[ce.DiagComputation],
        sounding_computations: List[ce.DiagComputation],
        suppress_exceptions=False) -> hr_results.ForecastHourResults:
    """Computes diag variables for all forecast times at the given model time.

    This is the primary routine that orchestrates the diagnostic calculations
    for a given model entry.  This iterates over all of the forecast hours
    defined in the model specification and finds the corresponding grib files
    for the model time and forecast hour.  A set of data that can be used to
    perform diagnostic variable calculations is generated.  This data is then
    given to all of the provided callables in order to generate the diagnostic
    varible results. The results are stored in an instance of
    ForecastHourResults and returned.
    """
    LOGGER.info("Started processing entry:%s", entry)
    model_spec = entry.model_spec
    pi_result_names, snd_result_names = ce.get_all_result_names(
        pressure_independent_computations, sounding_computations)
    results = hr_results.ForecastHourResults(model_spec.forecast_hours,
                                             model_spec.levels_hPa,
                                             pi_result_names, snd_result_names)
    batches = ce.get_computation_batches(pressure_independent_computations,
                                         sounding_computations)

    for hour in model_spec.forecast_hours:
        try:
            model_path = model_spec.make_model_path(hour, entry.model_time)
            LOGGER.info("Processing hour:%d with path:%s", hour, model_path)

            if not model_path.exists():
                LOGGER.warning("Could not find file: %s", model_path)
                continue

            nav_spec = grib.InputVarSpec(
                model_spec.nav_var_name,
                model_spec.nav_var_level_type,
                is_surface=model_spec.nav_var_is_surface,
                level=model_spec.nav_var_level)
            # Get data needed to perform diag computations.
            grib_ds, grib_lons, grib_lats = grib.grib_to_dataset(
                model_path, nav_spec, model_spec.levels_hPa, input_vars)

            try:
                track_row = get_forecast_row(track, entry.model_time, hour)
                tc_lon, tc_lat = get_tc_location_from_row(track_row)
            except IndexError:
                LOGGER.exception("No tc location for: %s", hour)
                continue

            grib_distances_from_tc_km = distances_from_tc(
                grib_lons, grib_lats, tc_lon, tc_lat)

            lerp = cg.BilinearInterpolator(model_spec.n_radii,
                                           model_spec.n_theta,
                                           model_spec.radii_step_km, tc_lon,
                                           tc_lat, grib_lons, grib_lats)

            # Pack data into a dictionary. Any of the dictionary items can
            # optionally be used as a parameter in a diag computation.
            call_args = dict(
                grib_dataset=grib_ds,
                cylindrical_grid_interpolator=lerp,
                land_lut=land_lut,
                model_track=track,
                track_row=track_row,
                grib_distances_from_tc_km=grib_distances_from_tc_km,
                tc_lon=tc_lon,
                tc_lat=tc_lat,
                model_spec=model_spec,
                hour=hour,
                grib_lons=grib_lons,
                grib_lats=grib_lats,
                results=results,
                model_time=entry.model_time)

            for batch in batches:
                batch.add_to_results(
                    results,
                    call_args,
                    hour,
                    model_spec.levels_hPa,
                    suppress_computation_exceptions=suppress_exceptions)

            LOGGER.info("Finished processing hour:%d with path:%s", hour,
                        model_path)
        except KeyboardInterrupt:
            raise
        except:
            if suppress_exceptions:
                LOGGER.exception(f"Encountered error processing model_time:"
                                 f"{entry.model_time} hour:{hour}")
            else:
                raise
    LOGGER.info("Finished processing entry:%s", entry)
    return results


def get_forecast_row(track: pd.DataFrame, model_time: dt.datetime,
                     hour: int) -> pd.DataFrame:
    row_idx = ((track["yyyymmddhh"] == model_time) & (track["tau"] == hour))
    row = track.loc[row_idx]
    return row


def get_tc_location_from_row(row: pd.DataFrame) -> Tuple[float, float]:
    return row["lon"][0] % 360, row["lat"][0]


def distances_from_tc(grib_lons: np.ndarray, grib_lats: np.ndarray,
                      tc_lon: float, tc_lat: float) -> np.ndarray:
    grid_x_km, grid_y_km = cg.convert_grid_to_tc_centric_km(
        grib_lons, grib_lats, tc_lon, tc_lat)
    distances = np.sqrt(grid_x_km**2 + grid_y_km**2)
    return distances


if __name__ == "__main__":
    setup_logging("info")
    main()
