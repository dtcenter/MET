*******
Testing
*******

make test
=========

Coming Soon!

Unit Tests
==========

Running Unit Tests
------------------

Coming Soon!

Input Data
----------

Input data used to run the MET unit tests in CI workflows are pulled from the DTC web server and stored on DockerHub.
On the web server, data is stored for each supported version, e.g. *v12.0*, *v12.1*, etc.
There is also a directory called *develop* that includes symbolic links to the latest version,
which is the version that is currently in development.
This is done so that the latest state of the input data is used for new development
and the final state of the input data is preserved when an official release is created.
The scripts described below can be used to easily update the input data and set up a new development cycle.

Setting up a new web server
^^^^^^^^^^^^^^^^^^^^^^^^^^^

.. note::

   These instructions require access to run commands as the *met_test* user on the DTC web server.

The GitHub Actions custom action
`metplus-action-data-update <https://github.com/dtcenter/metplus-action-data-update>`_
expects a specific URL defined in *update_data_volumes.py* script in its repo.
This directory should exist on the web server.
This can be a link to another directory, but the name must match the repo name, e.g. MET.
If this path must differ on a new web server, then modifications will be needed to the custom action.

The directory should also be linked from the *met_test* user's home directory with the name *MET_unit_test*.
This is done so the scripts used to update the input data and setup a new development cycle can
easily find the directory without having to change the path on a new machine.

It is recommended to clone the MET repository in the *met_test* user's home directory,
check out the *develop* branch, and create a symbolic link in the home directory to the scripts used to
update the input data and set up the next release directory.
This is not necessarily required, but makes it convenient to find and call the script.
::

    runas met_test
    cd ~/
    git clone --branch develop https://github.com/dtcenter/MET
    ln -s MET/internal/scripts/unit_test_ci/setup_met_next_release_data.sh
    ln -s MET/internal/scripts/unit_test_ci/update_met_unit_test_data.sh

The unit test input data directory contains directories for *develop* and each *vX.Y* version that is supported.
Each directory should contain a tarfile called **unit_test-all.tgz** and a file called **volume_mount_directories**.
Typically, the *develop* directory will contain symbolic links to the latest version's files.
The *vX.Y* directories will also contain a directory called unit_test that contains the input data.

If setting up a new web server,
create the *develop* directory and add the input data tarfile and volume mount files,
then run the script to setup the next development cycle (see :ref:`cg-testing-ut-id-setup`).
A sample volume mount file can be found in the MET repo under *internal/scripts/unit_test_ci*.


.. _cg-testing-ut-id-setup:

Setup next development cycle
^^^^^^^^^^^^^^^^^^^^^^^^^^^^

.. note::

   These instructions require access to run commands as the *met_test* user on the DTC web server.

Once the *main_vX.Y* branch for a release has been created, the *develop* branch will contain development
towards the next release. At this time, a new set of test data should be created for the next
release so that it can be updated while preserving the test data used for an official release.
For example, if the *main_v12.1* branch was created when the *12.1.0-rc1* release was created,
then a data directory to store data for *v13.0* (or similar) should be created.

Pull changes from develop to ensure that the latest version of script is used.
::

    runas met_test
    cd ~/MET
    git checkout develop
    git pull

Run the script, passing the *vX.Y* version of the next release as an argument.
If the script is linked from the home directory, run::

    ~/setup_met_next_release_data.sh v13.0

This will create the *v13.0* directory, copy the latest tarfile and volume mount files into *v13.0*,
extract the tarfile contents into the *v13.0*, and update the symbolic links in the *develop* directory
to point to the files in *v13.0*.

Adding new test files
^^^^^^^^^^^^^^^^^^^^^

.. note::

   These instructions require access to run commands as the *met_test* user on the DTC web server.

Updates to the input data, e.g. adding new test files, are made on the DTC web server.
The next time the MET CI unit tests are run,
the web server will be checked and the input data will be updated automatically.
Note that the unit tests are only run for develop/main branches or running via workflow dispatch.
A push event to a branch will not run the full unit test suite and therefore will not update the input data.

In the *MET_unit_test* directory, there is a directory called *unit_test*.
These files are the full set of fields and fields used for the unit tests.
**These files are used by the MET regression tests that are run locally.**

First, add any new files to the *unit_test* directory so they will be available to the MET regression tests.

Example::
    cp /path/to/my/file.ext MET_unit_test/unit_test/DIRNAME/

Next, add the new input files in the *unit_test* directory under the *vX.Y* directory that
corresponds to the current development cycle.

Example::
    cp /path/to/my/file.ext MET_unit_test/v23.1/unit_test/DIRNAME/

If any of the files are very large, consider creating a subset of these files.
For example, GRIB2 files can be subset with *wgrib2* and NetCDF files can be subset using NCO tools.
After the updates have been made, run the script to update the test data tarfile.

Pull changes from develop to ensure that the latest version of script is used.
::

    runas met_test
    cd ~/MET
    git checkout develop
    git pull

Run the script, passing the *vX.Y* version of the next release as an argument.
If the script is linked from the home directory, run::

    ~/update_met_unit_test_data.sh v13.0

This will save a copy the input data tarfile with the current date in YYYYMMDD format in case it needs to be recovered,
then create the tarfile using the contents of the *unit_test* directory.


Regression Tests
================

Coming Soon!