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
