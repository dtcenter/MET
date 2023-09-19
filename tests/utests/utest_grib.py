import unittest

import numpy as np
import numpy.testing as npt
from tc_diag_driver import grib


class MockGrib:
    def __init__(self):
        self.t = np.array([[0, 1], [2, 3]])
        self.u_1000 = np.array([[4, 5], [6, 7]])
        self.u_500 = np.array([[8, 9], [10, 11]])
        self.r = np.array([[12, 13], [14, 15]])
        self.lons = np.array([[16, 17], [16, 17]])
        self.lats = np.array([[18, 18], [19, 19]])

        self.data = {
            "t_0_isobaricInhPa": MockRecord(self.t, self.lons, self.lats),
            "2r_2_heightAboveGround": MockRecord(self.r),
            "u_1000_isobaricInhPa": MockRecord(self.u_1000),
            "u_500_isobaricInhPa": MockRecord(self.u_500)
        }

    def select(self, shortName, level, typeOfLevel):
        fake_record = self.data[f"{shortName}_{level:d}_{typeOfLevel}"]
        return [fake_record]

    def close(self):
        pass


class MockRecord:
    def __init__(self, values, lons=None, lats=None):
        self.values = values
        self.lons = lons
        self.lats = lats

    def latlons(self):
        return self.lats, self.lons


class TestInputVarSpec(unittest.TestCase):
    def test_UnspecifiedGribName_SameAsVarName(self):
        spec = grib.InputVarSpec("foo", "bar")
        self.assertEqual(spec.grib_name, "foo")

    def test_SpecifiedGribName(self):
        spec = grib.InputVarSpec("foo", "bar", "buzz")
        self.assertEqual(spec.grib_name, "buzz")


class Test_grib_to_dataset(unittest.TestCase):
    def setUp(self):
        spec_list = [
            grib.InputVarSpec("u", "isobaricInhPa"),
            grib.InputVarSpec("r_surf", "heightAboveGround", "2r", True, 2)
        ]

        nav_var_spec = grib.InputVarSpec("t", "isobaricInhPa", "t", False, 0)

        self.mock_grib = MockGrib()
        self.dataset, self.lons, self.lats = grib.grib_to_dataset(
            "NO_FILENAME",
            nav_var_spec, [1000, 500],
            spec_list,
            grib_file=self.mock_grib)

    def test_LonsExtractedCorrectly(self):
        npt.assert_array_equal(self.lons, self.mock_grib.lons)

    def test_LatsExtractedCorrectly(self):
        npt.assert_array_equal(self.lats, self.mock_grib.lats)

    def _has_coord(self, var_name, coord_name):
        return coord_name in self.dataset[var_name].coords.keys()

    def test_SurfaceVarHasLonCoord(self):
        self.assertTrue(self._has_coord("r_surf", "lon"))

    def test_SurfaceVarHasLatCoord(self):
        self.assertTrue(self._has_coord("r_surf", "lat"))

    def test_SurfaceVarDoesNotHaveLevelCoord(self):
        self.assertFalse(self._has_coord("r_surf", "level"))

    def test_SurfaceVarHasCorrectValues(self):
        npt.assert_array_equal(self.dataset["r_surf"].values, self.mock_grib.r)

    def test_SoundingVarHasLonCoord(self):
        self.assertTrue(self._has_coord("u", "lon"))

    def test_SoundingVarHasLonCoord(self):
        self.assertTrue(self._has_coord("u", "lat"))

    def test_SoundingVarHasLevelCoord(self):
        self.assertTrue(self._has_coord("u", "level"))

    def test_SoundingVarCanBeSelectedByLevel(self):
        with self.subTest(level=500):
            npt.assert_array_equal(self.dataset["u"].sel(level=500).values,
                                   self.mock_grib.u_500)

        with self.subTest(level=1000):
            npt.assert_array_equal(self.dataset["u"].sel(level=1000).values,
                                   self.mock_grib.u_1000)

    def test_LonCoordCreatedCorrectly(self):
        lon_data = self.dataset.coords["lon"].values
        npt.assert_array_equal(lon_data, np.array([16, 17]))

    def test_LatCoordCreatedCorrectly(self):
        lat_data = self.dataset.coords["lat"].values
        npt.assert_array_equal(lat_data, np.array([18, 19]))
