/** CLIENT
 * socket() : create socket and obtain client_fd
 * connect() : 
 * write() : send()
 * read)() : recv()
 * close() :
 * select() : ... when is it possible to send more data
 */

#include "client.h"

int main(int argc, char* argv[])
{
    if (argc < 2) {
        fprintf(stderr,"Usage: %s [port]\n",argv[0]);
        exit(EXIT_FAILURE);
    }

    int port = atoi(argv[1]);
    int client_fd, err, opt_val;
    struct sockaddr_in client; //socket properties
    char *buf;

    //INITIALISE MATCH
    client_fd = socket(AF_INET, SOCK_STREAM, 0); //create an endpoint for communication

    if (client_fd < 0){
        fprintf(stderr,"Could not create socket\n");
        exit(EXIT_FAILURE);
    }

    client.sin_family = AF_INET;
    client.sin_port = htons(port); //converts the unsigned short integer hostshort from host byte order to network byte order.
    client.sin_addr.s_addr = htonl(INADDR_ANY); //converts the unsigned integer hostlong from host byte order to network byte order.

    opt_val = 1;
    setsockopt(client_fd, SOL_SOCKET, SO_REUSEADDR, &opt_val, sizeof opt_val);

    err = connect(client_fd, (struct sockaddr *) &client, sizeof(client));
    if(err < 0) {
        fprintf(stderr, "Could not connect to server.\n");
        exit(EXIT_FAILURE);
    }

    printf("Connected to server on port: %d\n\tclient:\t%d\n", port, client_fd);


    //send message
    buf = calloc(BUFFER_SIZE, sizeof(char));
    
    err = sprintf(buf,"INIT");
    if( err < 0) {
        fprintf(stderr, "Error setting chars in buffer.");
        exit(EXIT_FAILURE);
    }

    err = send(client_fd, buf, BUFFER_SIZE, 0);

    while(true) {
        buf = calloc(BUFFER_SIZE, sizeof(char));

        int read = recv(client_fd, buf, BUFFER_SIZE, 0);
        if(read < 0) {
            fprintf(stderr, "Error reading data from socket into buffer.");
            exit(EXIT_FAILURE);
        }

        printf("[+]SERVER -> CLIENT message: %s\n", buf);
        printf("waiting to receive more messages.\n");
    }
}
