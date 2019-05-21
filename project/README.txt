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
8. send "CANCEL" --done
9. handle connections after game has started -- done
10. receive players move -- done
11. process players move -- done
12. roll dice -- done
12. evaluate players move  -- DONE
12. send "PASS" -- done
13. send "FAIL" -- done
14. handle players number of lives and existence within game lobby -- further testing required -- done
15. handle players unexpectedly leaving game -- further testing required -- DONE
16. handle players timeout issue for submitting move. --testing required --DONE
15. send "VICT" -- done
16. send "ELIM" -- done
17. Keep server open after a round has been played. Close after a game is terminated -- DONE
17. Create test environment and test many cases. -- DONE

CURRENTLY:
 ---
    - Need to advance client side game play (improve logging to terminal). --done
    - Need to send cancel to players if game cannot be established -- done
    - need to implement timer interrupts (pthread) -- done -> using select and sleep
    - need to decide whether to close all connections after a victor has been announced -- done : yes all connections closed
    - need to track client connection status throughout the programming by checking all recv calls -- DONE further testing required
    - Need to implement select() to find if a connection attempt is attempting to be made -- done
    - kill when send() function fails? ????????? DONE
         - can either kill connection
         - or attempt to resend after a delay. 
         - ** NOT SPECIFIED HENCE YOU CAN SELECT YOUR PATH AND DESCRIBE YOUR CHOICE THROUGH COMMENTS
    - If recv == 0 close client file descriptor -- further testing required DONE
    - implement sleep(10) to wait for all clients to send their message ... and then quickly cycle through all players -- done(needs to be 30 seconds though)
    - how to exit parent of child (fork) with the status of the child -- done
    - do not need to send pass or fail when VICT or ELIM is sent --done
    - remove all server print statements
    - remove all gets() breaks
    - fix all method descriptions
    - improve all code commmenting
    - improve function return value consistancy
    - write README file to describe who to use server and test server
    - remove number of player check and 3rd commmand line argument requirement
    
    still exists a bug with DOUB move -- BUG FIXED 21 MAY 2019