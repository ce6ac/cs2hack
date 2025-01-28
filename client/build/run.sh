#!/bin/bash

sudo setcap 'CAP_SYS_PTRACE=ep' cs2hack
./cs2hack \
-url https://cs2.sebbe.com/receiver \
-refresh 350 \
-fov 20 \
-smooth 4 \
-shots 2 \
-delay 30 \
-cool 80 \
-w 1440 \
-h 1080 \
-key <key here> \
-pw <pw here> \
