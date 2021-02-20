#!/bin/bash

retval="$( tail -n 1 logs/stdout.log )"
retval=${retval::1}

if [ ${retval::1} = 0 ]; then
    echo "Test PASS."
    exit 0
else
    echo "Test FAILED."
    exit 1
fi
