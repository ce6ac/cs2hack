#!/bin/bash

# example run.sh for cs2hack

# server related params
# -url      - the url cs2hack server is hosted on
# -key      - the key set on server to accept data
# -refresh  - time in ms between updates of player data, which also means time in between requests to server

# aim/trigger related params
# -port     - the port qmp can be accessed on (required for aimbot & trigger to work)
# -fov      - aimbot fov in pixels
# -smooth   - aimbot smoothness
# -shots    - maximum of shots aimbot will be enabled for
# -delay    - time in ms to wait before triggerbot shoots
# -cooldown - time in ms to wait after triggerbot has shot
# -w        - ingame resolution width (required for aimbot to work)
# -h        - ingame resolution height (required for aimbot to work)

sudo setcap 'CAP_SYS_PTRACE=ep' cs2hack
./cs2hack \
-url https://yourdomain.com \
-key top_secret_key \
-port 1234 \
-refresh 350 \
-fov 20 \
-smooth 6 \
-shots 2 \
-delay 25 \
-cooldown 80 \
-w 1440 \
-h 1080 \
-ep secret \
