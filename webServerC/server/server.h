#include<stdio.h>
#include<pthread.h>

void start_server(int port); 
void create_thread(pthread_t *thread, int client_socket_fd) ;
char * letturaHeadersRichiesta(int clientSocket_fd);
void scrittura_messaggioClient(int client_socket_fd, char*messaggio);

