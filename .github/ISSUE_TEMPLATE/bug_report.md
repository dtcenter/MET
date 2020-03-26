---
name: Bug report
about: Fix something that's not working
title: ''
labels: 'type: bug'
assignees: ''

---

*Replace italics below with details for this issue.*

## Describe the Problem ##
*Provide a clear and concise description of the bug here.*

### Expected Behavior ###
*Provide a clear and concise description of what you expected to happen here.*

### Environment ###
Describe your runtime environment:
*1. Machine: (e.g. HPC name, Linux Workstation, Mac Laptop)*
*2. OS: (e.g. RedHat Linux, MacOS)*
*3. Software version number(s)*

### To Reproduce ###
Describe the steps to reproduce the behavior:
*1. Go to '...'*
*2. Click on '....'*
*3. Scroll down to '....'*
*4. See error*
*Post relevant sample data following these instructions:*
*https://dtcenter.org/community-code/model-evaluation-tools-met/met-help-desk#ftp*

### Relevant Deadlines ###
*List relevant project deadlines here or state NONE.*

### Funding Source ###
*Define the source of funding and account keys here or state NONE.*

## Define the Metadata ##

### Assignee ###
- [ ] Select **engineer(s)** or **no engineer** required
- [ ] Select **scientist(s)** or **no scientist** required

### Labels ###
- [ ] Select **component(s)**
- [ ] Select **priority**
- [ ] Select **requestor(s)**

### Projects and Milestone ###
- [ ] Review **projects** and select relevant **Repository** and **Organization** ones
- [ ] Select **milestone**

## Define Related Issue(s) ##
Consider the impact to the other METplus components.
- [ ] Define related issue(s) for [METplus](https://github.com/NCAR/METplus/issues/new/choose), [MET](https://github.com/NCAR/MET/issues/new/choose), [METdb](https://github.com/NCAR/METdb/issues/new/choose), [METviewer](https://github.com/NCAR/METviewer/issues/new/choose), [METexpress](https://github.com/NCAR/METexpress/issues/new/choose), [METcalcpy](https://github.com/NCAR/METcalcpy/issues/new/choose), and/or [METplotpy](https://github.com/NCAR/METplotpy/issues/new/choose).

## Bugfix Checklist ##
See the [METplus Workflow](https://ncar.github.io/METplus/Contributors_Guide/github_workflow.html) for details.
- [ ] Complete the issue definition above.
- [ ] Fork this repository or branch **master_\<Current Version Number\>** into **bugfix_\<GitHub Issue Number\>_master_\<Current Version Number\>_\<Brief Description\>**.
- [ ] Fix the bug and test your changes.
- [ ] Add/update unit tests.
- [ ] Add/update documentation.
- [ ] Push local changes to GitHub.
- [ ] Submit a pull request to merge into **develop**, listing the **\<GitHub Issue Number\>** in the title.
- [ ] Iterate until the reviewer(s) accept and merge your changes.
- [ ] Delete your fork or branch.
- [ ] Complete the steps above to fix the bug for **develop** using a fork or branch named **bugfix_\<GitHub Issue Number\>_develop_\<Brief Description\>**
- [ ] Close this issue.
