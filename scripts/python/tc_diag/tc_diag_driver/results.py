from typing import Dict, List, Optional, Tuple, Union

import numpy as np
import xarray as xr


class ForecastHourResults:
    """Stores diag var results for a given forecast hour."""

    def __init__(
        self,
        forecast_hours: List[int],
        levels_hPa: List[int],
        pressure_independent_var_names: List[str],
        sounding_var_names: List[str],
        pressure_independent_units: Optional[List[str]] = None,
        sounding_units: Optional[List[str]] = None,
    ):
        self.forecast_hours = forecast_hours
        self.levels_hPa = levels_hPa

        soundings_shape = (len(forecast_hours), len(levels_hPa))
        soundings_coords = {"forecast_hour": forecast_hours, "level_hPa": levels_hPa}
        self.soundings = self._init_dataset(
            soundings_shape,
            soundings_coords,
            ("forecast_hour", "level_hPa"),
            sounding_var_names,
        )

        # All the other variables just have forecast hour as the coordinates
        # For convenience
        hour_results_shape = len(forecast_hours)
        hour_results_coords = {"forecast_hour": forecast_hours}

        self.pressure_independent = self._init_dataset(
            hour_results_shape,
            hour_results_coords,
            ("forecast_hour"),
            pressure_independent_var_names,
        )

        if pressure_independent_units is not None:
            self._add_predefined_units(
                self.pressure_independent,
                pressure_independent_var_names,
                pressure_independent_units,
            )

        if sounding_units is not None:
            self._add_predefined_units(
                self.soundings, sounding_var_names, sounding_units
            )

    def _add_predefined_units(
        self, ds: xr.Dataset, names: List[str], units: List[str]
    ) -> None:
        if len(names) != len(units):
            raise ValueError(
                f"Length of names: {len(names)} does not match units: {len(units)}"
            )

        for name, unit in zip(names, units):
            if unit is None:
                continue

            ds[name].attrs["units"] = unit

    def _init_dataset(
        self,
        shape: Tuple[int],
        coords: Dict["str", List[int]],
        dims: List[str],
        var_names: List[str],
    ) -> xr.Dataset:
        data_arrays = {}
        for var_name in var_names:
            empty_array = np.full(shape, np.nan)
            da = xr.DataArray(empty_array, name=var_name, dims=dims, coords=coords)
            data_arrays[var_name] = da

        return xr.Dataset(data_vars=data_arrays, coords=coords)

    def add_pressure_independent_result(
        self,
        var_name: str,
        hour: int,
        result: Union[float, xr.DataArray],
        units: Optional[str] = None,
    ) -> None:
        da = self.pressure_independent[var_name]
        self._add_units(da, result, units)

        da.loc[hour] = result

    def _should_add_units(
        self,
        array: xr.DataArray,
        result: Union[float, xr.DataArray],
        provided_units: Optional[str],
    ) -> bool:
        if not hasattr(array, "attrs"):
            return False

        if "units" in array.attrs:
            return False

        if provided_units is not None:
            return True

        if not hasattr(result, "attrs"):
            return False

        if "units" not in result.attrs:
            return False

        return True

    def add_sounding_result(
        self,
        var_name: str,
        hour: int,
        level_hPa: int,
        result: float,
        units: Optional[str] = None,
    ) -> None:
        da = self.soundings[var_name]

        self._add_units(da, result, units)

        da.loc[hour, level_hPa] = result

    def _add_units(
        self,
        array: xr.DataArray,
        result: Union[float, xr.DataArray],
        units: Optional[str],
    ):
        if self._should_add_units(array, result, units):
            if units is not None:
                array.attrs["units"] = units
            else:
                array.attrs["units"] = result.attrs["units"]
