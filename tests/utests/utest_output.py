import unittest

from tc_diag_driver import output
import xarray as xr
import numpy as np
import numpy.testing as npt


class Test_sorted_levels(unittest.TestCase):
    def test_MinimalSoundingDataset(self):
        data_vars = dict(foo=(["level_hPa"], np.arange(2)))
        ds = xr.Dataset(data_vars, coords=dict(level_hPa=np.array([100, 200])))

        levels = output._sorted_levels(ds)
        expected = np.array([200, 100])
        npt.assert_array_equal(levels, expected)


class Test_sorted_hours(unittest.TestCase):
    def test_MinimalSoundingDataset(self):
        data_vars = dict(foo=(["forecast_hour"], np.arange(2)))
        ds = xr.Dataset(data_vars, coords=dict(forecast_hour=np.array([6, 0])))

        hours = output._sorted_hours(ds)
        expected = np.array([0, 6])
        npt.assert_array_equal(hours, expected)


class Test_DiagVarOutputSpec(unittest.TestCase):
    def _assert_spec_is_equal(self, spec, var_name, units, scale_factor,
                              output_type):
        self.assertEqual(spec.var_name, var_name)
        self.assertEqual(spec.units, units)
        self.assertAlmostEqual(spec.scale_factor, scale_factor)
        self.assertEqual(spec.output_type, output_type)

    def test_ValidInput_ProducesCorrectSpec(self):
        for output_type in output.OUTPUT_TYPES:
            with self.subTest(output_type=output_type):
                args = dict(var_name="foo",
                            units="bar",
                            scale_factor=1.0,
                            output_type=output_type)
                spec = output.DiagVarOutputSpec(**args)
                self._assert_spec_is_equal(spec, **args)

    def test_InvalidInput_RaisesValueError(self):
        with self.assertRaises(ValueError):
            output.DiagVarOutputSpec("foo", "bar", "_not_valid_")

    def test_OutputTypeIsNormalized(self):
        spec = output.DiagVarOutputSpec("foo", "bar", "  SURfaCE  ")
        self.assertEqual(spec.output_type, "surface")


#TODO: More tests for the individual routines that produce diag output