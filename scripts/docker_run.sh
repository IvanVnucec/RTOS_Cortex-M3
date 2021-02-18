#!/bin/bash
# load constants
. ./scripts/constants.env

# recreate build folder, needed for Docker mount
rm -rf $HOST_BUILD_DIR
mkdir $HOST_BUILD_DIR

# build and run Docker image
echo "Building Docker containter."
sudo docker build -t="$CONTAINER_NAME" .
echo "Running Docker container."
sudo docker run --rm -it --mount type=bind,source=$HOST_BUILD_DIR,target=$TARGET_BUILD_DIR $CONTAINER_NAME
echo "Done running Docker container."
# exit success
exit 0
