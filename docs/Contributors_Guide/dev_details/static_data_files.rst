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

- Often updated *by developers* during development:

  - The :code:`config` directory contains all of the default configuration
    files used by the MET tools. These are updated routinely when adding new
    features and enhancements to the tools.

  - The :code:`python` and :code:`wrappers` directories contain Python
    scripts to support the Python-embedding logic used throughout MET.
    These are updated routinely when adding new Python-embedding features
    and enhancements.

  - :code:`table_files/met_header_columns_VX.Y.txt` files define
    line types and column names for each major **X.Y** released version
    of MET. Stat-Analysis reads these files when processing the STAT output
    from other MET tools. A new header file is added during development
    whenever the **X.Y** software version number is increased.

- Updated *by developers* prior to **each major X.Y release**:

  - :code:`table_files/ndbc_stations.xml`, described in
    :numref:`User's Guide Section %s <met_ndbc_stations>`, is read by
    ASCII2NC and contains buoy latitude and longitude locations that can
    change on a daily basis. To be used in real time, this file should be
    regenerated daily and the :code:`MET_NDBC_STATION` environment variable
    should define its location. Use the
    :code:`scripts/python/utility/build_ndbc_stations_from_web.py`
    utility to update its contents.

  - :code:`table_files/airnow_monitoring_site_locations_v2.txt`,
    described in :numref:`User's Guide Section %s <met_airnow_stations>`,
    is read by ASCII2NC, contains AIRNOW site latitude and longitude
    locations that can change over time, and should be routinely updated.
    The :code:`MET_AIRNOW_STATIONS` environment variable can be set to
    override its default location.

  - :code:`table_files/grib*.txt` files define the GRIB1 and GRIB2 table
    information. They are read by the MET libraries which read GRIB1 and
    GRIB2 input data. Additions are made to these tables over time and
    they should be routinely updated. The GRIB2 tables should be kept
    current with those included in the **wgrib2** software found in the
    `wgrib2 code repository <https://github.com/NOAA-EMC/wgrib2>`_.

  - :code:`tc_data/wwpts_us.txt` is read by TC-Pairs, contains hurricane
    watch/warning information, and is referenced in the TC-Pairs
    configuration file. Since hurricane watches and warnings change over
    time, this file should be routinely updated.

  - The :code:`map` directory contains map data read by the MET tools
    which create PostScript output plots. This data is derived from GIS
    shapefiles and defines the background map data for those plots.
    Since map data can change over time, these files should be periodically
    updated using the :code:`make_mapfiles` development utility.

- Updated *by developers* only as needed:

  - :code:`table_files/stat_column_description.txt` is read by
    Series-Analysis and contains a description for each statistic
    computed by the tool. These descriptions are written to the **long_name**
    attribute of the output variables. This only needs to be updated when
    Series-Analysis is enhanced to process new output statistics.

  - :code:`tc_data/*land*` files are read by TC-DLand and TC-Pairs to
    define the distance of storms to land. These definitions seldom change
    over time and should only be modified when the TC-DLand tool is modified
    to process the updated inputs.

  - :code:`climo/seeps/PPT24_seepsweights.nc` is read by Point-Stat when
    computing the SEEPS line type as described in :numref:`User's Guide
    Section %s <PS_seeps>`. This only needs to be updated when new SEEPS
    stations are added or the 24-hour precipitation climatology values
    are modified.

- Updated *by users* only as needed:

  - :code:`table_files/obs_error_table.txt`, described in
    :numref:`User's Guide Section %s <met_obs_error_table>`, is read by
    Ensemble-Stat and defines assumptions about observation error. The
    :code:`MET_OBS_ERROR_TABLE` environment variable can be set to
    override its default location. Generally, the observation error
    assumptions in this file do not change over time. Instead, it should
    be copied and modified by researchers for specific scientific
    applications and use cases.

- No updates typically required:

  - The :code:`colortables` directory contains color table definitions
    that are read by MET tools which create PostScript output plots.
    While users are encouraged to copy and modify these color table files
    for their specific needs, these default colortables do not change over
    time and typically require no updates.

  - The :code:`poly` directory contains polyline files read by the MET
    tools when applying polyline masking regions. The default polyline
    regions included were originally defined by NOAA/EMC, do not change
    over time, and typically require no updates.

  - The :code:`ps` directory contains font definition files that are read
    by the MET tools which create PostScript output plots. These font
    definitions do not change over time and typically require no updates.

