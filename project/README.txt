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
12. send "PASS"
13. send "FAIL"
14. handle players number of lives and existence within game lobby
15. handle players unexpectedly leaving game 
16. handle players timeout issue for submitting move.
15. send "VICT"
16. send "LOSER"
17. Create test environment and test many cases.

CURRENTLY:
 ---
 attempting to interpret and send PASS / FAIL message to client.
 need to handle enqueueing the client too.