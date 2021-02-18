#!/bin/bash
# load constants
. constants.env

# check if build folder exists
echo "Searching for $ELF_FILE file in $BUILD_DIR_NAME folder..."
if [ -d "$BUILD_DIR_NAME" ]; then
    cd $BUILD_DIR_NAME
    
    # check if .elf file exists
    if [ -f "$ELF_FILE" ]; then
        echo "SUCCESS: $ELF_FILE file exists!"
        exit 0
    else
        echo "ERROR: $ELF_FILE file does not exist!"
        exit 1
    fi
else
    echo "ERROR: $BUILD_DIR_NAME folder does not exist!"
    exit 2
fi
