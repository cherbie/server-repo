#!/bin/bash
#VARIABLES
socket=900
#EXECUTION
clear
xterm -e ./server $socket &
xterm -e ./game.sh &
xterm -e ./game.sh &
xterm -e ./game.sh &
xterm -e ./game.sh &