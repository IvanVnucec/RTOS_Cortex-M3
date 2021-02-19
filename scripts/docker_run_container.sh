#!/bin/bash

# load constants
. ./scripts/constants.env

readonly HOST_WORKDIR=/app  # defined in Dockerfile

echo "Running Docker container."
docker run --rm -v "$(pwd):$HOST_WORKDIR" $IMAGE_NAME
exit $?
