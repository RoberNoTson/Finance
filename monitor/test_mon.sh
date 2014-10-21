#!/bin/sh
# mon.sh - start watchlist_monitor in its own window

cd /Finance/bin/C/src/monitor

xterm -g 40x20 -T "Test Watchlist Monitor" -e "./watchlist_monitor ./watchlist_monitor.cnf"
