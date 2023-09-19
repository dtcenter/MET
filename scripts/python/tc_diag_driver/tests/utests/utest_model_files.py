import datetime as dt
import pathlib
import unittest

from tc_diag_driver import model_files


class Test_ModelDirSpec(unittest.TestCase):
    def test_FormatPathWithAllInput(self):
        fmt = "{forecast_time_hours}{model_name}{model_time:%Y%m%d}{resolution}{nested_grid_id}"
        spec = model_files.ModelSpec("foo", fmt, "res", "bar", [0, 100], [0],
                                     "t", 1000, "isobaricInHPA", False, 5, 4,
                                     10, "AVNO", "UNUSED_IN_TEST.dat", [], [],
                                     [], [])
        result = spec.make_model_path(10, dt.datetime(2022, 3, 24))
        expected = pathlib.Path("10foo20220324resbar")
        self.assertEqual(result, expected)
