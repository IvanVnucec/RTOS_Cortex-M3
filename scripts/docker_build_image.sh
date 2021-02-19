#!/bin/bash

# load constants
. ./scripts/constants.env

# should be eqal to the docker_run_container.sh


# build and run Docker image
echo "Building Docker containter."
docker build -t="$IMAGE_NAME" .
exit $?
