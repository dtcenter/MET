# MET repository conventions

Guidance for Claude Code and other AI assistants working in this repository,
and a useful summary for human contributors. The authoritative process
documentation is the [METplus Contributor's
Guide](https://metplus.readthedocs.io/en/latest/Contributors_Guide/index.html);
this file records the conventions that are easy to miss.

## Repository layout

- `src/` holds the C++ source. `src/basic`, `src/libcode`, and `src/tools`
  separate the utility libraries, the MET libraries, and the tools themselves.
- `internal/test_unit/` holds the unit tests: `xml/unit_<tool>.xml` drives each
  tool, `config/` holds the configuration files those cases read, and
  `python/unit.py` runs them.
- `docs/Users_Guide/` is the Sphinx documentation, including
  `release-notes.rst`.
- `internal/scripts/installation/compile_MET_all.sh` builds the external
  libraries and then MET.
- `.github/workflows/` holds the CI. Note that the SonarQube scan covers only
  `src`, `internal`, `scripts/python`, and `data/wrappers`, so nothing under
  `.github/` is ever scanned.

## Building

Run these from the top of the source tree, in one shell so the environment
carries through. The development environment file is specific to each machine.

```
source <your development environment file>
touch aclocal.m4; sleep 1
touch configure config.h.in; sleep 1
git ls-files | grep 'Makefile\.in$' | xargs touch
./configure --prefix=`pwd` --enable-all
make -j8 install
```

The `touch` steps keep the autoconf and automake outputs newer than their
inputs, so make does not regenerate them and rewrite tracked files.

### Relinking after a library change

`make install` does not relink the tools when only a library changed. The tool
Makefiles pull libraries in through `-L` and `-l` flags, so make has no
dependency on the `.a` files: edit a file under `src/libcode/`, run
`make -j8 install`, and the archive is rebuilt while every binary under `bin/`
keeps the code it was linked against. The build reports success and the tool
goes on running the old library.

Force the relink by deleting both copies of the binary, the one in the tool's
build directory and the installed one, then building that tool:

```
rm -f src/tools/other/plot_data_plane/plot_data_plane bin/plot_data_plane
make -C src/tools/other/plot_data_plane install
```

Before trusting a before-and-after comparison, confirm the binary contains the
change by grepping the diff for an added string literal and looking for it with
`strings bin/<tool> | grep`. A new function name is not a usable check: it
appears in the symbol table but not in `strings` output unless the code prints
it.

## Unit tests

`internal/test_unit/xml/unit_<tool>.xml` carries the exact command for every
case, so running one case directly is far faster than driving the whole suite
and is the quickest way to reproduce a reported defect. Export
`MET_TEST_INPUT`, `MET_TEST_OUTPUT`, and `MET_BASE`, expand the XML entities,
and run `bin/<tool>` directly.

`internal/test_unit/python/unit.py` judges a case on the process exit code
against `retval`, plus the files named in `<output>`. It does not compare log
text, so extra warnings or debug lines do not affect pass or fail. The
regression comparison, `comp_dir.py`, diffs the output directory against truth,
so a change to an output file needs a truth update while a change that leaves
the output files identical needs none.

Prefer extending an existing case over adding a new one. A case that already
reads the right file type and requests several fields often needs only one more
argument to cover a new defect, and it brings its downstream consumers with it.
Confirm that a regression test actually fails without the fix: run the modified
case against a build of the unchanged base branch first.

The unit tests leave untracked files in the source tree, including
`compute_tc_diag_py.log` and `dataplane_py.err` at the top level and
`compute_tc_diag_py.log` under `internal/test_unit/`. **Stage files by name in a
tree that has been used for testing, never with `git add -A`.** The commit looks
clean in `git status` because these files are genuinely new. That is how a
277-line `unit.py -cmd` script carrying one developer's absolute paths reached
`develop` in #3431 and sat there unreferenced for two months.

## Documentation

`docs/conf.py` requires the `sphinx_design` and `sphinx_rtd_theme` extensions,
which are usually not installed alongside a development Python. A throwaway
virtual environment is enough:

```
python3 -m venv /tmp/docsenv
/tmp/docsenv/bin/pip install sphinx sphinx_design sphinx_rtd_theme
/tmp/docsenv/bin/sphinx-build -b html docs /tmp/html -q -w /tmp/warnings.txt
```

The pull request template asks whether the documentation builds with no errors
or warnings, so run this whenever a pull request touches `docs/`. Check that the
new text rendered in the generated HTML rather than trusting the exit status: a
malformed directive can be skipped without failing the build.

## Pull requests

- Title the pull request after the branch, writing the issue number with a `#`
  so GitHub links it: `feature_3443_sq_cpp_S5025` gives
  `Feature #3443 sq_cpp_S5025`. When the issue lives in another repository,
  prefix it, as in `Feature dtcenter/METplus#3408 ...`, because a bare `#3408`
  silently links to whatever carries that number here.
- Fill in every item of `.github/pull_request_template.md`. Answer what the pull
  request can answer and leave the rest for the author rather than deleting
  them; a missing item reads as an oversight and drops a question the project
  meant to ask.
- Immediately after the change summary, list the files the pull request
  **adds**, **renames** (as `old/path -> new/path`), and **removes**, in that
  order, as separate lists, from
  `git diff --name-status --find-renames <base>...HEAD`. Do not list modified
  files. Omit the section when there are none.
- When source files that affect the build are added, renamed, or removed, tag
  @georgemccabe so he is notified. That means files the build system compiles or
  installs, such as sources and headers under `src/` and `Makefile.am` files,
  not files read only at runtime or by the test suite.
- Link the pull request to its issue with a closing keyword such as
  `Fixes #123`, and also set the link by hand in the **Development** section of
  the sidebar. GitHub links and auto-closes from the keyword only when the pull
  request targets the default branch.

### Release notes and upgrade instructions

Add an entry to `docs/Users_Guide/release-notes.rst` under the current version,
and update the upgrade instructions when someone upgrading from the last
release would have to do something differently.

A pull request that only changes **unreleased** functionality needs neither.
Both exist to tell someone upgrading from the last release what is different for
them, so a defect introduced and fixed within the same development cycle is
invisible to them. The test is whether a user of the previous release would
notice, not whether the work fixed something.

- Judge it by behavior a user can observe: output values, file contents, column
  meanings, configuration syntax, command line arguments, exit status, or
  messages at the default verbosity. Changes visible only at raised verbosity,
  in debug output, or in the build and test machinery do not count.
- Say so explicitly in the pull request description, with the reasoning, so a
  reviewer does not have to work out whether the omission was deliberate.
- To settle whether a defect is unreleased, check whether the commit that
  introduced it ever reached a release branch:
  `git branch -r --contains <commit> | grep main_v` returning nothing, together
  with the affected file on `origin/main_vX.Y` still showing the old behavior,
  is enough to state it without hedging.
- When it is genuinely unclear whether a user would notice, ask the author
  rather than writing an entry just to be safe. A release note for something
  nobody experienced makes the real entries harder to find.

## Reading the logs

Tool logs are where defects show themselves first and are easy to skim past.
When a change is verified by running something before and after, diff the two
logs and account for any line that appeared, disappeared, or changed wording,
and for any run whose elapsed time moved substantially. Report the numbers
rather than a verdict.

The unit test logs hold no MET tool output unless it is asked for: `unit.py`
captures each tool's output and emits it through `logger.debug()`, whose handler
sits at INFO unless `-log` is given. Use `-verbose` to capture it, either on the
whole suite with `internal/test_unit/bin/unit_test.sh -verbose` or on one file
with `unit.py -verbose`, and expect roughly 35 MB per full run. Compare two runs
by grouping `WARNING:` lines by message text, after collapsing paths, numbers,
and valid times, rather than by comparing totals. Expect the counts not to
balance: a warning downgraded to `Debug(3)` disappears from any case running at
the default verbosity of 2.

## Inspecting NetCDF output

A MET build can leave the wrong `ncdump` ahead of netCDF's on `PATH`, because
the external libraries install HDF4 and netCDF into the same prefix and HDF4
ships an `ncdump` of its own. When that prefix comes first, `ncdump file.nc`
reports `ncopen failed` on a perfectly valid NetCDF-4 file. Before concluding
that a file is bad, run `ncdump -k file.nc`: netCDF's prints `netCDF-4`, while
HDF4's rejects the flag and prints a usage block.
