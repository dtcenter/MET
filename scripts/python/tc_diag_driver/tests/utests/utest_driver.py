import unittest

import numpy as np
import numpy.testing as npt
from tc_diag_driver import driver


class Test_distances_from_tc(unittest.TestCase):
    def test_TrivialCase(self):
        lons = np.array([[0, 1], [0, 1]])
        lats = np.array([[0, 0], [1, 1]])
        distances = driver.distances_from_tc(lons, lats, 0.5, 0.5)
        expected = np.array([[78.55806775, 78.55806775],
                             [78.55806775, 78.55806775]])
        npt.assert_almost_equal(distances, expected)


class Test_get_atcf_info(unittest.TestCase):
    def test_UnormalizedInput_CorrectBasin(self):
        basin, _storm_num = driver._get_atcf_info("  Al052022  ")
        self.assertEqual(basin, "al")

    def test_UnormalizedInput_CorrectStormNum(self):
        _basin, storm_num = driver._get_atcf_info("  Al052022  ")
        self.assertEqual(storm_num, 5)

    def test_PreNormalizedInput_CorrectBasin(self):
        basin, _storm_num = driver._get_atcf_info("al052022")
        self.assertEqual(basin, "al")

    def test_PreNormalizedInput_CorrectStormNum(self):
        _basin, storm_num = driver._get_atcf_info("al052022")
        self.assertEqual(storm_num, 5)

    def test_BadID_RaisesValueError(self):
        with self.assertRaises(ValueError):
            driver._get_atcf_info("NOT AN ID")
