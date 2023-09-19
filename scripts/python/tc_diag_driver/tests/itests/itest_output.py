import datetime as dt
import io
import unittest

from tc_diag_driver import output
from tc_diag_driver import results as hr_results

EXPECTED_DIAG = "               *   AVNO  2022051200   *\n" \
"               *   AL13  INVEST       *\n" \
"\n" \
f"{output.STORM_HEADER}" \
"\n" \
"NTIME 002   DELTAT 006\n" \
"TIME    (HR)         0     6\n" \
"D       (STM)     25.0  23.0\n" \
"\n"\
f"{output.SOUNDING_HEADER}" \
"\n"\
"NLEV 003 SURF 0200 0100\n" \
"TIME    (HR)         0     6\n" \
"E_SURF  (SFC)       15  9999\n" \
"F_0200  (SND)        2  9999\n" \
"F_0100  (SND)        1  9999\n" \
"\n" \
f"{output.CUSTOM_HEADER}" \
"\n" \
"NVAR 001\n" \
"C       (CUS)       35  9999\n"


class Test_to_diag_file(unittest.TestCase):
    def setUp(self):
        self.header_info = output.DiagHeaderInfo("avno",
                                                 dt.datetime(2022, 5, 12),
                                                 "al", 13, "invest")

        storm_var_name = "d"
        surface_var_name = "e_surf"
        sounding_var_name = "f"
        custom_var_name = "c"
        self.results = hr_results.ForecastHourResults(
            [0, 6], [100, 200],
            [surface_var_name, storm_var_name, custom_var_name],
            [sounding_var_name])

        self.results.add_pressure_independent_result(storm_var_name, 0, 25.0)
        self.results.add_pressure_independent_result(storm_var_name, 6, 23.0)
        self.results.add_pressure_independent_result(surface_var_name, 0, 15.0)
        self.results.add_pressure_independent_result(custom_var_name, 0, 35.0)

        self.results.add_sounding_result(sounding_var_name, 0, 100, 10.0)
        self.results.add_sounding_result(sounding_var_name, 0, 200, 20.0)

        self.output_var_specs = [
            output.DiagVarOutputSpec(storm_var_name,
                                     "stm",
                                     "storm",
                                     output_float=True),
            output.DiagVarOutputSpec(surface_var_name, "sfc", "surface"),
            output.DiagVarOutputSpec(sounding_var_name,
                                     "snd",
                                     "sounding",
                                     scale_factor=0.1),
            output.DiagVarOutputSpec(custom_var_name, "cus", "custom")
        ]

    def _assert_output_equal(self, result_string, expected_string):
        comp_lines = io.StringIO(result_string).readlines()
        expected_lines = io.StringIO(expected_string).readlines()

        self.assertEqual(len(comp_lines), len(expected_lines),
                         f"Wrong number of lines generated:\n{result_string}")

        for result_line, expected_line, line_number in zip(
                comp_lines, expected_lines, range(len(comp_lines))):
            with self.subTest(line_number=line_number):
                self.assertEqual(result_line, expected_line)

    def test_ResultsWithMissingHour(self):
        buffer = io.StringIO()
        output.to_diag_file(buffer, self.results, self.output_var_specs,
                            self.header_info, 9999)

        self._assert_output_equal(buffer.getvalue(), EXPECTED_DIAG)
