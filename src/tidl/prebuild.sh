#!/bin/bash
SCRIPT_DIR=$( cd -- "$( dirname -- "${BASH_SOURCE[0]}" )" &> /dev/null && pwd )
tidlc -e -p -l C -i $SCRIPT_DIR/noti_service.tidl -o notification_tidl_proxy

if [ $? -ne 0 ]; then
    echo "*** Failed to generate TIDL code ***"
    exit 1
fi

mv notification_tidl_proxy.h $SCRIPT_DIR/../notification/include/
mv notification_tidl_proxy.c $SCRIPT_DIR/../notification/src/
