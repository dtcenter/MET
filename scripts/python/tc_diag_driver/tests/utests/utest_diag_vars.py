import unittest

import pandas as pd
from tc_diag_driver import diag_vars


class Test_track_row_lookup(unittest.TestCase):
    def setUp(self):
        self.df = pd.DataFrame(dict(lat=[10], lon=[-20]))

    def test_No360ConversionLookup(self):
        val = diag_vars.track_row_lookup(self.df, "lat")
        self.assertEqual(val, 10)

    def test_360ConversionLookup(self):
        val = diag_vars.track_row_lookup(self.df, "lon", convert_to_0_360=True)
        self.assertEqual(val, 340)
