#ifndef __CLIENT_TEST_H__
#define __CLIENT_TEST_H__

#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <unistd.h>
#include <pthread.h>

int main(int argc, char *argv[])
{
    // Declare variables
    int cfd, ufd, sfd, portno, efd;
    struct sockaddr_in serv_addr;
    struct hostent *server;

    char buffer[256];

    // Make sure that server name and port are available in command line arguments
    if (argc < 3)
    {
        fprintf(stderr, "usage %s hostname port\n", argv[0]);
        exit(0);
    }

    // Get port number
    portno = atoi(argv[2]);

    /* --- create mutiple sockets ---*/
    printf("Createing socket... \n");
    if ((sfd = socket(AF_INET, SOCK_STREAM, 0)) < 0)
    {
        perror("ERROR opening socket\n");
        exit(1);
    }

    if ((cfd = socket(AF_INET, SOCK_STREAM, 0)) < 0)
    {
        perror("ERROR opening socket\n");
        close(sfd);
        exit(1);
    }

    if ((ufd = socket(AF_INET, SOCK_STREAM, 0)) < 0)
    {
        perror("ERROR opening socket\n");
        close(cfd);
        close(sfd);
        exit(1);
    }

    /* --- search for server ---*/
    printf("Getting server name... \n");
    if ((server = gethostbyname(argv[1])) == NULL)
    {
        fprintf(stderr, "ERROR, no such host\n");
        close(ufd);
        close(cfd);
        close(sfd);
        exit(0);
    }

    // Populate serv_addr structure
    printf("Creating memory buffer... \n");
    memset(&serv_addr, 0, sizeof(serv_addr));
    serv_addr.sin_family = AF_INET; // Set to AF_INET
    memcpy(&serv_addr.sin_addr.s_addr,
           server->h_addr_list[0], // Set server address
           server->h_length);
    serv_addr.sin_port = htons(portno); // Set port (convert to network byte ordering)

    /* --- connect to server ---*/
    printf("Waiting for connection... \n");
    if (connect(cfd, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) < 0)
    {
        perror("ERROR connecting \n");
        close(ufd);
        close(cfd);
        close(sfd);
        exit(1);
    }

    if (connect(sfd, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) < 0)
    {
        perror("ERROR connecting \n");
        close(ufd);
        close(cfd);
        close(sfd);
        exit(1);
    }

    if (connect(ufd, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) < 0)
    {
        perror("ERROR connecting \n");
        close(ufd);
        close(cfd);
        close(sfd);
        exit(1);
    }

    /* --- get player name ---*/
    char name[15] = "no-name";
    printf("Write a username: ");
    scanf("%s", name);

    /* --- test correct fd connections ---*/
    strcpy(name, "Jacob cfd");
    if ((efd = write(cfd, &name, sizeof(name))) < 0)
    {
        perror("ERROR writing to socket \n");
        close(ufd);
        close(cfd);
        close(sfd);
        exit(1);
    }

    strcpy(name, "Jacob sfd");
    if ((efd = write(sfd, &name, sizeof(name))) < 0)
    {
        perror("ERROR writing to socket \n");
        close(ufd);
        close(cfd);
        close(sfd);
        exit(1);
    }

    strcpy(name, "Jacob ufd");
    if ((efd = write(ufd, &name, sizeof(name))) < 0)
    {
        perror("ERROR writing to socket \n");
        close(ufd);
        close(cfd);
        close(sfd);
        exit(1);
    }
}
#endif // __CLIENT_TEST_H__