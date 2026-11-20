# MET repository conventions

Guidance for AI coding agents working in this repository, and a useful summary
for human contributors.

## Read the Contributor's Guides first

Two guides govern work in this repository. Read them before opening an issue or
a pull request, and follow them wherever they and this file overlap.

**The [METplus Contributor's
Guide](https://metplus.readthedocs.io/en/latest/Contributors_Guide/index.html)**
covers the process shared by every METplus component, MET included. Its [GitHub
Workflow](https://metplus.readthedocs.io/en/latest/Contributors_Guide/github_workflow.html)
chapter is the one that applies here in full: how issues, labels, milestones,
and project boards are used, how feature branches are named, and how pull
requests are opened, reviewed, and merged. Follow it rather than inventing a
process. Its Coding Standards, Testing, and Continuous Integration chapters
describe the METplus Python wrappers, meaning PEP8, pytest under
`internal/tests`, and the METplus workflows, and do not apply to MET.

**The [MET Contributor's
Guide](https://metplus.readthedocs.io/projects/met/en/latest/Contributors_Guide/index.html)**,
whose source is `docs/Contributors_Guide/` in this repository, is where MET's
own conventions live: the
[development environment](https://metplus.readthedocs.io/projects/met/en/latest/Contributors_Guide/dev_env.html),
[testing](https://metplus.readthedocs.io/projects/met/en/latest/Contributors_Guide/testing.html)
with the unit tests under `internal/test_unit`,
[continuous integration](https://metplus.readthedocs.io/projects/met/en/latest/Contributors_Guide/continuous_integration.html)
for the workflows in `.github/workflows`,
[code profiling](https://metplus.readthedocs.io/projects/met/en/latest/Contributors_Guide/code_profiling.html),
and development details such as temporary file use and static data files.
Several of its chapters, [Coding
Standards](https://metplus.readthedocs.io/projects/met/en/latest/Contributors_Guide/coding_standards.html)
among them, are still "Coming Soon!" stubs. That guide is where MET's coding
conventions belong, so as those chapters are filled in, move the relevant
material out of this file and into them, leaving only the pointer here.

This file is the short form: what neither guide covers yet, or what is easy to
miss in a MET tree. It deliberately does not repeat them. The guides are
authoritative on process and on conventions; this file is authoritative on the
practical traps of building, testing, and releasing MET recorded below. Where
this file and a guide genuinely conflict, say so in the pull request rather
than silently following one.

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

Check that `$MET_TEST_INPUT` resolves the way the XML expects before concluding
that an input file is missing. The XML builds its paths as
`${MET_TEST_INPUT}/model_data/...`, but the exported value may point one level
above that, with the data under a `unit_test` subdirectory. Searching for the
file named in the test case settles it faster than trusting the variable.

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
  meant to ask. Change `- [ ]` to `- [x]` on every item that has been answered
  or filled in: a `[Yes or No]` replaced with an answer, a "Describe..." item
  with text after it, a review date set. Leave unanswered items unchecked, along
  with the action items under **Pull Request Checklist**, which record steps
  taken rather than answers given, and check items again whenever more answers
  are supplied later.
- Immediately after the change summary, list the files the pull request
  **adds**, **renames** (as `old/path -> new/path`), and **removes**, in that
  order, as separate lists, from
  `git diff --name-status --find-renames <base>...HEAD`. Do not list modified
  files. Omit the section when there are none.
- When one change goes to two base branches as two pull requests, write the
  narrative once rather than twice. The pull request targeting `develop` is the
  primary one and carries the full **Change Summary**, testing description, and
  release note reasoning; where neither targets `develop`, the first one opened
  is primary. The secondary pull request gives its base branch, links the
  primary, and states either that the changes are identical or exactly what
  differs. Everything the template asks per pull request stays in both bodies:
  the review-by date, the SonarQube and Use Cases answers, and the whole **Pull
  Request Checklist**, since the milestone, project board, and development cycle
  are chosen to match each base branch. Two full copies of one description leave
  a reviewer unable to tell whether they are looking at one change or two.
- When source files that affect the build are added, renamed, or removed, remind
  the author to tag @georgemccabe so he is notified. That means files the build
  system compiles or installs, such as sources and headers under `src/` and
  `Makefile.am` files, not files read only at runtime or by the test suite.
  Write his username in the pull request only when such a file was actually
  added, renamed, or removed. When none were, say nothing about the tag at all,
  in any form. Naming him to report that no tag is needed sends him exactly the
  notification this rule exists to ration.
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
the default verbosity of 2. The same applies in reverse to `DEBUG 4` output,
where skipping a call to `process_data_plane()` removes its `Grid information:`
and `Data plane information:` blocks, which can move the line count by tens of
thousands without any user-visible change.

## Inspecting NetCDF output

A MET build can leave the wrong `ncdump` ahead of netCDF's on `PATH`, because
the external libraries install HDF4 and netCDF into the same prefix and HDF4
ships an `ncdump` of its own. When that prefix comes first, `ncdump file.nc`
reports `ncopen failed` on a perfectly valid NetCDF-4 file. Before concluding
that a file is bad, run `ncdump -k file.nc`: netCDF's prints `netCDF-4`, while
HDF4's rejects the flag and prints a usage block.

## What the SonarQube scan covers

`internal/scripts/sonarqube/sonar-project.properties` sets
`sonar.sources=src,internal,scripts/python,data/wrappers`. Anything outside
those paths is never scanned, `.github` included, so the CI job scripts under
`.github/jobs/` carry no findings however many a rule would raise in them. Note
that the SonarQube Cloud project is configured separately and does analyze
`.github/jobs`, so the two can disagree.

A file being inside `sonar.sources` does not mean the analyzer read it either.
`internal/scripts/installation/compile_MET_all.sh` is a 952-line bash script
under `internal/` with no exclusion covering it, and the shell analyzer reported
zero findings in it while flagging the same rule 53 times in the smaller scripts
beside it. The likeliest explanation is that the analyzer failed to parse the
file and skipped it silently.

Read a zero count as "nothing reported" rather than "nothing there". Before
describing a rule as driven to zero, grep the tree for the pattern and account
for every file that does not appear in the findings.
