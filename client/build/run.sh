#!/bin/bash

sudo setcap 'CAP_SYS_PTRACE=ep' cs2hack
./cs2hack -url https://cs2.sebbe.com/receiver -fov 20 -smooth 3 -shots 2 -w 1440 -h 1080 -refresh 350
