ARG MET_BASE_IMAGE=minimum

FROM dtcenter/met-base:${MET_BASE_IMAGE}
MAINTAINER John Halley Gotway <johnhg@ucar.edu>

#
# This Dockerfile checks out MET from GitHub and compiles the specified branch or tag from source.
#
ARG SOURCE_BRANCH

#
# SOURCE_BRANCH is not defined when built via Docker Hub.
#
RUN if [ "x${SOURCE_BRANCH}" = "x" ]; then \
      echo "ERROR: SOURCE_BRANCH undefined! Rebuild with \"--build-arg SOURCE_BRANCH={branch name}\""; \
      exit 1; \
    else \
      echo "Build Argument SOURCE_BRANCH=${SOURCE_BRANCH}"; \
    fi

ENV MET_GIT_NAME    ${SOURCE_BRANCH}
ENV MET_REPO_DIR    /met/MET-${MET_GIT_NAME}
ENV MET_GIT_URL     https://github.com/dtcenter/MET
ENV MET_DEVELOPMENT true

#
# Set the working directory.
#
WORKDIR /met

#
# Download and install MET and GhostScript fonts.
# Delete the MET source code for tagged releases matching "v"*.
#
RUN echo "Checking out MET ${MET_GIT_NAME} from ${MET_GIT_URL}" \
 && git clone ${MET_GIT_URL} ${MET_REPO_DIR} \
 && cd ${MET_REPO_DIR} \
 && git checkout ${MET_GIT_NAME} \
 && ../scripts/docker/build_met_docker.sh
