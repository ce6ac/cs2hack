#!/bin/bash

sudo setcap 'CAP_SYS_PTRACE=ep' cs2hack
./cs2hack \
-url https://cs2hack.com/receiver \
-port 1337 \
-refresh 350 \
-fov 20 \
-smooth 6 \
-shots 2 \
-delay 25 \
-cool 80 \
-w 1440 \
-h 1080 \
-ep secret \
