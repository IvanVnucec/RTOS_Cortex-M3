#!/bin/bash
# load constants
. ./scripts/constants.env

echo "Running Docker container."
docker run --rm -v "$(pwd):/app" $CONTAINER_NAME
exit $?
