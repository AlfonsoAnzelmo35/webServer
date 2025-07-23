#include<stdio.h>
void scrittura_messaggioClient(int client_socket_fd, char*messaggio);
pthread_t crea_thread(int client_socket_fd) ;
char * getMimeType(char *fileExtention) ;
char * encodeUrl(char *request) ;
void build_http_response(int fd, char * request) ; 
