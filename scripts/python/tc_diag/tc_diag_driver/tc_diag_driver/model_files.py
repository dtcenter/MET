"""Contains code to handle model filenames."""
import dataclasses
import datetime as dt
import inspect
import pathlib
from typing import Any, Dict, List

import yaml


@dataclasses.dataclass
class ModelSpec:
    model_name: str
    path_format: str
    resolution: str
    nested_grid_id: str
    levels_hPa: List[int]
    forecast_hours: List[int]
    # Need to specify which grib variable will be used to obtain the lon/lat
    # grid
    nav_var_name: str
    nav_var_level: int
    nav_var_level_type: str
    nav_var_is_surface: bool
    # Cylindrical grid specification
    n_radii: int
    n_theta: int
    radii_step_km: int
    # ATCF file tech-ID to use to get the TC lon/lat
    atcf_tech_id: str
    output_file_format: str
    # Specification entries for what to read from the grib file and how to read
    # them.
    input_var_specs: List[Dict[str, Any]]
    # Specification entries for what and how to compute the pressure independent
    # variables.
    pressure_independent_computation_specs: List[Dict[str, Any]]
    # Specification entries for what and how to compute the sounding independent
    # variables.
    sounding_computation_specs: List[Dict[str, Any]]
    # Specification entries for what and how to output computed variables.
    output_specs: List[Dict[str, Any]]

    def make_model_path(self, forecast_time_hours: int,
                        model_time: dt.datetime) -> pathlib.Path:
        path_str = self.path_format.format(
            forecast_time_hours=forecast_time_hours,
            model_name=self.model_name,
            model_time=model_time,
            resolution=self.resolution,
            nested_grid_id=self.nested_grid_id)
        return pathlib.Path(path_str)


def model_spec_from_file(filename: pathlib.Path) -> ModelSpec:
    with open(filename, "r") as in_file:
        spec_dict = yaml.safe_load(in_file)

    # Filter the arguments used to construct the ModelSpec.
    # We want to remove entries in the dictionary that don't correspond to
    # constructor arguments. This allows additonal entries in the config to
    # be used without being passed to the ModelSpec constructor. It can be
    # very useful to add config entries that are yaml anchors/aliases.
    sig = inspect.signature(ModelSpec.__init__)
    args = [p.name for p in sig.parameters.values()]
    filtered = {k: spec_dict[k] for k in spec_dict.keys() if k in args}

    spec = ModelSpec(**filtered)
    return spec
