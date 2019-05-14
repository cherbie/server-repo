#!/bin/bash
#VARIABLES
socket=900
#EXECUTION
clear
xterm -e ./server $socket &
xterm -e ./game $socket &
xterm -e ./game $socket &
xterm -e ./game $socket &