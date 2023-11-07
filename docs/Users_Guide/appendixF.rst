.. _appendixF:

***************************
Appendix F Python Embedding
***************************

Introduction
============

MET includes the ability to embed Python to a limited degree. Users may use their own Python scripts and any associated Python packages they wish in order to prepare 2D gridded data fields, point observations, and matched pairs as input to the MET tools. We fully expect that this degree of embedding will increase in the future. In addition, plans are in place to extend Python with MET in upcoming releases, allowing users to invoke MET tools directly from their Python script. While MET version 8.0 was built on Python 2.x, MET versions 9.0 and beyond are built on Python 3.6+.

.. _compiling_python_support:

Compiling MET for Python Embedding
==================================

In order to use Python embedding, a local Python installation must be available when compiling the MET software with the following requirements:

1. Python version 3.10.4+

2. C-language Python header files and libraries

3. **NumPy** Python package

4. **netCDF4** Python package

5. **Pandas** Python package

6. **Xarray** Python package

7. **YAML** Python package

Users should be aware that in some cases, the C-language Python header files and libraries may be deleted at the end of the Python installation process, and they may need to confirm their availability prior to compiling MET. Once the user has confirmed the above requirements are satisfied, they can compile the MET software for Python embedding by passing the **\-\-enable-python** option to the **configure** script on the command line. This will link the MET C++ code directly to the Python libraries. The **NumPy** and **netCDF4** Python packages are required by the Python scripts included with the MET software that facilitate the passing of data in memory and the reading and writing of temporary files when Python embedding is used. The **YAML** Python package is required by the tropical cyclone diagnostics Python scripts called by the TC-Diag tool.

In addition to using **\-\-enable-python** with **configure** as mentioned above, the following environment variables must also be set prior to executing **configure**: **MET_PYTHON_BIN_EXE**, **MET_PYTHON_CC**, and **MET_PYTHON_LD**. These may either be set as environment variables or as command line options to **configure**. These environment variables are used when building MET to enable the compiler to find the requisite Python executable, header files, and libraries in the user's local filesystem. Fortunately, Python provides a way to set these variables properly. This frees the user from the necessity of having any expert knowledge of the compiling and linking process. Along with the **Python** executable in the users local Python installation, there should be another executable called **python3-config**, whose output can be used to set these environment variables as follows:

• Set **MET_PYTHON_BIN_EXE** to the full path of the desired Python executable.

• On the command line, run "**python3-config \-\-cflags**". Set the value of **MET_PYTHON_CC** to the output of that command.

• Again on the command line, run "**python3-config \-\-ldflags \-\-embed**". Set the value of **MET_PYTHON_LD** to the output of that command.

Make sure that these are set as environment variables or that you have included them on the command line prior to running **configure**

If a user attempts to invoke Python embedding with a version of MET that was not compiled with Python, MET will return an ERROR:

.. code-block:: none
   :caption: MET Errors Without Python Enabled

   ERROR  : Met2dDataFileFactory::new_met_2d_data_file() -> Support for Python has not been compiled!
   ERROR  : To run Python scripts, recompile with the --enable-python option.

   - or -

   ERROR  : process_point_obs() -> Support for Python has not been compiled!
   ERROR  : To run Python scripts, recompile with the --enable-python option.

Controlling Which Python MET Uses When Running
==============================================

When MET is compiled with Python embedding support, MET uses the Python executable in that Python installation by default when Python embedding is used. However, for users of highly configurable Python environments, the Python instance set at compilation time may not be sufficient. Users may want to use an alternate Python installation if they need additional packages not available in the Python installation used when compiling MET. In MET versions 9.0+, users have the ability to use a different Python executable when running MET than the version used when compiling MET by setting the environment variable **MET_PYTHON_EXE**. Whenever **MET_PYTHON_EXE** is set, MET writes a temporary file, as described in :numref:`Contributor's Guide Section %s <tmp_files_python_embedding>`.

If a user's Python script requires packages that are not available in the Python installation used when compiling the MET software, they will encounter a runtime error when using MET. In this instance, the user will need to change the Python MET is using to a different installation with the required packages for their script. It is the responsibility of the user to manage this Python installation, and one popular approach is to use a custom Anaconda (Conda) Python environment. Once the Python installation meeting the user's requirements is available, the user can force MET to use it by setting the **MET_PYTHON_EXE** environment variable to the full path of the Python executable in that installation. For example:

.. code-block:: none
   :caption: Setting MET_PYTHON_EXE

   export MET_PYTHON_EXE=/usr/local/python3/bin/python3

Setting this environment variable triggers slightly different processing logic in MET than when MET uses the Python installation that was used when compiling MET. When using the Python installation that was used when compiling MET, Python is called directly and data are passed in memory from Python to the MET tools. When the user sets **MET_PYTHON_EXE**, MET does the following:

1. Wrap the user's Python script and arguments with a wrapper script (write_tmp_mpr.py, write_tmp_point.py, or write_tmp_dataplane.py) and specify the name of a temporary file to be written.

2. Use a system call to the **MET_PYTHON_EXE** Python instance to execute these commands and write the resulting data objects to a temporary ASCII or NetCDF file.

3. Use the Python instance that MET was compiled with to run a wrapper script (read_tmp_ascii.py or read_tmp_dataplane.py) to read data from that temporary file.

With this approach, users are able to execute Python scripts using their own custom Python installations.

.. _pyembed-data-structures:

Data Structures Supported by Python Embedding
=============================================

Python embedding with MET tools offers support for three different types of data structures:

1. Two-dimensional (2D) gridded dataplanes

2. Point data conforming to the :ref:`MET 11-column format<table_reformat-point_ascii2nc_format>`

3. Matched-pair data conforming to the :ref:`MET MPR Line Type<table_PS_format_info_MPR>`

Details for each of these data structures are provided below.

.. note::

   All sample commands and directories listed below are relative to the top level of the MET source code directory.

.. _pyembed-2d-data:

Python Embedding for 2D Gridded Dataplanes
------------------------------------------

Currently, MET supports two different types of Python objects for two-dimensional gridded dataplanes: NumPy N-dimensional arrays (ndarrays) and Xarray DataArrays. The keyword **PYTHON_NUMPY** is used on the command line when using ndarrays, and **PYTHON_XARRAY** when using Xarray DataArrays. Example commands are included at the end of this section. 

Python Script Requirements for 2D Gridded Dataplanes
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^

1. The data must be stored in a variable with the name **met_data**

2. The **met_data** variable must be of type **Xarray DataArray** or **NumPy N-D Array**

3. The data inside the **met_data** variable must be **double precision floating point** type

4. A Python dictionary named **attrs** must be defined in the user's script and contain the :ref:`required attributes<pyembed-2d-attrs>` and
   any :ref:`optional attributes<pyembed-2d-attrs>`

.. _pyembed-2d-attrs:

Attributes for 2D Gridded Dataplanes
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^

.. list-table:: 2D Dataplane Attributes
   :widths: 5 5 10 5
   :header-rows: 1

   * - key
     - description
     - data type/format
     - required/optional
   * - valid
     - valid time
     - string (YYYYMMDD_HHMMSS)
     - required
   * - init
     - initialization time
     - string (YYYYMMDD_HHMMSS)
     - required
   * - lead
     - forecast lead
     - string (HHMMSS)
     - required
   * - accum
     - accumulation interval
     - string (HHMMSS)
     - required
   * - name
     - variable name
     - string
     - required
   * - long_name
     - variable long name
     - string
     - required
   * - level
     - variable level
     - string
     - required
   * - units
     - variable units
     - string
     - required
   * - grid
     - :ref:`grid information<pyembed-grid-attrs>`
     - string or dict
     - required
   * - fill_value
     - :ref:`missing data value<pyembed-fillvalue-attrs>`
     - int or float
     - optional

.. note::
   
   Often times Xarray DataArray objects come with their own set of attributes available as a property. To avoid conflict with the required attributes
   for MET, it is advised to strip these attributes and rely on the **attrs** dictionary defined in your script.

.. _pyembed-fillvalue-attrs:

Python embedding for 2D gridded dataplanes provides support for a user-defined missing data (or fill value). By default, the MET tools will respect (and ignore) the following special values in a user's **met_data** variable:

1. NaN
2. Inf
3. -9999
4. -9999.

If a user has a 2D dataplane with another value that should be considered a fill value by MET, then the user must use the **fill_value** attribute in the **attrs** dictionary. An example would be if a user had a 2D dataplane with missing data indicated with -99. A user can use the **fill_value** attribute in their **attrs** dictionary which will tell MET to ignore those values:

.. code-block:: none
   :caption: User Fill Value for 2D Dataplane
   
   'fill_value': -99

Alternatively, the user can choose to replace their special values with one of the four supported values instead of setting the **fill_value** attribute. Note that only a single user-defined fill value is supported at this time.

.. _pyembed-grid-attrs:

The grid entry in the **attrs** dictionary must contain the grid size and projection information in the same format that is used in the netCDF files written out by the MET tools. The value of this item in the dictionary can either be a string, or another dictionary. Examples of the **grid** entry defined as a string are:

• Using a named grid supported by MET:

.. code-block:: none
   :caption: Named Grid

   'grid': 'G212'

• As a grid specification string, as described in :ref:`appendixB`:

.. code-block:: none
   :caption: Grid Specification String

   'grid': 'lambert 185 129 12.19 -133.459 -95 40.635 6371.2 25 25 N'

• As the path to an existing gridded data file:

.. code-block:: none
   :caption: Grid From File

   'grid': '/path/to/sample_data.grib'

When specified as a dictionary, the contents of the **grid** entry vary based upon the grid **type**. The required elements for supported grid types are:

• **Lambert Conformal** grid dictionary entries:

  • type                           ("Lambert Conformal")
  • name                           (string)
  • hemisphere                     (string: "N" or "S")
  • scale_lat_1, scale_lat_2       (double)
  • lat_pin, lon_pin, x_pin, y_pin (double)
  • lon_orient                     (double)
  • d_km, r_km                     (double)
  • nx, ny                         (int)

• **Polar Stereographic** grid dictionary entries:

  • type                           ("Polar Stereographic")
  • name                           (string)
  • hemisphere                     (string: "N" or "S")
  • scale_lat                      (double)
  • lat_pin, lon_pin, x_pin, y_pin (double)
  • lon_orient                     (double)
  • d_km, r_km                     (double)
  • nx, ny                         (int)

• **Mercator** grid dictionary entries:

  • type   ("Mercator")
  • name   (string)
  • lat_ll (double)
  • lon_ll (double)
  • lat_ur (double)
  • lon_ur (double)
  • nx, ny (int)

• **LatLon** grid dictionary entries:

  • type                 ("LatLon")
  • name                 (string)
  • lat_ll, lon_ll       (double)
  • delta_lat, delta_lon (double)
  • Nlat, Nlon           (int)

• **Rotated LatLon** grid dictionary entries:

  • type                                     ("Rotated LatLon")
  • name                                     (string)
  • rot_lat_ll, rot_lon_ll                   (double)
  • delta_rot_lat, delta_rot_lon             (double)
  • Nlat, Nlon                               (int)
  • true_lat_south_pole, true_lon_south_pole (double)
  • aux_rotation                             (double)

• **Gaussian** grid dictionary entries:

  • type     ("Gaussian")
  • name     (string)
  • lon_zero (double)
  • nx, ny   (int)

• **SemiLatLon** grid dictionary entries:

  • type     ("SemiLatLon")
  • name     (string)
  • lats     (list of doubles)
  • lons     (list of doubles)
  • levels   (list of doubles)
  • times    (list of doubles)

Additional information about supported grids can be found in :ref:`appendixB`.

Finally, an example **attrs** dictionary is shown below:

.. code-block:: none
   :caption: Sample Attrs Dictionary

   attrs = {
      
      'valid':     '20050807_120000',
      'init':      '20050807_000000',
      'lead':      '120000',
      'accum':     '120000',

      'name':      'Foo',
      'long_name': 'FooBar',
      'level':     'Surface',
      'units':     'None',
 
      # Define 'grid' as a string or a dictionary
 
      'grid': {
         'type': 'Lambert Conformal',
         'hemisphere': 'N',
         'name': 'FooGrid',
         'scale_lat_1': 25.0,
         'scale_lat_2': 25.0,
         'lat_pin': 12.19,
         'lon_pin': -135.459,
         'x_pin': 0.0,
         'y_pin': 0.0,
         'lon_orient': -95.0,
         'd_km': 40.635,
         'r_km': 6371.2,
         'nx': 185,
         'ny': 129,
       }
   }

Running Python Embedding for 2D Gridded Dataplanes
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^

On the command line for any of the MET tools which will be obtaining its data from a Python script rather than directly from a data file, the user should specify either **PYTHON_NUMPY** or **PYTHON_XARRAY** wherever a (forecast or observation) data file would normally be given. Then in the **name** entry of the config file dictionaries for the forecast or observation data (typically used to specify the field name from the input data file), the user should list the **full path** to the Python script to be run followed by any command line arguments for that script. Note that for tools like MODE that take two data files, it is entirely possible to use the **PYTHON_NUMPY** for one file and the **PYTHON_XARRAY** for the other.

Listed below is an example of running the Plot-Data-Plane tool to call a Python script for data that is included with the MET release tarball. Assuming the MET executables are in your path, this example may be run from the top-level MET source code directory:

.. code-block:: none
   :caption: plot_data_plane Python Embedding

   plot_data_plane PYTHON_NUMPY fcst.ps \
   'name="scripts/python/examples/read_ascii_numpy.py data/python/fcst.txt FCST";' \
   -title "Python enabled plot_data_plane"
    
The first argument for the Plot-Data-Plane tool is the gridded data file to be read. When calling Python script that has a two-dimensional gridded dataplane stored in a NumPy N-D array object, set this to the constant string **PYTHON_NUMPY**. The second argument is the name of the output PostScript file to be written. The third argument is a string describing the data to be plotted. When calling a Python script, set **name** to the full path of the Python script to be run along with any command line arguments for that script. Lastly, the **-title** option is used to add a title to the plot. Note that any print statements included in the Python script will be printed to the screen. The above example results in the following log messages:

.. code-block:: none
		
   DEBUG 1: Opening data file: PYTHON_NUMPY
   Input File: 'data/python/fcst.txt'
   Data Name : 'FCST'
   Data Shape: (129, 185)
   Data Type:  dtype('float64')
   Attributes: {'name': 'FCST',  'long_name': 'FCST_word',
                'level': 'Surface', 'units': 'None',
                'init': '20050807_000000', 'valid': '20050807_120000',
                'lead': '120000',  'accum': '120000'
                'grid': {...} } 
   DEBUG 1: Creating postscript file: fcst.ps

Special Case for Ensemble-Stat, Series-Analysis, and MTD
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^

The Ensemble-Stat, Series-Analysis, MTD and Gen-Ens-Prod tools all have the ability to read multiple input files. Because of this feature, a different approach to Python embedding is required. A typical use of these tools is to provide a list of files on the command line. For example:

.. code-block::
   :caption: Gen-Ens-Prod Command Line

   gen_ens_prod ens1.nc ens2.nc ens3.nc ens4.nc -out ens_prod.nc -config GenEnsProd_config

In this case, a user is passing 4 ensemble members to Gen-Ens-Prod to be evaluated, and each member is in a separate file. If a user wishes to use Python embedding to process the ensemble input files, then the same exact command is used however special modifications inside the GenEnsProd_config file are needed. In the config file dictionary, the user must set the **file_type** entry to either **PYTHON_NUMPY** or **PYTHON_XARRAY** to activate the Python embedding for these tools. Then, in the **name** entry of the config file dictionaries for the forecast or observation data, the user must list the **full path** to the Python script to be run. However, in the Python command, replace the name of the input gridded data file to the Python script with the constant string **MET_PYTHON_INPUT_ARG**. When looping over all of the input files, the MET tools will replace that constant **MET_PYTHON_INPUT_ARG** with the path to the input file currently being processed and optionally, any command line arguments for the Python script. Here is what this looks like in the GenEnsProd_config file for the above example:

.. code-block::
   :caption: Gen-Ens-Prod MET_PYTHON_INPUT_ARG Config

   file_type = PYTHON_NUMPY;
   field = [ { name = "gen_ens_prod_pyembed.py MET_PYTHON_INPUT_ARG"; } ];

In the event the user requires command line arguments to their Python script, they must be included alongside the file names separated by a delimiter. For example, the above Gen-Ens-Prod command with command line arguments for Python would look like:

.. code-block::
   :caption: Gen-Ens-Prod Command Line with Python Args
   
   gen_ens_proce ens1.nc,arg1,arg2 ens2.nc,arg1,arg2 ens3.nc,arg1,arg2 ens4.nc,arg1,arg2 \
   -out ens_prod.nc -config GenEnsProd_config

In this case, the user's Python script will receive "ens1.nc,arg1,arg2" as a single command line argument for each execution of the Python script (i.e. 1 time per file). The user must parse this argument inside their Python script to obtain **arg1** and **arg2** as separate arguments. The list of input files and optionally, any command line arguments can be written to a single file called **file_list** that is substituted for the file names and command line arguments. For example:

.. code-block::
   :caption: Gen-Ens-Prod File List

   echo "ens1.nc,arg1,arg2 ens2.nc,arg1,arg2 ens3.nc,arg1,arg2 ens4.nc,arg1,arg2" > file_list
   gen_ens_prod file_list -out ens_prod.nc -config GenEnsProd_config

Finally, the above tools do not require data files to be present on a local disk. If the user wishes, their Python script can obtain data from other sources based upon only the command line arguments to their Python script. For example:

.. code-block::
   :caption: Gen-Ens-Prod Python Args Only

   gen_ens_prod 20230101,0 20230102,0 20230103,0 -out ens_prod.nc -confg GenEnsProd_config

In the above command, each of the arguments "20230101,0", "20230102,0", and "20230103,0" are provided to the user's Python script in separate calls. Then, inside the Python script these arguments are used to construct a filename or query to a data server or other mechanism to return the desired data and format it the way MET expects inside the Python script, prior to calling Gen-Ens-Prod.

Examples of Python Embedding for 2D Gridded Dataplanes
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^

**Grid-Stat with Python embedding for forecast and observations**

.. code-block:: none
   :caption: GridStat Command with Dual Python Embedding

   grid_stat 'PYTHON_NUMPY' 'PYTHON_NUMPY' GridStat_config -outdir /path/to/output

.. code-block:: none
   :caption: GridStat Config with Dual Python Embedding

   fcst = {
      field = [
         {
           name = "/path/to/fcst/python/script.py python_arg1 python_arg2";
         }
      ];
    }

    obs = {
      field = [
         {
           name = "/path/to/obs/python/script.py python_arg1 python_arg2";
         }
      ];
    }

.. _pyembed-point-obs-data:

Python Embedding for Point Observations
---------------------------------------

MET also supports point observation data supplied in the :ref:`MET 11-column format<table_reformat-point_ascii2nc_format>`.

Python Script Requirements for Point Observations
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^

1. The data must be stored in a variable with the name **point_data**

2. The **point_data** variable must be a Python list representation of a NumPy N-D Array created from a Pandas DataFrame

3. The **point_data** variable must have data in each of the 11 columns required for the MET tools even if it is NA

To provide the data that MET expects for point observations, the user is encouraged when designing their Python script to consider how to map their observations into the MET 11-column format. Then, the user can populate their observations into a Pandas DataFrame with the following column names and dtypes:

.. list-table:: Point Observation DataFrame Columns and Dtypes
   :widths: 5 5 10
   :header-rows: 1

   * - column name
     - data type (dtype)
     - description
   * - typ
     - string
     - Message Type
   * - sid
     - string
     - Station ID
   * - vld
     - string
     - Valid Time (YYYYMMDD_HHMMSS)
   * - lat
     - numeric
     - Latitude (Degrees North)
   * - lon 
     - numeric
     - Longitude (Degrees East)
   * - elv
     - numeric
     - Elevation (MSL)
   * - var
     - string
     - Variable name (or GRIB code)
   * - lvl
     - numeric
     - Level
   * - hgt
     - numeric
     - Height (MSL or AGL)
   * - qc
     - string
     - QC string
   * - obs
     - numeric
     - Observation Value

To create the variable for MET, use the **.values** property of the Pandas DataFrame and the **.tolist()** method of the NumPy N-D Array. For example:

.. code-block:: Python
   :caption: Convert Pandas DataFrame to MET variable

   # Pandas DataFrame
   my_dataframe = pd.DataFrame()

   # Convert to MET variable
   point_data = my_dataframe.values.tolist()

Running Python Embedding for Point Observations
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^

The Point2Grid, Plot-Point-Obs, Ensemble-Stat, and Point-Stat tools support Python embedding for point observations. Python embedding for these tools can be invoked directly on the command line by replacing the input MET NetCDF point observation file name with the **full path** to the Python script and any arguments. The Python command must begin with the prefix **PYTHON_NUMPY=**. The full command should be enclosed in quotes to prevent embedded whitespace from causing parsing errors. An example of this is shown below for Plot-Point-Obs:

.. code-block:: none
   :caption: plot_point_obs with Python Embedding

   plot_point_obs \
   "PYTHON_NUMPY=scripts/python/examples/read_ascii_point.py data/sample_obs/ascii/sample_ascii_obs.txt" \
   output_image.ps

The ASCII2NC tool also supports Python embedding, however invoking it varies slightly from other MET tools. For ASCII2NC, Python embedding is used by providing the "-format python" option on the command line. With this option, point observations may be passed as input. An example of this is shown below:

.. code-block:: none
   :caption: ascii2nc with Python Embedding

   ascii2nc -format python \
   "scripts/python/examples/read_ascii_point.py data/sample_obs/ascii/sample_ascii_obs.txt" \
   sample_ascii_obs_python.nc

Both of the above examples use the **read_ascii_point.py** example script which is included with the MET code. It reads ASCII data in MET's 11-column point observation format and stores it in a Pandas DataFrame to be read by the MET tools using Python embedding for point data. The **read_ascii_point.py** example script can be found in:

• MET installation directory in *scripts/python/examples*.

• `MET GitHub repository <https://github.com/dtcenter/MET>`_ in *scripts/python/examples*.

Examples of Python Embedding for Point Observations
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^

**Point-Stat with Python embedding for forecast and observations**

.. code-block:: none
   :caption: PointStat Command with Dual Python Embedding

   point_stat 'PYTHON_NUMPY' 'PYTHON_NUMPY=/path/to/obs/python/script.py python_arg1 python_arg2' PointStat_config -outdir /path/to/output

.. code-block:: none
   :caption: PointStat Config with Dual Python Embedding

   fcst = {
      field = [
         {
           name = "/path/to/fcst/python/script.py python_arg1 python_arg2";
         }
      ];
    }

.. _pyembed-mpr-data:

Python Embedding for MPR Data
-----------------------------

The MET Stat-Analysis tool also supports Python embedding. By using the command line option **-lookin python**, Stat-Analysis can read matched pair (MPR) data formatted in the MET MPR line-type format via Python.

.. note::

   This functionality assumes you are passing only the MPR line type information, and not other statistical line types. Sometimes users configure MET tools to write the MPR line type to the STAT file (along with all other line types). The example below will not work for those files, but rather only files from MET tools containing just the MPR line type information, or optionally, data in another format that the user adapts to the MPR line type format.

Python Script Requirements for MPR Data
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^

1. The data must be stored in a variable with the name **mpr_data**

2. The **mpr_data** variable must be a Python list representation of a NumPy N-D Array created from a Pandas DataFrame

3. The **met_data** variable must have data in **exactly** 36 columns, corresponding to the summation of the :ref:`common STAT output<table_PS_header_info_point-stat_out>` and the :ref:`MPR line type output<table_PS_format_info_MPR>`.

If a user does not have an existing MPR line type file created by the MET tools, they will need to map their data into the 36 columns expected by Stat-Analysis for the MPR line type data. If a user already has MPR line type files, the most direct way for a user to read MPR line type data is to model their Python script after the sample **read_ascii_mpr.py** script. Sample code is included here for convenience:

.. code-block:: Python
   :caption: Reading MPR line types with Pandas

   # Open the MPR line type file
   mpr_dataframe = pd.read_csv(input_mpr_file,\
                               header=None,\
                               delim_whitespace=True,\
                               keep_default_na=False,\
                               skiprows=1,\
                               usecols=range(1,36),\
                               dtype=str)

   # Convert to the variable MET expects
   mpr_data = mpr_dataframe.values.tolist()

Running Python Embedding for MPR Data
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^

Stat-Analysis can be run using the **-lookin python** command line option:

.. code-block:: none
   :caption: Stat-Analysis with Python Embedding of MPR Data
   
   stat_analysis \
   -lookin python scripts/python/examples/read_ascii_mpr.py point_stat_mpr.txt \
   -job aggregate_stat -line_type MPR -out_line_type CNT \
   -by FCST_VAR,FCST_LEV

In this example, rather than passing the MPR output lines from Point-Stat directly into Stat-Analysis (which is the typical approach), the **read_ascii_mpr.py** Python embedding script reads that file and passes the data to Stat-Analysis. The aggregate_stat job is defined on the command line and CNT statistics are derived from the MPR input data. Separate CNT statistics are computed for each unique combination of FCST_VAR and FCST_LEV present in the input.

The **read_ascii_mpr.py** sample script can be found in:

• MET installation directory in *scripts/python/examples*.

• `MET GitHub repository <https://github.com/dtcenter/MET>`_ in *MET/scripts/python/examples*.

MET Python Package
==================

MET comes with a Python package that provides core functionality for the Python embedding capability. In rare cases, advanced users may find the classes and functions included with this Python package useful.

To utilize the MET Python package **standalone** when NOT using it with Python embedding, users must add the following to their **PYTHONPATH** environment variable:

.. code-block::
   :caption: MET Python Module PYTHONPATH

   export PYTHONPATH={MET_INSTALL_DIR}/share/met/python

where {MET_INSTALL_DIR} is the top level directory where MET is installed, for example **/usr/local/met**.
