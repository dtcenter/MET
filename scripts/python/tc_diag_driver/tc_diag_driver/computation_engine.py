import dataclasses
import importlib
import logging
from typing import Any, Callable, Dict, List, Optional, Tuple, Union

from tc_diag_driver import results as hr_results

LOGGER = logging.getLogger(__name__)
LOGGER.addHandler(logging.NullHandler())


@dataclasses.dataclass
class DiagComputation:
    name: str
    computation: Callable
    batch_order: int = 0
    kwargs: Optional[Dict[str, Any]] = None
    output_vars: Optional[List[str]] = None
    units: Optional[List[str]] = None

    def __post_init__(self):
        if self.output_vars is None:
            self.output_vars = [self.name]

        if self.units is None:
            self.units = [None] * len(self.output_vars)
        elif isinstance(self.units, str):
            self.units = [self.units]

        if len(self.output_vars) != len(self.units):
            raise ValueError(
                f"Error in DiagComputation: {self.name}, number of units don't "
                f"match number of output vars. Output: {self.output_vars} "
                f"Units: {self.units}")

        if self.kwargs is None:
            self.kwargs = {}


@dataclasses.dataclass
class ComputationBatch:
    order: int
    pressure_independent: List[DiagComputation]
    sounding: List[DiagComputation]

    def add_to_results(self,
                       results: hr_results.ForecastHourResults,
                       call_args: Dict[str, Any],
                       hour: int,
                       levels_hPa: List[int],
                       suppress_computation_exceptions=False) -> None:
        self._compute_pressure_independent_vars(
            hour, results, call_args, suppress_computation_exceptions)
        self._compute_sounding_vars(levels_hPa, hour, results, call_args,
                                    suppress_computation_exceptions)

    def _compute_pressure_independent_vars(
            self, forecast_hour: int, results: hr_results.ForecastHourResults,
            call_args: Dict[str, Any], suppress_exceptions: bool) -> None:
        for comp in self.pressure_independent:
            self._compute_result(comp, results, forecast_hour, call_args,
                                 suppress_exceptions)

    def _compute_sounding_vars(self, levels_hPa: int, forecast_hour: int,
                               results: hr_results.ForecastHourResults,
                               call_args: Dict[str, Any],
                               suppress_exceptions: bool) -> None:
        for level_hPa in levels_hPa:
            for comp in self.sounding:
                self._compute_result(comp, results, forecast_hour, call_args,
                                     suppress_exceptions, level_hPa)

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
        try:
            result_tuple = comp.computation(**all_args)
        except KeyboardInterrupt:
            raise
        except:
            LOGGER.info(
                "Encountered exception while running computation: %s at hour: %d with args: %s",
                comp, forecast_hour, call_args)
            if not suppress_exceptions:
                raise

        try:
            len(result_tuple)
        except TypeError:
            result_tuple = [result_tuple]

        if len(result_tuple) != len(comp.output_vars):
            raise ValueError(
                f"Received {len(result_tuple)} results, "
                f"expected: {len(comp.output_vars)} for computation: {comp}")

        for result_name, value, units in zip(comp.output_vars, result_tuple,
                                             comp.units):
            if level_hPa is None:
                results.add_pressure_independent_result(result_name,
                                                        forecast_hour,
                                                        value,
                                                        units=units)
            else:
                results.add_sounding_result(result_name,
                                            forecast_hour,
                                            level_hPa,
                                            value,
                                            units=units)


def diag_computations_from_entry(
        config_entry: Dict[str, Any]) -> List[DiagComputation]:
    computations = []
    for var_name, spec in config_entry.items():
        batch_order = spec.get("batch_order", 0)
        to_call = get_callable_from_import_path(spec["callable"])
        kwargs = spec.get("kwargs", None)
        output_vars = spec.get("output_vars", None)
        units = spec.get("units", None)
        comp = DiagComputation(var_name, to_call, batch_order, kwargs,
                               output_vars, units)
        computations.append(comp)

    return computations


def get_all_result_names(
        pressure_indedpendent: List[DiagComputation],
        sounding: List[DiagComputation]) -> Tuple[List[str], List[str]]:
    pi_var_names = get_result_names(pressure_indedpendent)
    snd_var_names = get_result_names(sounding)
    return pi_var_names, snd_var_names


def get_result_names(computations: List[DiagComputation]) -> List[str]:
    names = []
    for c in computations:
        names.extend(c.output_vars)
    return names


def get_computation_batches(
        pressure_indedpendent: List[DiagComputation],
        sounding: List[DiagComputation]) -> List[ComputationBatch]:
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


def _add_comps_to_dict(batch_dict: Dict[int, DiagComputation], comp_type: int,
                       comps: List[DiagComputation]) -> None:
    for c in comps:
        group = batch_dict.setdefault(c.batch_order, ([], []))
        group[comp_type].append(c)


def get_callable_from_import_path(
        import_path_or_callable: Union[str, Callable]) -> Callable:
    if hasattr(import_path_or_callable, "__call__"):
        return import_path_or_callable

    p = import_path_or_callable.split(".")
    module_name = ".".join(p[:-1])
    callable_name = p[-1]

    mod = importlib.import_module(module_name)
    to_call = getattr(mod, callable_name)
    return to_call
