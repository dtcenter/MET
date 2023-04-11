.. _appendixF:

***************************
Appendix F Python Embedding
***************************

Introduction
============

MET includes the ability to embed Python to a limited degree. Users may use their own Python scripts and whatever associated Python packages they wish in order to prepare 2D gridded data fields, point observations, and matched pairs as input to the MET tools. We fully expect that this degree of embedding will increase in the future. In addition, plans are in place to extend Python with MET in upcoming releases, allowing users to invoke MET tools directly from their Python script. While MET version 8.0 was built on Python 2.x, MET versions 9.0 and beyond are built on Python 3.6+.

.. _compiling_python_support:

Compiling MET for Python Embedding
==================================

In order to use Python embedding, a local Python installation must be available when compiling the MET software with the following requirements:

1. C-language Python header files and libraries

2. **NumPy** Python package

3. **netCDF4** Python package

4. **Pandas** Python package

5. **Xarray** Python package

Users should be aware that in some cases, the C-language Python header files and libraries may be deleted at the end of the Python installation process and they may need to confirm their availability prior to compiling MET. Once the user has confirmed the above requirements are satisfied, they can compile the MET software for Python embedding by passing the **\-\-enable-python** opotion to the **configure** script on the command line. This will link the MET C++ code directly to the Python libraries. The **NumPy** and **netCDF4** Python packages are required by Python scripts included with the MET software that facilitate the passing of data in memory and the reading and writing of temporary files with Python embedding is used.

In addition to using **\-\-enable-python** with **configure** as mentioned above, the following environment variables must also be set prior to executing **configure**: **MET_PYTHON_BIN_EXE**, **MET_PYTHON_CC**, and **MET_PYTHON_LD**. These may either be set as environment variables or as command line options to **configure**. These environment variables are used when building MET to enable the compiler to find the requisite Python executable, header files, and libraries in the user's local filesystem. Fortunately, Python provides a way to set these variables properly. This frees the user from the necessity of having any expert knowledge of the compiling and linking process. Along with the **Python** executable in the users local Python installation, there should be another executable called **python3-config**, whose output can be used to set these environment variables as follows:

• Set **MET_PYTHON_BIN_EXE** to the full path of the desired Python executable.

• On the command line, run "**python3-config --cflags**". Set the value of **MET_PYTHON_CC** to the output of that command.

• Again on the command line, run "**python3-config --ldflags**". Set the value of **MET_PYTHON_LD** to the output of that command.

Make sure that these are set as environment variables or that you have included them on the command line prior to running **configure**.


Controlling Which Python MET uses with MET_PYTHON_EXE
=====================================================

When MET is compiled with Python embedding support, MET uses the Python executable in that Python installation by default when Python embedding is used. However, for users of highly configurable Python environments, the Python instance set at compilation time may not be sufficient. Users may want to use an alternate Python installation if they need additional packages not available in the Python installation used when compiling MET. In MET versions 9.0+, users have the ability to use a different Python executable when running MET than the version used when compiling MET by setting the environment variable **MET_PYTHON_EXE**.

If a user's Python script requires packages that are not available in the Python installation used when compiling the MET software, they will encounter a runtime error when using MET. In this instance, the user will need to change the Python MET is using to a different installation with the required packages for their script. It is the responsibility of the user to manage this Python installation, and one popular approach is to use a custom Anaconda (Conda) Python environment. Once the Python installation meeting the user's requirements is available, the user can force MET to use it by setting the **MET_PYTHON_EXE** environment variable to the full path of the Python executable in that installation. For example:

.. code-block:: none

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

Python Embedding for 2D gridded dataplanes
------------------------------------------

We now describe how to write Python scripts so that the MET tools may extract 2D gridded data fields from them. Currently, MET offers two ways to interact with Python scripts: by using NumPy N-dimensional arrays (ndarrays) or by using Xarray DataArrays. The interface to be used (NumPy or Xarray) is specified on the command line (more on this later). The user's scripts can use any Python libraries that are supported by the local Python installation, or any personal or institutional libraries or code that are desired in order to implement the Python script, so long as the data has been loaded into either a NumPy ndarray or an Xarray DataArray by the end of the script. This offers advantages when using data file formats that MET does not directly support. If there is Python code to read the data format, the user can use those tools to read the data, and then copy the data into a NumPy ndarray or an Xarray DataArray. MET can then ingest the data via the Python script. Note that whether a NumPy ndarray or an Xarray DataArray is used, the data should be stored as double precision floating point numbers. Using different data types, such as integers or single precision floating point numbers, will lead to unexpected results in MET.

**Using NumPy N-dimensional Arrays**

The data must be loaded into a 2D NumPy ndarray named **met_data**. In addition there must be a Python dictionary named **attrs** which contains metadata such as timestamps, grid projection and other information. Here is an example **attrs** dictionary:

.. code-block:: none

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

In the **attrs** dictionary, valid time, initialization time, lead time and accumulation time (if any) must be indicated by strings. Valid and initialization times must be given in YYYYMMDD[_HH[MMSS]] format, and lead and accumulation times must be given in HH[MMSS] format, where the square brackets indicate optional elements. The dictionary must also include strings for the name, long_name, level, and units to describe the data. The rest of the **attrs** dictionary gives the grid size and projection information in the same format that is used in the netCDF files written out by the MET tools. Those entries are also listed below. Note that the **grid** entry in the **attrs** dictionary can either be defined as a string or as a dictionary itself.

If specified as a string, **grid** can be defined as follows:

• As a named grid:

.. code-block:: none

  'grid': 'G212'

• As a grid specification string, as described in :ref:`appendixB`:

.. code-block:: none

  'grid': 'lambert 185 129 12.19 -133.459 -95 40.635 6371.2 25 25 N'

• As the path to an existing gridded data file:

.. code-block:: none

  'grid': '/path/to/sample_data.grib'

When specified as a dictionary, the contents of the **grid** dictionary vary based on the grid **type** string. The entries for the supported grid types are described below:

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

**Using Xarray DataArrays**

To use Xarray DataArrays, a similar procedure to the NumPy case is followed. The Xarray DataArray can be represented as a NumPy N-dimensional array (ndarray) via the **values** property of the DataArray, and an **attrs** property that contains a dictionary of attributes. The user must name the Xarray DataArray to be **met_data**. When one of the MET tools runs the Python script, it will look for an Xarray DataArray named **met_data**, and will retrieve the data and metadata from the **values** and **attrs** properties, respectively, of the Xarray DataArray. The Xarray DataArray **attrs** dictionary is populated in the same way as for the NumPy interface (please see :ref:`pyembed-2d-data` for requirements of each entry in the **attrs** dictionary). The **values** NumPy ndarray property of the Xarray DataArray is also populated in the same way as the NumPy case.

.. note::
   Currently, MET does not support Xarray Dataset structures. If you have a Dataset in Xarray, you can create a DataArray of a single variable using:

   met_data = xr.DataArray(ds.varname,attrs=ds.attrs)

   | ds = your Dataset name
   | varname = variable name in the Dataset you'd like to use in MET

__________________

It remains to discuss command lines and config files. Two methods for specifying the Python command and input file name are supported. 

**Python Embedding Option 1:**

On the command line for any of the MET tools which will be obtaining its data from a Python script rather than directly from a data file, the user should specify either PYTHON_NUMPY or PYTHON_XARRAY wherever a (forecast or observation) data file name would normally be given. Then in the **name** entry of the config file dictionaries for the forecast or observation data, the user should list the Python script to be run followed by any command line arguments for that script. Note that for tools like MODE that take two data files, it would be entirely possible to use the NumPy interface for one file and the Xarray interface for the other.

___________________

Listed below is an example of running the Plot-Data-Plane tool to call a Python script for data that is included with the MET release tarball. Assuming the MET executables are in your path, this example may be run from the top-level MET source code directory.

.. code-block:: none

  plot_data_plane PYTHON_NUMPY fcst.ps \
    'name="scripts/python/examples/read_ascii_numpy.py data/python/fcst.txt FCST";' \
    -title "Python enabled plot_data_plane"
    
The first argument for the Plot-Data-Plane tool is the gridded data file to be read. When calling a NumPy Python script, set this to the constant string PYTHON_NUMPY. The second argument is the name of the output PostScript file to be written. The third argument is a string describing the data to be plotted. When calling a Python script, set **name** to the Python script to be run along with command line arguments. Lastly, the **-title** option is used to add a title to the plot. Note that any print statements included in the Python script will be printed to the screen. The above example results in the following log messages.

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

**Python Embedding Option 2 using MET_PYTHON_INPUT_ARG:**

The second option was added to support the use of Python embedding in tools which read multiple input files. Option 1 reads a single field of data from a single source, whereas tools like Ensemble-Stat, Series-Analysis, and MTD read data from multiple input files. While option 2 can be used in any of the MET tools, it is required for Python embedding in Ensemble-Stat, Series-Analysis, and MTD.

On the command line for any of the MET tools, specify the path to the input gridded data file(s) as the usage statement for the tool indicates. Do **not** substitute in PYTHON_NUMPY or PYTHON_XARRAY on the command line. In the config file dictionary set the **file_type** entry to either PYTHON_NUMPY or PYTHON_XARRAY to activate the Python embedding logic. Then, in the **name** entry of the config file dictionaries for the forecast or observation data, list the Python script to be run followed by any command line arguments for that script. However, in the Python command, replace the name of the input gridded data file with the constant string MET_PYTHON_INPUT_ARG. When looping over multiple input files, the MET tools will replace that constant **MET_PYTHON_INPUT_ARG** with the path to the file currently being processed. The example plot_data_plane command listed below yields the same result as the example shown above, but using the option 2 logic instead.

The Ensemble-Stat, Series-Analysis, and MTD tools support the use of file lists on the command line, as do some other MET tools. Typically, the ASCII file list contains a list of files which actually exist on your machine and should be read as input. For Python embedding, these tools loop over the ASCII file list entries, set MET_PYTHON_INPUT_ARG to that string, and execute the Python script. This only allows a single command line argument to be passed to the Python script. However multiple arguments may be concatenated together using some delimiter, and the Python script can be defined to parse arguments using that delimiter. When file lists are constructed in this way, the entries will likely not be files which actually exist on your machine. In this case, users should place the constant string "file_list" on the first line of their ASCII file lists. This will ensure that the MET tools will parse the file list properly.

.. code-block:: none
		
  plot_data_plane data/python/fcst.txt fcst.ps \
    'name="scripts/python/examples/read_ascii_numpy.py MET_PYTHON_INPUT_ARG FCST"; \
     file_type=PYTHON_NUMPY;' \
    -title "Python enabled plot_data_plane"

.. _pyembed-point-obs-data:

Python Embedding for Point Observations
---------------------------------------

The ASCII2NC tool supports the "-format python" option. With this option, point observations may be passed as input. An example of this is shown below:

.. code-block:: none

                ascii2nc -format python \
                "python/examples/read_ascii_point.py data/sample_obs/ascii/sample_ascii_obs.txt" \
                sample_ascii_obs_python.nc

The Point2Grid, Plot-Point-Obs, Ensemble-Stat, and Point-Stat tools also process point observations. They support Python embedding of point observations directly on the command line by replacing the input MET NetCDF point observation file name with the Python command to be run. The Python command must begin with the prefix 'PYTHON_NUMPY=' and be followed by the path to the User's Python script and any arguments. The full command should be enclosed in single quotes to prevent embedded whitespace from causing parsing errors. An example of this is shown below:

.. code-block:: none

                plot_point_obs \
                "PYTHON_NUMPY=python/examples/read_ascii_point.py data/sample_obs/ascii/sample_ascii_obs.txt" \
                output_image.ps

Both of the above examples use the **read_ascii_point.py** sample script which is included with the MET code. It reads ASCII data in MET's 11-column point observation format and stores it in a Pandas DataFrame to be read by the MET tools using Python embedding for point data. The **read_ascii_point.py** sample script can be found in:

• MET installation directory in *scripts/python/examples*.

• `MET GitHub repository <https://github.com/dtcenter/MET>`_ in *scripts/python/examples*.

.. _pyembed-mpr-data:

Python Embedding for MPR data
-----------------------------

The Stat-Analysis tool supports the "-lookin python" option. With this option, matched pair (MPR) data may be passed as input. An example of this is provided in :numref:`StA-pyembed`. That example uses the **read_ascii_mpr.py** sample script which is included with the MET code. It reads MPR data and stores it in a Pandas dataframe to be read by the Stat-Analysis tool with Python.

The **read_ascii_mpr.py** sample script can be found in:

• MET installation directory in *scripts/python/examples*.

• `MET GitHub repository <https://github.com/dtcenter/MET>`_ in *MET/scripts/python/examples*.
