.. _static_data_files:

Static Data Files
=================

The MET software package includes static data files that are read at
runtime and impact the behavior of the MET tools. These static data
files are organized into subdirectories of the top-level :code:`data` 
directory. When the MET :code:`configure` script is run, the
:code:`--prefix path` option (default :code:`/usr/local`) defines
the directory where the resulting executables and data files should
be installed. The :code:`make install` step copies the binaries into
the installation :code:`bin` directory and data files into the
:code:`share/met` directory. By default these static data files are
read from the installed :code:`share/met` directory at runtime
unless the **MET_BASE** environment variable, described in
:numref:`User's Guide Section %s <met_base_env_var>`, is set to
override the default location.

Depending on their type, the content of these data files grows stale
over time and requires updates. Listed below are descriptions of some 
of the static data sources found in :code:`share/met`, along with
recommended update frequency and method.

  - :code:`table_files/ndbc_stations.xml`, described in
    :numref:`User's Guide Section %s <met_ndbc_stations>`, is read by
    ASCII2NC and contains buoy latitude and longitude locations that can
    change on a daily basis.
    To be used in real time, this file should be regenerated daily and the
    :code:`MET_NDBC_STATION` environment variable should define its
    current location. Since this static file is included with the code,
    its contents should be updated **for each major X.Y release** using the
    :code:`scripts/python/utility/build_ndbc_stations_from_web.py` utility.

  - :code:`table_files/airnow_monitoring_site_locations_v2.txt`,
    described in :numref:`User's Guide Section %s <met_airnow_stations>`,
    is read by ASCII2NC and contains AIRNOW site latitude and longitude
    locations that can change over time. The :code:`MET_AIRNOW_STATIONS`
    environment variable can be set to override its default location.
    Since this static file is included with the code, its contents should
    be updated **for each major X.Y release**.

  - :code:`table_files/obs_error_table.txt`, described in
    :numref:`User's Guide Section %s <met_obs_error_table>`, is read by
    Ensemble-Stat and defines assumptions about observation error. The
    :code:`MET_OBS_ERROR_TABLE` environment variable can be set to
    override its default location. Generally, the observation error
    assumptions in this file do not change over time. Instead, it should
    be copied and modified by researchers for specific scientific
    applications and use cases. 

  - :code:`table_files/grib*.txt` files define the GRIB1 and
    GRIB2 table information. They are read by the MET libraries which
    read GRIB1 and GRIB2 input data. Additions are made to these tables
    over time and their contents should be reviewed and updated **for each
    major X.Y** release. The GRIB2 tables should be kept current with those
    provided with the **wgrib2** software, available at
    https://github.com/NOAA-EMC/wgrib2.  

  - :code:`table_files/met_header_columns_VX.Y.txt` files define
    line types and column names for each **X.Y** released version of MET.
    Stat-Analysis reads these files when processing the STAT output from
    other MET tools. A new header file must be added **for each major X.Y
    release**.

  - :code:`table_files/stat_column_descriptions*.txt` is read by
    Series-Analysis and contains descriptions of each statistic that are
    written to the **long_name** attribute of the output variables.
    This only needs to be updated when Series-Analysis is enhanced to
    write new output statistic types.

  - :code:`tc_data/wwpts_us.txt` is read by TC-Pairs, contains hurricane
    watch/warning information, and is referenced in the TC-Pairs
    configuration file. Since hurricane watches and warnings change over
    time, its contents should be updated **for each major X.Y release**.

  - :code:`tc_data/*land*` files are read be TC-DLand and/or TC-Pairs to
    define storms distance to land. These definitions seldom change over
    time and should only be modified when the TC-DLand tool is modified
    to process the updated inputs.
 
