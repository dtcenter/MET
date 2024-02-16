import dataclasses
import importlib
import logging
from typing import Any, Callable, Dict, List, Optional, Tuple, Union

import numpy as np

from tc_diag_driver import results as hr_results

LOGGER = logging.getLogger(__name__)
LOGGER.addHandler(logging.NullHandler())

PASS_FUNC_PATH = "pass"

ConverterFunc = Optional[Callable[[float], float]]


@dataclasses.dataclass
class DiagComputation:
    name: str
    computation: Callable
    batch_order: int = 0
    kwargs: Optional[Dict[str, Any]] = None
    output_vars: Optional[List[str]] = None
    units: Optional[List[str]] = None
    unit_converters: Optional[List[ConverterFunc]] = None

    def __post_init__(self):
        self._normalize_output_vars()
        self._normalize_units()
        self._validate_units_length()
        self._normalize_unit_converters()
        self._validate_unit_converter_length()
        self._normalize_kwargs()

    def _normalize_output_vars(self) -> None:
        if self.output_vars is None:
            self.output_vars = [self.name]

    def _normalize_units(self) -> None:
        if self.units is None:
            self.units = [None] * len(self.output_vars)
        elif isinstance(self.units, str):
            self.units = [self.units]

    def _validate_units_length(self) -> None:
        if len(self.output_vars) != len(self.units):
            raise ValueError(
                f"Error in DiagComputation: {self.name}, number of units doesn't "
                f"match number of output vars. Output: {self.output_vars} "
                f"Units: {self.units}"
            )

    def _normalize_unit_converters(self) -> None:
        if self.unit_converters is None:
            self.unit_converters = [None] * len(self.output_vars)

    def _validate_unit_converter_length(self) -> None:
        if len(self.unit_converters) != len(self.units):
            raise ValueError(
                f"Error in DiagComputation: {self.name}, number of unit converters "
                f"doesn't match number of output vars. Output: {self.output_vars} "
                f"Converters: {self.unit_converters}"
            )

    def _normalize_kwargs(self) -> None:
        if self.kwargs is None:
            self.kwargs = {}

    def get_missing_result(self) -> List[float]:
        return [np.NaN] * len(self.output_vars)


@dataclasses.dataclass
class ComputationBatch:
    order: int
    pressure_independent: List[DiagComputation]
    sounding: List[DiagComputation]

    def add_to_results(
        self,
        results: hr_results.ForecastHourResults,
        call_args: Dict[str, Any],
        hour: int,
        levels_hPa: List[int],
        suppress_computation_exceptions=False,
    ) -> None:
        self._compute_pressure_independent_vars(
            hour, results, call_args, suppress_computation_exceptions
        )
        self._compute_sounding_vars(
            levels_hPa, hour, results, call_args, suppress_computation_exceptions
        )

    def _compute_pressure_independent_vars(
        self,
        forecast_hour: int,
        results: hr_results.ForecastHourResults,
        call_args: Dict[str, Any],
        suppress_exceptions: bool,
    ) -> None:
        for comp in self.pressure_independent:
            self._compute_result(
                comp, results, forecast_hour, call_args, suppress_exceptions
            )

    def _compute_sounding_vars(
        self,
        levels_hPa: int,
        forecast_hour: int,
        results: hr_results.ForecastHourResults,
        call_args: Dict[str, Any],
        suppress_exceptions: bool,
    ) -> None:
        for level_hPa in levels_hPa:
            for comp in self.sounding:
                self._compute_result(
                    comp,
                    results,
                    forecast_hour,
                    call_args,
                    suppress_exceptions,
                    level_hPa,
                )

    def _compute_result(
        self,
        comp: DiagComputation,
        results: hr_results.ForecastHourResults,
        forecast_hour: int,
        call_args: Dict[str, Any],
        suppress_exceptions: bool,
        level_hPa: Optional[int] = None,
    ) -> None:
        all_args = {**call_args, **comp.kwargs}
        if level_hPa is not None:
            all_args["level_hPa"] = level_hPa

        # Result will either be a float or a Tuple of values if a
        # computation produces multiple results at the same time.
        result_tuple = None
        try:
            result_tuple = comp.computation(**all_args)
        except KeyboardInterrupt:
            raise
        except Exception as ex:
            LOGGER.info(
                "Encountered exception while running computation: %s at hour: %d",
                comp.name,
                forecast_hour,
            )
            if not suppress_exceptions:
                msg = (
                    f"Encountered error while running computation: "
                    f"{comp.name} at hour: {forecast_hour}"
                )
                raise Exception(msg).with_traceback(ex.__traceback__)

        if result_tuple is None:
            result_tuple = comp.get_missing_result()
        else:
            result_tuple = self._normalize_result_tuple(result_tuple)
            self._validate_result_tuple_length(result_tuple, comp)
            result_tuple = self._apply_unit_conversions(result_tuple, comp)

        for result_name, value, units in zip(
            comp.output_vars, result_tuple, comp.units
        ):
            if level_hPa is None:
                results.add_pressure_independent_result(
                    result_name, forecast_hour, value, units=units
                )
            else:
                results.add_sounding_result(
                    result_name, forecast_hour, level_hPa, value, units=units
                )

    def _normalize_result_tuple(self, result_tuple: List[float]) -> List[float]:
        try:
            len(result_tuple)
        except TypeError:
            result_tuple = [result_tuple]

        return result_tuple

    def _validate_result_tuple_length(
        self, result_tuple: List[float], comp: DiagComputation
    ) -> None:
        if len(result_tuple) != len(comp.output_vars):
            raise ValueError(
                f"Received {len(result_tuple)} results, "
                f"expected: {len(comp.output_vars)} for computation: {comp}"
            )

    def _apply_unit_conversions(
        self, result_tuple: List[float], comp: DiagComputation
    ) -> List[float]:
        if comp.unit_converters is None:
            return result_tuple

        if len(result_tuple) != len(comp.unit_converters):
            raise ValueError(
                f"Received {len(result_tuple)} results, and "
                f"{len(comp.unit_converters)} unit converters.  Expected an "
                f"equal amount."
            )

        new_tuple = []
        for result, converter in zip(result_tuple, comp.unit_converters):
            if converter is None:
                new_tuple.append(result)
            else:
                converted = converter(result)
                new_tuple.append(converted)

        return new_tuple


def diag_computations_from_entry(config_entry: Dict[str, Any]) -> List[DiagComputation]:
    computations = []
    for var_name, spec in config_entry.items():
        batch_order = spec.get("batch_order", 0)
        to_call = get_callable_from_import_path(spec["callable"])
        kwargs = spec.get("kwargs", None)
        output_vars = spec.get("output_vars", None)
        units = spec.get("units", None)

        unit_converter_paths = spec.get("unit_converters", None)
        converter_funcs = converter_funcs_from_paths(unit_converter_paths)

        comp = DiagComputation(
            var_name, to_call, batch_order, kwargs, output_vars, units, converter_funcs
        )
        computations.append(comp)

    return computations


def get_all_result_names(
    pressure_indedpendent: List[DiagComputation], sounding: List[DiagComputation]
) -> Tuple[List[str], List[str]]:
    pi_var_names = get_result_names(pressure_indedpendent)
    snd_var_names = get_result_names(sounding)
    return pi_var_names, snd_var_names


def get_result_names(computations: List[DiagComputation]) -> List[str]:
    names = []
    for c in computations:
        names.extend(c.output_vars)
    return names


def get_all_result_units(
    pressure_indedpendent: List[DiagComputation], sounding: List[DiagComputation]
) -> Tuple[List[str], List[str]]:
    pi_var_units = get_result_units(pressure_indedpendent)
    snd_var_units = get_result_units(sounding)
    return pi_var_units, snd_var_units


def get_result_units(computations: List[DiagComputation]) -> List[str]:
    units = []
    for c in computations:
        units.extend(c.units)
    return units


def get_computation_batches(
    pressure_indedpendent: List[DiagComputation], sounding: List[DiagComputation]
) -> List[ComputationBatch]:
    batch_dict = {}
    type_pi = 0
    type_snd = 1
    _add_comps_to_dict(batch_dict, type_pi, pressure_indedpendent)
    _add_comps_to_dict(batch_dict, type_snd, sounding)

    batch_indices = sorted(batch_dict.keys())
    batches = [
        ComputationBatch(i, batch_dict[i][type_pi], batch_dict[i][type_snd])
        for i in batch_indices
    ]
    return batches


def _add_comps_to_dict(
    batch_dict: Dict[int, DiagComputation], comp_type: int, comps: List[DiagComputation]
) -> None:
    for c in comps:
        group = batch_dict.setdefault(c.batch_order, ([], []))
        group[comp_type].append(c)


def get_callable_from_import_path(
    import_path_or_callable: Union[str, Callable]
) -> Callable:
    if hasattr(import_path_or_callable, "__call__"):
        return import_path_or_callable

    p = import_path_or_callable.split(".")
    module_name = ".".join(p[:-1])
    callable_name = p[-1]

    mod = importlib.import_module(module_name)
    to_call = getattr(mod, callable_name)
    return to_call


def converter_funcs_from_paths(
    paths: Optional[List[str]] = None,
) -> List[ConverterFunc]:
    if paths is None:
        return None

    if isinstance(paths, str):
        paths = [paths]

    converter_funcs = []
    for func_path in paths:
        if func_path.lower().strip() == PASS_FUNC_PATH:
            converter_funcs.append(None)
        else:
            func = get_callable_from_import_path(func_path)
            converter_funcs.append(func)

    return converter_funcs
