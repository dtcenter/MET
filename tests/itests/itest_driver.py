import datetime as dt
import pathlib
import unittest
from typing import List

import numpy as np
import numpy.testing as npt
from atcf_tools import track_tools

from tc_diag_driver import computation_engine as ce
from tc_diag_driver import diag_vars, driver, grib, model_files
from tests import test_data


def _constant(constant_val: float, **_kwargs) -> float:
    return constant_val


class Test_process_model_entry(unittest.TestCase):
    def setUp(self):
        pass

    def _arrange_shared_test_specs(self, atcf_filename: str,
                                   forecast_hours: List[int]):
        fmt = f"{test_data.GFS_DIR}"+"/{model_time:%Y%m%d%H}/{resolution}/" \
                "{model_name}.{model_time:%Y%m%d%H}.pgrb2.{resolution}" \
                ".f{forecast_time_hours:03d}"
        model_time = dt.datetime(2021, 8, 27, 12)

        land_lut = diag_vars.get_land_lut(
            pathlib.Path("tests/land_lut/current_operational_gdland.dat"))

        # Create minimal ModelSpec and ModelEntry objects
        model_spec = model_files.ModelSpec("gfs", fmt, "1p00", "UNUSED",
                                           [1000, 500], forecast_hours, "t",
                                           1000, "isobaricInhPa", False, 5, 4,
                                           10, "AVNO", "UNUSED_IN_TEST.dat",
                                           [], [], [], [])
        model_entry = driver.ModelEntry(model_spec, "al092021", atcf_filename,
                                        model_time, "UNUSED_DIR/")

        return land_lut, model_spec, model_entry

    @unittest.skipUnless(test_data.GFS_DIR.exists(),
                         f"No access to GFS test dir: {test_data.GFS_DIR}")
    def test_IdaSingleTime(self):
        land_lut, model_spec, model_entry = self._arrange_shared_test_specs(
            test_data.ATCF_FILENAME, [0])

        # Setup the input specifications
        input_vars = [
            grib.InputVarSpec("u", "isobaricInhPa"),
            grib.InputVarSpec("r_surf", "heightAboveGround", "2r", True, 2)
        ]

        # Setup the DiagComputations.
        # We're just using r_surf and u as test variables.
        pi_vars = [
            ce.DiagComputation("r_surf",
                               diag_vars.mean_in_radius_range,
                               kwargs=dict(min_radius_km=0,
                                           max_radius_km=500,
                                           grib_var_name="r_surf"))
        ]
        sounding_vars = [
            ce.DiagComputation("u",
                               diag_vars.mean_in_radius_range,
                               kwargs=dict(min_radius_km=0,
                                           max_radius_km=500,
                                           grib_var_name="u"))
        ]

        track = track_tools.get_adeck_track(test_data.ATCF_FILENAME,
                                            model_spec.atcf_tech_id)

        results = driver.process_model_entry(model_entry, land_lut, track,
                                             input_vars, pi_vars,
                                             sounding_vars)

        # Check that the results match the expected values.
        with self.subTest(var="Sounding Var: u"):
            u = results.soundings["u"]
            u_hr0 = u.loc[0].data
            u_expected = np.array([-4.22984762, -0.51299086])
            npt.assert_allclose(
                u_hr0,
                u_expected,
                rtol=0.001,
            )

        with self.subTest(var="Pressure Independent Var: r_surf"):
            r = results.pressure_independent["r_surf"]
            r_hr0 = r.loc[0].data
            self.assertAlmostEqual(r_hr0, 90.9696706, places=3)

    def test_TrackWithFewerHoursThanForecast(self):
        # Using an atcf file with AVNO removed after hour 12 for the model time.
        short_atcf_file = "tests/itests/test_data/atcf/short_aal092021.dat"
        land_lut, model_spec, model_entry = self._arrange_shared_test_specs(
            short_atcf_file, forecast_hours=[0, 24])
        input_vars = [grib.InputVarSpec("u", "isobaricInhPa")]

        pi_vars = [
            ce.DiagComputation("always_10",
                               _constant,
                               kwargs={"constant_val": 10})
        ]

        sounding_vars = []

        track = track_tools.get_adeck_track(short_atcf_file,
                                            model_spec.atcf_tech_id)

        results = driver.process_model_entry(model_entry, land_lut, track,
                                             input_vars, pi_vars,
                                             sounding_vars)

        expected = np.array([10.0, np.nan])
        actual = results.pressure_independent["always_10"]
        npt.assert_allclose(actual, expected)


# We don't care about most of what is in each ATCF forecast row.  This tests
# that the variables we care about are correct for a sample model time and
# forecast hour.
class Test_get_forecast_row(unittest.TestCase):
    def setUp(self) -> None:
        track = track_tools.get_adeck_track(test_data.ATCF_FILENAME, "AVNO")
        self.row = driver.get_forecast_row(track, dt.datetime(2021, 8, 25, 0),
                                           6)

    def test_SingleRowIsReturned(self):
        self.assertEqual(len(self.row), 1)

    def test_lat(self):
        self.assertAlmostEqual(self.row["lat"][0], 12.8)

    def test_lon(self):
        self.assertAlmostEqual(self.row["lon"][0], -72.8)
