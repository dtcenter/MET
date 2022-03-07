FROM centos:7
MAINTAINER George McCabe <mccabe@ucar.edu>

ENV OUTPUT_DIR /data/output/met_test_truth
RUN mkdir -p ${OUTPUT_DIR}

ARG TRUTH_DIR

COPY ${TRUTH_DIR} ${OUTPUT_DIR}/

ARG TRUTH_DIR

# Define the volume mount point
VOLUME ${OUTPUT_DIR}

USER root
CMD ["true"]