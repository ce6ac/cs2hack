#!/bin/bash

DEPS=./deps

mkdir -p $DEPS

# curl
if [ ! -f "$DEPS/curl/build/lib/libcurl.a" ]; then
    echo "build: fetching curl..."
    git clone --depth=1 https://github.com/curl/curl.git $DEPS/curl
    mkdir -p $DEPS/curl/build && cd $DEPS/curl/build
    cmake .. -DBUILD_SHARED_LIBS=OFF \
     -DCMAKE_BUILD_TYPE=Release \
     -DBUILD_TESTING=OFF \
     -DBUILD_CURL_EXE=OFF \
     -DCURL_USE_OPENSSL=ON \
     -DCURL_USE_LIBSSH2=OFF \
     -DCURL_DISABLE_LDAP=ON \
     -DCURL_DISABLE_LDAPS=ON \
     -DUSE_NGHTTP2=OFF \
     -DUSE_NGTCP2=OFF \
     -DUSE_QUICHE=OFF \
     -DUSE_LIBIDN2=OFF

    make -j$(nproc)
    cd ../../..
    echo "build: curl ready"
else
    echo "build: curl already built, skipping"
fi

# IXWebSocket
if [ ! -f "$DEPS/ixwebsocket/build/libixwebsocket.a" ]; then
    echo "build: fetching IXWebSocket..."
    git clone --depth=1 https://github.com/machinezone/IXWebSocket.git $DEPS/ixwebsocket
    mkdir -p $DEPS/ixwebsocket/build && cd $DEPS/ixwebsocket/build
    cmake .. -DUSE_TLS=1 -DCMAKE_BUILD_TYPE=Release -DUSE_WS=1 -DUSE_TEST=0
    make -j$(nproc)
    cd ../../..
    echo "build: IXWebSocket ready"
else
    echo "build: IXWebSocket already built, skipping"
fi
# memflow
if [ ! -d "$DEPS/memflow" ]; then
    echo "build: fetching memflow (0.1.5)..."
    git clone --depth=1 --branch 0.1.5 \
        https://github.com/memflow/memflow.git $DEPS/memflow
else
    echo "build: memflow already present, skipping"
fi

cd $DEPS/memflow/memflow-win32-ffi
if sudo cargo build --release ; then
    cd ../memflow-qemu-procfs
    if sudo cargo build --release --all-features ; then
        cd ../../..
        echo "build: memflow ready"
    else
        echo "error while building memflow-qemu-procfs"
        exit 1
    fi
else
    echo "error while building memflow-win32-ffi"
    exit 1
fi

# cs2hack client
make