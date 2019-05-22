AUTHORS:
    - CLAYTON HERBST (22245091)
    - FRASER LONERAGAN (22243455)

COMPILING INSTRUCTIONS:
    1. In order to build the server program use the makefile provided. 
    2. Run: "make server" to build the program. 
    3. Further details are outlined at the top of the makefile.

GAME SETTINGS:
    - Navigate to the "./server.h" header file in order to change game characteristics such as:
        + Number of players
        + Number of lives
        + Wait time to connect
        + Wait time to receive move from client
        + etc.
    - The game settings can be adjusted in the #define macros at the top of the header file.
    - The names of the macros are meant to be self explanatory however if you are lost, see below:

MACRO DEFINITIONS:
    - NUM_LIVES            NUMBER OF LIVES EACH PLAYER IS GIVEN.
    - NUM_PLAYERS          NUMBER OF PLAYERS NEEDED FOR GAME TO BE ESTABLISHED.
    - MSG_SIZE             ADJUSTS READ & WRITE BUFFER LENGTH. 14 BYTES IS CONSISTANT WITH THE PROJECT DESCRIPTION.
    - WAIT_TIME_MOVE       MAXIMUM TIME IN SECONDS SERVER WAITS FOR TO RECEIVE CLIENT "MOV" MESSAGE
    - WAIT_TIME_INIT       MAXIMUM TIME IN SECONDS SERVER WAITS FOR CLIENT TO SEND "INIT" MESSAGE
    - WAIT_TIME_CONN       NUMBER OF SECONDS SERVER SLEEPS WAITING FOR CLIENTS TO SEND CONNECTION REQUEST
    - WAIT_TIME_SEND       SECONDS SERVER MUST WAIT FOR CLIENT TCP SOCKET TO BE READY FOR WRITING
    - NUM_DICE             NUMBER OF DICE INVOLVED IN THE GAME