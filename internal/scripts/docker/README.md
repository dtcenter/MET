# How to Use Dockerfiles

Run all of the Docker commands from the top-level directory of the MET repository

## Build image with minimum requirements needed to build MET

```docker build -t dtcenter/met-base:minimum -f internal/scripts/docker/Dockerfile.minimum .
docker push dtcenter/met-base:minimum```

## Build image with requirements to build MET and run MET unit tests

```docker build -t dtcenter/met-base:unit_test -f internal/scripts/docker/Dockerfile.test .
docker push dtcenter/met-base:unit_test```

## Build MET from clone

```docker build -t dtcenter/met:${TAG_NAME} --build-arg SOURCE_BRANCH=${BRANCH_NAME} internal/scripts/docker
docker push dtcenter/met:${TAG_NAME}```

where:
* TAG_NAME is the name of the DockerHub tag to create
* BRANCH_NAME is the MET branch to checkout

## Build MET from local source code with minimum requirements

```docker build -t dtcenter/met:${TAG_NAME} --build-arg SOURCE_BRANCH=${BRANCH_NAME} -f internal/scripts/docker/Dockerfile.copy .
docker push dtcenter/met:${TAG_NAME}```

where:
* TAG_NAME is the name of the DockerHub tag to create
* BRANCH_NAME is the identifier to use for $MET_GIT_NAME inside image

## Build MET from local source code with unit test requirements

```docker build -t dtcenter/met:${TAG_NAME} --build-arg SOURCE_BRANCH=${BRANCH_NAME} --build-arg MET_BASE_IMAGE=unit_test -f internal/scripts/docker/Dockerfile.copy .
docker push dtcenter/met:${TAG_NAME}```

where:
* TAG_NAME is the name of the DockerHub tag to create
* BRANCH_NAME is the identifier to use for $MET_GIT_NAME inside image
