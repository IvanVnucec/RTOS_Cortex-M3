#!/bin/bash

# load constants
. ./scripts/constants.env

if [ -d "$LOGS_DIR" ]; then
    retval="$( tail -n 1 $LOGS_DIR/stdout.log )"
    retval=${retval::1}

    if [ ${retval::1} = 0 ]; then
        echo "Test PASS."
        exit 0
    else
        echo "Test FAILED."
        exit 1
    fi

else
    echo "ERROR: No $LOGS_DIR folder."
    exit 2
fi
