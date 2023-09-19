#!/usr/bin/env bash
export PYTHONPATH=.

LAND_FILE=tests/post_resample_test/input_data/v2023-04-07_gdland_table.dat
if [ ! -f $LAND_FILE ]; then
    wget -nH -P tests/post_resample_test/input_data/ https://rammb-data.cira.colostate.edu/~/rdemaria/gdland/v2023-04-07_gdland_table.dat
fi


python -m tc_diag_driver.post_resample_driver -o tests/post_resample_test/output/parent/ tests/post_resample_test/post_resample.yml tests/post_resample_test/input_data/tmp_tc_diag_AL092022_GFSO_2022092400_f6_parent_29849_0_.nc
python -m tc_diag_driver.post_resample_driver -o tests/post_resample_test/output/nest/ tests/post_resample_test/post_resample_nest.yml tests/post_resample_test/input_data/tmp_tc_diag_AL092022_GFSO_2022092400_f6_nest_29849_0_.nc
