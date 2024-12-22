#!/bin/bash

cd memflow/memflow-win32-ffi/
if sudo cargo build --release ; then
    cd ../memflow-qemu-procfs

    if sudo cargo build --release --all-features ; then
        cd ../../
        make
    else
        echo "error while building memflow-qemu-procfs"
    fi

else
    echo "error while building memflow-win32-ffi"
fi
