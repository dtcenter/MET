# SonarQube scripts

This directory holds the scripts that run the SonarQube Source Code Analyzer
against MET and upload the results to the DTC SonarQube server.

| Script | Purpose |
| --- | --- |
| `run_sonarqube.sh` | Check out a branch, tag, or hash, build it under the SonarQube build wrapper, and run the scanner. |
| `run_nightly.sh` | Wrapper that calls `run_sonarqube.sh` from cron and emails the team on failure. |
| `sonar-project.properties` | Scanner configuration: the source paths, the bison-generated files to exclude, and the path to the build wrapper's `compile_commands.json`. |

## Retrieving findings from the server

The scanner only uploads an analysis report. To pull the resulting findings
back down, including the justifications recorded for issues that have been
accepted or marked a false positive, use `fetch_sonarqube_findings.sh` in the
**private** [METplus-Internal](https://github.com/dtcenter/METplus-Internal)
repository, under `scripts/sonarqube/`.
