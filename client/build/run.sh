#!/bin/bash

sudo setcap 'CAP_SYS_PTRACE=ep' cs2hack
./cs2hack
