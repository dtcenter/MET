How to Use Dockerfiles
======================

GitHub actions are triggered for Pull Requests to the "develop" and "main_v*"
branches and for pushes to the branches "feature_*", "bugfix_*", "develop",
"develop-ref", and "main_v*" created in this repository to automatically build
images from these Dockerfiles and push them to DockerHub repositories.  The
MET Docker image is used in Dockerfiles in the METplus wrappers component
repository for testing.

Descriptions of the Dockerfiles in this directory are provided below. Sample commands for building these Docker images locally are also provided. Please note that these commands should be run from the top-level directory of
the MET repository.

1. `Dockerfile` checks out MET from GitHub, compiles the specified branch
or tag from source, sets a working directory, downloads and installs 
GhostScript fonts, and deletes the MET source code for tagged releases
matching "v"*. It uses Docker image for the base compilation environment for
MET from the
[dtcenter/met-base](https://hub.docker.com/repository/docker/dtcenter/met-base)
DockerHub repository.  Tagged versions are available in the
[dtcenter/met](https://hub.docker.com/repository/docker/dtcenter/met-base)
DockerHub repository.  This MET image can be built manually by running:
```
docker build -t dtcenter/met:${TAG_NAME} --build-arg SOURCE_BRANCH=${BRANCH_NAME} internal/scripts/docker
docker push dtcenter/met:${TAG_NAME}
```

where:
* `${TAG_NAME}` is the name of the DockerHub tag to create
* `${BRANCH_NAME}` is the MET branch to checkout

2. `Dockerfile.copy` compiles MET using the specified branch or tag from local
source code, sets a working directory, downloads and installs 
GhostScript fonts, and deletes the MET source code for tagged releases
matching "v"*. It uses a local Docker image for the base compilation
environment for MET from the
[METbaseimage](https://github.com/dtcenter/METbaseimage/)
GitHub repository. See the
[METbaseimage README.md](https://github.com/dtcenter/METbaseimage/blob/main/README.md)
file for more information on manually creating the Docker image for the base
compilation environment. This MET image can be built manually by running:
```
docker build -t dtcenter/met:${TAG_NAME} --build-arg SOURCE_BRANCH=${BRANCH_NAME} -f internal/scripts/docker/Dockerfile.copy .
docker push dtcenter/met:${TAG_NAME}
```

where:
* `${TAG_NAME}` is the name of the DockerHub tag to create
* `${BRANCH_NAME}` is the identifier to use for $MET_GIT_NAME inside image

3. `Dockerfile.copy` compiles MET using the specified branch or tag from local
source code, sets a working directory, downloads and installs MET and
GhostScript fonts, and deletes the MET source code for tagged releases
matching "v"*. It uses a local Docker image for the base compilation
environment along with the additional packages required for running the MET
unit tests from the
[METbaseimage](https://github.com/dtcenter/METbaseimage/)
GitHub repository. See the
[METbaseimage README.md](https://github.com/dtcenter/METbaseimage/blob/main/README.md)
file for more information on manually creating the Docker image with the base
compilation environment and with the additional packages required for running
the MET unit tests. This MET image can be built manually by running:
```
docker build -t dtcenter/met:${TAG_NAME} --build-arg SOURCE_BRANCH=${BRANCH_NAME} --build-arg MET_BASE_TAG=${MET_BASE_TAG} -f internal/scripts/docker/Dockerfile.copy .
docker push dtcenter/met:${TAG_NAME}
```

where:
* `${TAG_NAME}` is the name of the DockerHub tag to create
* `${BRANCH_NAME}` is the identifier to use for $MET_GIT_NAME inside image
* `${MET_BASE_TAG}` is the version of [dtcenter/met-base-unit-test](https://hub.docker.com/repository/docker/dtcenter/met-base-unit-test) to be used