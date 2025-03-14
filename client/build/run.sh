#!/bin/bash

# example run.sh for cs2hack

# server related params
# -url    <string>   - the url cs2hack server is hosted on
# -ep     <string>   - the endpoint to display data on on the webserver
# -key    <string>   - the key set on server to accept data
# -refresh <value>   - time in ms between updates of player data, which also means time in between requests to server

# aim/trigger related params:
# -port     <value>  - the port qmp can be accessed on (required for aimbot & trigger to work)
# -fov      <value>  - aimbot fov in pixels
# -smooth   <value>  - aimbot smoothness
# -shots    <value>  - maximum of shots aimbot will be enabled for
# -vischeck          - uses vischeck param if added
# -delay    <value>  - time in ms to wait before triggerbot shoots
# -cooldown <value>  - time in ms to wait after triggerbot has shot
# -team              - includes teammates as targets

# required for aimbot:
# -w <value>         - ingame resolution width
# -h <value>         - ingame resolution height

sudo setcap 'CAP_SYS_PTRACE=ep' cs2hack
./cs2hack \
-url https://yourdomain.com \
-key top_secret_key \
-port 1234 \
-refresh 350 \
-fov 20 \
-smooth 6 \
-shots 2 \
-vischeck \
-delay 25 \
-cooldown 80 \
-w 1440 \
-h 1080 \
-ep secret \
