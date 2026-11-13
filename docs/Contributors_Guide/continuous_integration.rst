**********************
Continuous Integration
**********************

MET utilizes GitHub Actions to run processes automatically when changes
are pushed to GitHub.

GitHub Actions Workflows
========================

GitHub Actions runs workflows defined by files in the :code:`.github/workflows`
directory of a GitHub repository. Workflows that are run by multiple METplus
components are documented in the `METplus Contributor's Guide
<https://metplus.readthedocs.io/en/latest/Contributors_Guide/continuous_integration.html>`_.
These common workflows include:

- `Testing (testing.yml) <https://metplus.readthedocs.io/en/latest/Contributors_Guide/continuous_integration.html#testing-testing-yml>`_ to perform regression testing.
- `Documentation (documentation.yml) <https://metplus.readthedocs.io/en/latest/Contributors_Guide/continuous_integration.html#documentation-documentation-yml>`_ to check for problems building the documentation and for broken links.
- `SonarQube (sonarqube.yml) <https://metplus.readthedocs.io/en/latest/Contributors_Guide/continuous_integration.html#sonarqube-sonarqube-yml>`_ to perform static code analysis.
- `Build Docker Image and Trigger METplus Workflow (build_docker_and_trigger_metplus.yml) <https://metplus.readthedocs.io/en/latest/Contributors_Guide/continuous_integration.html#build-docker-image-and-trigger-metplus-workflow-build-docker-and-trigger-metplus-yml>`_ to confirm that changes to METplus components do not break existing METplus use cases.
- `Release Checksum (release-checksum.yml) <https://metplus.readthedocs.io/en/latest/Contributors_Guide/continuous_integration.html#add-checksum-to-release-release-checksum-yml>`_ to create and attach checksum file assets to software releases.
- `Release Docker Images (release-docker-images.yml) <https://metplus.readthedocs.io/en/latest/Contributors_Guide/continuous_integration.html#create-release-docker-images-release-docker-images-yml>`_ to routinely recreate Docker images for recent software releases to minimize vulnerabilities.
- `Update Truth (update_truth.yml) <https://metplus.readthedocs.io/en/latest/Contributors_Guide/continuous_integration.html#update-reference-branch-update-reference-branch-yml>`_ to update the reference truth data used by the testing workflow.

.. _cg-ci-testing:

Testing (testing.yml)
---------------------

The Testing workflow is documented in the METplus Contributor's Guide, linked above.
MET adds one input to it, which is available when the workflow is started manually
from the **Run workflow** menu on the **Actions** tab:

- **Capture the full MET tool output in the unit test logs** (*verbose_logs*),
  which defaults to false.

When enabled, the unit tests are run with the *-verbose* argument described in
:ref:`cg-testing-ut-verbose`, so that the log files uploaded as workflow artifacts
contain the output of each MET tool rather than just the pass or fail status of each test.
Enable it when a change is expected to affect the log output, for example to count how many
times a particular WARNING message is issued before and after a change.
It is off by default because it makes the log artifacts considerably larger.

.. _cg-ci-compilation_options:

Compilation Options (compilation_options.yml)
---------------------------------------------

MET supports multiple compile-time options which are specified as arguments to the
"configure" script. The Compilation Options workflow compiles MET multiple times to
confirm that each configuration results in a successful build. These configurations
include compiling MET with:

- All options enabled
- All options disabled
- Each individual option enabled by itself

This workflow confirms that MET compiles successfully with a variety of configurations
but does not run any unit tests or compare output against a truth dataset.

This workflow is run automatically on a schedule, weekly at 07:00 UTC on Sunday, and
whenever a tag is pushed. As with any scheduled workflow, the weekly run is driven by
the copy of the workflow file on the default branch, **develop**, and compiles that
branch. It can also be started manually from the **Run workflow** menu on the
**Actions** tab to check any other branch.

Running it manually is worth considering for a change that affects the build. Every
other MET workflow compiles with "--enable-all", so a defect that only appears for
some other combination of options, such as a header that is included transitively by
an optional dependency, is invisible to them and is caught here instead.

.. _cg-ci-claude:

Claude Code (claude.yml and claude-code-review.yml)
---------------------------------------------------

Two workflows run `Claude Code <https://claude.com/claude-code>`_ in the MET repository.
They perform different jobs and are invoked in different ways.

Claude Code (claude.yml)
^^^^^^^^^^^^^^^^^^^^^^^^

This workflow is an on-demand assistant. It runs only when the trigger phrase
:code:`@claude` appears in an issue, a pull request comment, or a pull request review,
and it then carries out whatever that comment asks. It has no fixed prompt, so it may be
asked to explain a failing job, summarize a change, or draft a suggestion. It is granted
read access to workflow results so that it can inspect CI output when asked about it.

Claude Code Review (claude-code-review.yml)
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^

This workflow is an automated reviewer. It runs a fixed code review prompt against the
changes in a pull request. Its output is a set of review comments anchored to specific
lines of the diff, followed by one top-level comment in the pull request conversation
that summarizes what the review flagged. When the review finds nothing, that summary
comment is the only comment it posts, so a review that found no issues reads differently
from one that never ran.

A review is performed:

- Automatically, once per pull request, when the pull request is opened or marked ready
  for review. It does not run again when more commits are pushed, so an iterative review
  cycle produces one review rather than one review per push.
- On demand, when the **claude-review** label is applied to a pull request. Remove and
  re-apply the label to request another review.
- On demand, from the **Run workflow** menu on the **Actions** tab, which takes the pull
  request number as an input. GitHub only offers this menu to users with write access to
  the repository.

A review is **not** performed:

- For pull requests whose base branch name ends in :code:`-ref`. These are the automated
  *Update <branch>-ref* pull requests created by the `Update Truth (update_truth.yml)
  <https://metplus.readthedocs.io/en/latest/Contributors_Guide/continuous_integration.html#update-reference-branch-update-reference-branch-yml>`_
  workflow, which merge accumulated history rather than code written for review.
- For draft pull requests, until they are marked ready for review.
- For pull requests opened from a fork. GitHub withholds repository secrets from such
  runs on public repositories, so the workflow cannot authenticate.

Authentication and Resource Use
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^

Both workflows authenticate using the :code:`CLAUDE_CODE_OAUTH_TOKEN` repository secret.
This token is tied to the Claude subscription of the person who generated it, and usage
from both workflows counts against that person's account rather than being billed to an
organization. Anyone changing how often these workflows run should be aware of whose
account is paying for them.

The assistant workflow requests read-only repository scopes. The review workflow
additionally requests :code:`pull-requests: write`, which is what allows it to post its
inline comments and its summary comment. Neither workflow writes to the contents of the
repository.

The review workflow also names the tools Claude is allowed to use, which are the ones the
code review plugin needs: the inline comment tool, the read-only :code:`gh` commands that
read the pull request, and :code:`gh pr comment` for the summary. Note that the action
runs in its auto permission mode, in which that list is an allow rule rather than a hard
restriction. A tool call the list does not cover is decided by the action's own safety
review rather than refused outright, so the list should be read as what the review is
expected to need rather than as a boundary it cannot cross.

Each workflow sets a job timeout, and the review workflow additionally limits the number
of turns Claude may take and cancels any superseded run for the same pull request.
