import unittest

import numpy as np
import xarray as xr
import xarray.testing as xtest
from tc_diag_driver import results as hr_results


class TestForecastHourResults(unittest.TestCase):
    def test_SoundingNoMissingData(self):
        hours = [10]
        levels = [100, 200]
        hour_results = hr_results.ForecastHourResults(hours, levels, ["bar"],
                                                      ["foo"])

        expected = xr.Dataset(data_vars={
            "foo": (("forecast_hour", "level_hPa"), np.array([[0.0, 1.0]]))
        },
                              coords={
                                  "forecast_hour": hours,
                                  "level_hPa": levels
                              })

        hour_results.add_sounding_result("foo", 10, 100, 0.0)
        hour_results.add_sounding_result("foo", 10, 200, 1.0)
        returned = hour_results.soundings

        xtest.assert_allclose(returned, expected)

    def test_SoundingWithMissingHour(self):
        hours = [10, 20]
        levels = [100, 200]
        hour_results = hr_results.ForecastHourResults(hours, levels, ["bar"],
                                                      ["foo"])

        expected_data = np.array([[0.0, 1.0], [np.nan, np.nan]])
        expected = xr.Dataset(
            data_vars={"foo": (("forecast_hour", "level_hPa"), expected_data)},
            coords={
                "forecast_hour": hours,
                "level_hPa": levels
            })

        # Only add results for hour 10 so that hour 20 is missing.
        hour_results.add_sounding_result("foo", 10, 100, expected_data[0, 0])
        hour_results.add_sounding_result("foo", 10, 200, expected_data[0, 1])
        returned = hour_results.soundings

        xtest.assert_allclose(returned, expected)

    def _make_expected_pressure_independent_dataset(self, hours,
                                                    expected_data):
        return xr.Dataset(
            data_vars={"bar": (("forecast_hour"), expected_data)},
            coords={"forecast_hour": hours})

    def test_PressureIndependentNoMissingData(self):
        hours = [10]
        levels = [100, 200]
        hour_results = hr_results.ForecastHourResults(hours, levels, ["bar"],
                                                      ["foo"])

        expected_data = np.array([0.0])
        expected = self._make_expected_pressure_independent_dataset(
            hours, expected_data)

        hour_results.add_pressure_independent_result("bar", 10, 0.0)
        xtest.assert_allclose(hour_results.pressure_independent, expected)

    def test_PressureIndependentMissingData(self):
        hours = [10, 20]
        levels = [100, 200]
        hour_results = hr_results.ForecastHourResults(hours, levels, ["bar"],
                                                      ["foo"])

        expected_data = np.array([0.0, np.nan])
        expected = self._make_expected_pressure_independent_dataset(
            hours, expected_data)

        hour_results.add_pressure_independent_result("bar", 10, 0.0)
        xtest.assert_allclose(hour_results.pressure_independent, expected)
