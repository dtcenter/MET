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
- `Documentation (documentation.yml) <https://metplus.readthedocs.io/en/latest/Contributors_Guide/continuous_integration.html#documentation-documentation-yml>`_ to check for problems building the documentation.
- `SonarQube (sonarqube.yml) <https://metplus.readthedocs.io/en/latest/Contributors_Guide/continuous_integration.html#sonarqube-sonarqube-yml>`_ to perform static code analysis.
- `Build Docker Image and Trigger METplus Workflow (build_docker_and_trigger_metplus.yml) <https://metplus.readthedocs.io/en/latest/Contributors_Guide/continuous_integration.html#build-docker-image-and-trigger-metplus-workflow-build-docker-trigger-metplus-yml>`_ to confirm that changes to METplus components do not break existing METplus use cases.
- `Release Checksum (release-checksum.yml) <https://metplus.readthedocs.io/en/latest/Contributors_Guide/continuous_integration.html#release-checksum-release-checksum-yml>`_ to create and attach checksum file assets to software releases.
- `Release Docker Images (release-docker-images.yml) <https://metplus.readthedocs.io/en/latest/Contributors_Guide/continuous_integration.html#release-docker-images-release-docker-images-yml>`_ to routinely recreate Docker images for recent software releases to minimize vulnerabilities.
- `Update Truth (update_truth.yml) <https://metplus.readthedocs.io/en/latest/Contributors_Guide/continuous_integration.html#update-reference-branch-update-reference-branch-yml>`_ to update the reference truth data used by the testing workflow.

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
