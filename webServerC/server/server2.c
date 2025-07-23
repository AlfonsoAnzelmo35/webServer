#include <stdio.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <string.h>
#include <unistd.h>
#include <stdlib.h>
#include <pthread.h>
#include <errno.h>
#include <string.h>
#include "server.h"
#include "../utilities/utilities.h"

typedef struct {
        int fd;
} ParameterThread;

int bufferSize_read = 500 ;

char * letturaHeadersRichiesta(int clientSocket_fd){
	char *buf_headers = malloc(bufferSize_read * sizeof(char));
	int nBytesRead = 0, totBytesRead = 0 ;
	
	printf("--clientSocket_fd %d\n", clientSocket_fd);
	
	// Imposta timeout di 5 secondi sulla read
 	struct timeval timeout;
    	timeout.tv_sec = 5;
    	timeout.tv_usec = 0;
    	if (setsockopt(clientSocket_fd, SOL_SOCKET, SO_RCVTIMEO, &timeout, sizeof(timeout)) < 0) {
        	perror("setsockopt SO_RCVTIMEO");
        	free(buf_headers);
        	exit(EXIT_FAILURE);
    	}


	//lettura headers
	while(1){
		
		if((nBytesRead = read(clientSocket_fd, (buf_headers + totBytesRead), bufferSize_read)) < 0){
			fprintf(stderr, "Errore %d: %s\n", errno, strerror(errno)) ;
                 	exit(EXIT_FAILURE);
		}

		totBytesRead += nBytesRead ;
		buf_headers[totBytesRead] = '\0';  
		
		if(totBytesRead >= bufferSize_read - 1){
			bufferSize_read *= 2;
			buf_headers = realloc(buf_headers, totBytesRead + 500);
		}
		if(nBytesRead == 0){
			break;
		}


		//printf("%d\n", nBytesRead);
		//printf("HEADERS: %s\n", buf_headers);
		if(strlen(buf_headers) != 0) 
			return buf_headers;
		/*if(strstr(buf_headers, "Accept-Language:") != NULL || strstr(buf_headers, "User-Agent:") != NULL ||
			        strstr(buf_headers,"Host:")){
			printf("trovata!");
			return buf_headers;
		}*/

	}
	
	printf(" HEADERS DELLA RICHIESTA : %s \n", buf_headers);
	return buf_headers;
}


void *handle_client(void *arg){
	ParameterThread *parameterThread = (ParameterThread*)arg;
		
	//lettura degli headers della richiesta
	char * request_headers = letturaHeadersRichiesta(parameterThread->fd);
	printf("\n HEADERS DELLA RICHIESTA : %s \n", request_headers);

	//costruzione e invio della risposta
	build_http_response(parameterThread->fd, request_headers);
	


	
	//chiudi il file descriptor del socket del client salvato appena prima che il thread termini.
	free(request_headers);
	
	close(parameterThread->fd) ;
}

void create_thread(pthread_t *thread, int client_socket_fd){
	ParameterThread *parameterThread = malloc(sizeof(ParameterThread));
	parameterThread->fd = client_socket_fd;
		
	
	pthread_create(thread, NULL, handle_client, parameterThread);
}



void start_server(int port){
	//creazione socket
	int server_socket_fd = 0;
       	if((server_socket_fd = socket(AF_INET, SOCK_STREAM, 0)) < 0){
		printf("socket creation error");
		exit(0);
	}
	printf("socket created\n");

	//bind del socket ad un nome
	struct sockaddr_in my_server_socketAddress;
	my_server_socketAddress.sin_family      = AF_INET;
	my_server_socketAddress.sin_addr.s_addr = inet_addr("172.21.28.127");
	my_server_socketAddress.sin_port   	= htons(port) ;
	
	if(bind(server_socket_fd, (struct sockaddr *)&my_server_socketAddress,
			       	sizeof(my_server_socketAddress)) < 0){
		 fprintf(stderr, "Errore %d: %s\n", errno, strerror(errno)) ;
                 exit(EXIT_FAILURE);
	}

	printf("binding success, server 172.21.28.127 on port 9001 ready for listening\n");
	
	while(1){	
		//listen sul socket di client 
	        if(listen(server_socket_fd, 3) < 0){
                 	fprintf(stderr, "Errore %d: %s\n", errno, strerror(errno)) ;
                 	exit(EXIT_FAILURE);	
		}			
		printf("listening......\n");
	
		//accetta connessioni
		int client_socket_fd = 0;
		struct sockaddr myClient_SocketAddress ;
		int client_socket_len = sizeof(myClient_SocketAddress);
		if((client_socket_fd = accept(server_socket_fd, &myClient_SocketAddress, &client_socket_len)) < 0){
			printf("connection accepting problem\n");
			exit(0);
		}
		printf("connessione accettata\n");
		
		//creo il thread
		pthread_t thread ;
		create_thread(&thread, client_socket_fd) ;
			
		if(pthread_join(thread, NULL))
			printf("thread terminato!!\n");
	
	}
	//chiusura del socket
	close(server_socket_fd);
	printf("closing socket bye bye.....\n");


			
}
