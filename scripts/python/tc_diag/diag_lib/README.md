# diag_lib
Library of routines to support TC diagnostic code.

## Install
The library can be installed with the command:
```bash
pip install git+ssh://git@bear.cira.colostate.edu/tc_diagnostic/diag_lib.git
```

A particular version of the library can be installed with a command such as:
```bash
pip install git+ssh://git@bear.cira.colostate.edu/tc_diagnostic/diag_lib.git@v0.1.0
```

## Environment
Two environments are provided: `diag_lib_dev` in `environment_dev.yml` and 
`diag_lib_test` in `environment_test.yml`.  These provide the environment used
to develop the library and a minimal environment that can be used for testing.

## Running Tests
Assuming you have the `diag_lib_test` environment activated, you can use the
command `python -m unittest discover -s tests -p "*test_*"` to run all of the
automated tests.