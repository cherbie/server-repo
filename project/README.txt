HOW TO TEST:
1. TEST BY RUNNING THE BASH FILE "./server.sh"
2. will need to have xterm command line utility
3. OTHERWISE JUST RUN THE ./server & ./game applications by opening many terminal windows.

PROJECT STATUS:
1. setup server socket -- done
2. set up player strct -- done
3. listen for player connections -- done
4. handle connection timeout
4. send "WELCOME,%d" -- done
5. send "REJECT" -- done
6. start game with players in game -- done
7. send "START,%d,%d" -- done
8. send "CANCEL"
9. handle connections after game has started -- done
10. receive players move -- done
11. process players move -- done
12. roll dice -- done
12. evaluate players move
12. send "PASS" -- done
13. send "FAIL" -- done
14. handle players number of lives and existence within game lobby -- further testing required
15. handle players unexpectedly leaving game -- further testing required
16. handle players timeout issue for submitting move.
15. send "VICT" -- done / further work required
16. send "LOSER" -- done / further work required
17. Keep server open after a round has been played?
17. Create test environment and test many cases.

CURRENTLY:
 ---
    - Need to advance client side game play (improve logging to terminal).
    - Need to send cancel to players if game cannot be established
    - need to implement timer interrupts (pthread)
    - need to decide whether to close all connections after a victor has been announced
    - need to track client connection status throughout the programming by checking all recv calls
    - Need to implement select() to find if a connection attempt is attempting to be made
    - kill when send() function fails.
    - implement sleep(10) to wait for all clients to send their message ... and then quickly cycle through all players
    - how to exit parent of child (fork) with the status of the child
    