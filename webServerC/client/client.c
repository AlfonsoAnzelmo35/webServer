#include <stdio.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <string.h>
#include <unistd.h>
#include <stdlib.h>
#include "client.h"

void lettura_messaggioServer(int server_socket_fd){
        //lettura del messaggio di benvenuto
        char buf[1024] ;
	int n = 0 ;
	if((n = read(server_socket_fd, buf, sizeof(buf) - 1)) < 0){
             	printf("reading problem\n");
        	exit(0);
        }
	printf("---messaggio dal server %s \n", buf);
	buf[n] = '\0';
}

void chat(int server_socket_fd){
	//messaggio di benvenuto
	lettura_messaggioServer(server_socket_fd);

        //scrittura di un messaggio
	int n = 0 ;
	char messaggio[4096];
	while((n = read(STDIN_FILENO, messaggio, sizeof(messaggio) - 1)) > 0){
		messaggio[n] = '\0' ;
		if(write(server_socket_fd, messaggio, strlen(messaggio))< 0) {
			printf("writing message in the chat function problem");
                	exit(0);
		}
		if((strcmp(messaggio,"chiudi chat") == 0)){
                        break;
                }
        }

	//messaggio di addio
	lettura_messaggioServer(server_socket_fd);
}
void start_client(){                                                                                                
	//creazione socket
        int server_socket_fd = 0;
        if((server_socket_fd = socket(AF_INET, SOCK_STREAM, 0)) < 0){
                printf("socket creation error");
        	exit(0);
	} 
        printf("socket created\n");
	
	//connessione ad un server 
        struct sockaddr_in my_socketAddress;
        my_socketAddress.sin_family   = AF_INET;
        my_socketAddress.sin_addr.s_addr       = inet_addr("172.21.28.127");
        my_socketAddress.sin_port   	       = htons(9001) ;	
	if(connect(server_socket_fd, (struct sockaddr*)&my_socketAddress, sizeof(my_socketAddress)) < 0){
		printf("connecting failed");
        	exit(0);
	}
	printf("connessione avvenuta\n");

	chat(server_socket_fd);
        //chiusura del socket
        close(server_socket_fd);
        printf("closing socket bye bye.....\n");
}
