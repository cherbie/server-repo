/**
* Based on code found at https://github.com/mafintosh/echo-servers.c (Copyright (c) 2014 Mathias Buus)
* Copyright 2019 Nicholas Pritchard, Ryan Bunney
**/

/**
 * @brief A simple example of a network server written in C implementing a basic echo
 * 
 * This is a good starting point for observing C-based network code but is by no means complete.
 * We encourage you to use this as a starting point for your project if you're not sure where to start.
 * Food for thought:
 *   - Can we wrap the action of sending ALL of out data and receiving ALL of the data?
 */

/** SERVER
 * socket() : create a socket
 * bind() : bind server to a specific port
 * listen() : start to listen for client connections
 * accept() : blocks until a connection is accepted
 * read()
 * write()
 * close()
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <unistd.h>

#define BUFFER_SIZE 1024

int main (int argc, char *argv[]) {
    if (argc < 2) {
        fprintf(stderr,"Usage: %s [port]\n",argv[0]);
        exit(EXIT_FAILURE);
    }

    int port = atoi(argv[1]);

    int server_fd, client_fd, err, opt_val;
    /**
     * The sockaddr_in structure is used to store addresses for the Internet address family. 
     * Values of this type shall be cast by applications to struct sockaddr for use with socket 
     * functions.
     */
    struct sockaddr_in server, client; 
    char *buf;

    /**
     * Creates a socket:
     * SOCK_STREAM : type provides sequenced, reliable, two-way connection based on byte streams (CONNECTION BASED)
     * protocol (0) : specifies a particular protocol to be used with the socket.
     * @return : "(int) descriptor referencing the socket, -1 on error"
     */
    server_fd = socket(AF_INET, SOCK_STREAM, 0); //create an endpoint for communication


    if (server_fd < 0){
        fprintf(stderr,"Could not create socket\n");
        exit(EXIT_FAILURE);
    }

    /**
     * sockaddr_in server
     * sa_family_t     sin_family   AF_INET. 
     * in_port_t       sin_port     Port number. 
     * struct in_addr  sin_addr     IP address. 
     * note: The sin_port and sin_addr members shall be in network byte order.
     */
    server.sin_family = AF_INET;
    server.sin_port = htons(port); //converts the unsigned short integer hostshort from host byte order to network byte order.
    server.sin_addr.s_addr = htonl(INADDR_ANY); //converts the unsigned integer hostlong from host byte order to network byte order.

    opt_val = 1;

    /**
     * manipulate options for the socket referred to by the file descriptor server_fd.
     * setsockopt(int socket, int level, int option_name, const void *option_value, socklen_t option_len);
     * SOL_SOCKET : constant
     * Level : manipulate options at the socket level SOL_SOCKET
     * SO_REUSEADDR : Indicates that the rules used in validating addresses supplied in a bind(2) call should allow reuse of local addresses
     * opt_val : 
     * @return : 0 = success & -1 on error and errno set appropriately
     */
    setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR, &opt_val, sizeof opt_val);

    /**
     * Bind a name to a socket.
     * Server is made the point of reference when an event occurs at the socket address.
     * sockaddr_in needs to be type casted into sockaddr
     * sizeof() gives the byte size of the server structure;
     */
    err = bind(server_fd, (struct sockaddr *) &server, sizeof(server)); 
    if (err < 0){
        fprintf(stderr,"Could not bind socket\n");
        exit(EXIT_FAILURE);
    } 

    /**
     * listen for connections on a socket.
     * server_fd : file descriptor.
     * 128 : "backlog" -- defines the maximum length to which the queue of
     * pending connections for sockfd may grow.
     */
    err = listen(server_fd, 128);
    if (err < 0){
        fprintf(stderr,"Could not listen on socket\n");
        exit(EXIT_FAILURE);
    } 

    printf("Server is listening on %d\n", port);

    while (true) {
        socklen_t client_len = sizeof(client);

        /**
         * accept a connection on a socket.
         * server_fd : listening socket
         * &client : creates new connected socket
         * @return : returns a new file descriptor referring to that socket
         */
        // Will block until a connection is made
        client_fd = accept(server_fd, (struct sockaddr *) &client, &client_len);

        if (client_fd < 0) {
            fprintf(stderr,"Could not establish new connection\n");
            exit(EXIT_FAILURE);

        } 
        
        /**
        The following while loop contains some basic code that sends messages back and forth
        between a client (e.g. the socket_client.py client). 
        
        The majority of the server 'action' will happen in this part of the code (unless you decide
        to change this dramatically, which is allowed). The following function names/definitions will 
        hopefully provide inspiration for how you can start to build up the functionality of the server. 
        
        parse_message(char *){...} : 
            * This would be a good 'general' function that reads a message from a client and then 
            determines what the required response is; is the client connecting, making a move, etc. 
            * It may be useful having an enum that is used to track what type of client message is received
            (e.g. CONNECT/MOVE etc.) 
            
        send_message() {...}: 
            * This would send responses based on what the client has sent through, or if the server needs 
            to send all clients messages
        
        play_game_round() {...}: Implements the functionality for a round of the game
            * 'Roll' the dice (using a random number generator) and then check if the move made by the user
            is correct
            * update game state depending on success or failure. 
            
        setup_game/teardown_game() {} : 
            * this will set up the initial state of the game (number of rounds, players
            etc.)/ print out final game results and cancel socket connections. 
            
        Accepting multiple connections (we recommend not starting this until after implementing some
        of the basic message parsing/game playing): 
            * Whilst in a while loop
                - Accept a new connection 
                - Create a child process 
                - In the child process (which is associated with client), perform game_playing functionality
                (or read the messages) 
        **/
        
        while (true) {  
            buf = calloc(BUFFER_SIZE, sizeof(char)); // Clear our buffer so we don't accidentally send/print garbage

            /**
             * calls are used to receive messages from a socket.
             * They may be used to receive data on both connectionless and connection-oriented sockets.
             * recv --> normally only used on a connected socket.
             * sock_fd : 
             * @return : number of bytes received or -1 on error.
             */
            int read = recv(client_fd, buf, BUFFER_SIZE, 0);    // Try to read from the incoming client

            if (read < 0){
                fprintf(stderr,"Client read failed\n");
                exit(EXIT_FAILURE);
            }

            printf("[+]CLIENT -> SERVER: %s\n", buf);

            buf[0] = '\0';
            sprintf(buf, "My polite respondance");

            err = send(client_fd, buf, strlen(buf), 0); // Try to send something back
            // printf("Client's message is: %s",buf);
            sleep(5); //Wait 5 seconds

            buf[0] = '\0';
            sprintf(buf, "Let the games begin\n");

            err = send(client_fd, buf, strlen(buf), 0); // Send another thing
            if (err < 0){
                fprintf(stderr,"Client write failed\n");
                exit(EXIT_FAILURE);
            }

            read = recv(client_fd, buf, BUFFER_SIZE, 0); // See if we have a response

            if (read < 0){
                fprintf(stderr,"Client read failed\n");
                exit(EXIT_FAILURE);
            }
            
            /** strstr(buf, "move");
             * finds the first occurrence of the substring.
             * @return : returns a pointer to the first occurrence in haystack of any of the 
             * entire sequence of characters specified in needle, 
             * or a null pointer if the sequence is not present in haystack.
             */
            if (strstr(buf, "move") == NULL) {  // Check if the message contained 'move'
                fprintf(stderr, "Unexpected message, terminating\n");
                exit(EXIT_FAILURE);
            }

            buf[0] = '\0';
            sprintf(buf, "You lose\n");

            err = send(client_fd, buf, strlen(buf), 0); // Send our final response

            if (err < 0){
                    fprintf(stderr,"Client write failed\n");
                    exit(EXIT_FAILURE);
            }

            free(buf);
        }

    }
}
