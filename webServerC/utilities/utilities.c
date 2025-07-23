#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <errno.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/socket.h>
#include "utilities.h"
#include "base64.h"


char img_for_resource_not_found[]= "/home/alfonso/webServerC/immagini/Grump_GattoIncazzato.jpg" ;
char html_for_resource_not_found[] = "/home/alfonso/webServerC/htmls/resource_not_found.html" ;

struct request_file{
	char file[100];
	char ext[10];
};

void scrittura_messaggioClient(int client_socket_fd, char*messaggio){
        //scrittura di un messaggio
        int n = 0;
        if((n = write(client_socket_fd, messaggio, strlen(messaggio) + 1)) < 0){
                printf("writing message problem\n");
                exit(0);
        }
        *(messaggio + n) = '\0' ;
}

char * get_mime_type(char *fileExtention){
	if(strcmp(fileExtention,"html") == 0 || strcmp(fileExtention,"htm") == 0)
		return "text/html";

	if(strcmp(fileExtention,"txt") == 0) return "text/plain";
	
	if(strcmp(fileExtention,"json") == 0)return "application/json";

}

char * get_requested_Url(char * request){
	char *token = strtok(request, " ");//come un iteratore vai alla prossima stringa
	return strtok(NULL, " ");
	//return strtok(NULL, " ");
}

void get_file_fileExtension(struct request_file *requestfile, char * requested_url){
        char copy[100] ;
        char copy_requested_url[100];
        char *token = strtok(copy_requested_url, "/");
	requestfile->file[0] = '\0';    
	requestfile->ext[0]  = '\0';
	char tmpToken[50] ;

        strcpy(copy_requested_url, requested_url);

        do
        {
            if(token == NULL) strcpy(requestfile->file, tmpToken);        
            
            strcpy(tmpToken, token) ;
            tmpToken[sizeof(tmpToken) - 1] = '\0';       
        }
        while (token = strtok(NULL, "/"));

	token = strtok(tmpToken, ".");
        token = strtok(NULL, ".");
	strcpy(requestfile->ext, token);        
}


char * decode_url(char *requested_url){
	int i = 0;
	char *decoded_url = malloc(200 * sizeof(char));
	strcpy(decoded_url, requested_url);
	
	for(i = 0; i < strlen(requested_url); i++){
		if(requested_url[i] == '%')
			decoded_url[i] = ' ';
		if(requested_url[i] == '%')
                        decoded_url[i] = ':';
	}	
	return decoded_url;
}

void get_file_fileExtension2(struct request_file *requestfile, char * requested_file){
	//rimuovi /, prendi il nome file
	char *token = strtok(requested_file, "/");
	strcpy(requestfile->file, token);

	//prendi l'estensione
	token = strtok(requested_file, ".");
	token = strtok(NULL, ".");
	strcpy(requestfile->ext, token);
}



void file_not_found_send_html(int fd_client_socket){
	printf("\n invio html\n");
	struct stat st;
        char buf[256] ;
        stat(html_for_resource_not_found, &st) ;
	
	snprintf(buf, 256,
                "HTTP/1.1 404 NOT FOUND\r\n"
                "Content-Type: text/html; charset=utf-8\r\n"
                "Content-Length: %ld\r\n"
                "Connection: keep-alive\r\n"
                "Cache-Control: no-cache\r\n"
                "Date: Sun, 14 Jul 2025 10:23:45 GMT\r\n"
                "Server: Apache/2.4.41 (Ubuntu)\r\n"
                "\r\n", st.st_size);
        send(fd_client_socket, buf, strlen(buf) , 0) ;


	//apertura del file html 	
	int fd_HTML = 0 ;
       	if((fd_HTML = open(html_for_resource_not_found, O_RDONLY))< 0){
		fprintf(stderr, "Errore %d: %s\n", errno, strerror(errno)) ;
                exit(EXIT_FAILURE);
	}

	char html_buf[4096];	
	int tot_byte_read = 0;
        int byte_read = 0;
 	while((byte_read = read(fd_HTML, html_buf + tot_byte_read, sizeof(html_buf))) > 0) {
            	
                if(byte_read < 0){
                	fprintf(stderr, "Errore %d: %s\n", errno, strerror(errno)) ;
                        exit(EXIT_FAILURE);
                }

		ssize_t sent = 0;
                while (sent < byte_read) {
                        ssize_t s = send(fd_client_socket, html_buf + sent, byte_read - sent, 0);
                        if (s < 0) {
                                fprintf(stderr, "Errore send: %d: %s\n", errno, strerror(errno));
                                exit(EXIT_FAILURE);
                        }
                        sent += s;
               }//end while
   		break;	 
        }
	close(fd_HTML);
	printf("fine invio file html");
		
}

void file_not_found_send_img(int fd_client_socket){
	printf("\n invio immagine\n");
	//invio header della risposta, con la size del corpo
	struct stat st;	
	char buf[256] ;
	stat(img_for_resource_not_found, &st) ;
	
	snprintf(buf, 256,
		"HTTP/1.1 404 NOT FOUND\r\n"
		"Content-Type: image/jpeg\r\n"
		"Content-Length: %ld\r\n"
		"Connection: keep-alive\r\n"
		"Cache-Control: no-cache\r\n"
		"Date: Sun, 14 Jul 2025 10:23:45 GMT\r\n"
		"Server: Apache/2.4.41 (Ubuntu)\r\n"
		"\r\n", st.st_size);
	send(fd_client_socket, buf, strlen(buf) , 0) ;

	//scrittura dell'immagine	
	int fd_failed;	
	if((fd_failed = open(img_for_resource_not_found, O_RDONLY)) < 0){  
		fprintf(stderr, "Errore %d: %s\n", errno, strerror(errno)) ;
                exit(EXIT_FAILURE);
	}
	char * response = malloc(st.st_size * sizeof(char));	
	int tot_byte_read = 0;
	int byte_read = 0;
	while((byte_read = read(fd_failed,  response + tot_byte_read,
                        st.st_size - tot_byte_read))> 0) {
		ssize_t sent = 0;
		while (sent < byte_read) {
      			ssize_t s = send(fd_client_socket, response + sent, byte_read - sent, 0);
      			if (s < 0) {
          			fprintf(stderr, "Errore send: %d: %s\n", errno, strerror(errno));
            			exit(EXIT_FAILURE);
        		}
        		sent += s;
			
			if(byte_read < 0){
				fprintf(stderr, "Errore %d: %s\n", errno, strerror(errno)) ;
                        	exit(EXIT_FAILURE);
				}
		}//end while
	}
	
 	printf("fine invio file img");
}

char *get_base64Img(char* path){
	printf("\n invio file \n") ;

	//apri l'immagine del file 404
        int fd_failed;
        if((fd_failed = open(img_for_resource_not_found, O_RDONLY)) < 0){
                fprintf(stderr, "Errore %d: %s\n", errno, strerror(errno)) ;
                exit(EXIT_FAILURE);
        }

	//lettura dell'immagine del file 404
	char buf_img[4096] ;
	int tot_byte_read = 0;
        int byte_read     = 0;
        while((byte_read = read(fd_failed, buf_img, sizeof(buf_img))) > 0){
		tot_byte_read += byte_read;	
	}

	//ottenimento dell'immagine in base64
	size_t output_len = 0 ;
	return base64_encode(buf_img, strlen(buf_img), &output_len) ;
	
}
void file_not_found_send_html_2(int fd_client_socket){
	//invio header della risposta, con la size del corpo
        struct stat st;
        char buf[256] ;
	long int header_size = 0 ;
	
        
	stat(img_for_resource_not_found, &st) ;
	header_size += st.st_size;
	
	stat(html_for_resource_not_found, &st) ;
	header_size += st.st_size;

	
	snprintf(buf, 256,
                "HTTP/1.1 404 NOT FOUND\r\n"
                "Content-Type: image/jpeg\r\n"
                "Content-Length: %ld\r\n"
                "Connection: keep-alive\r\n"
                "Cache-Control: no-cache\r\n"
                "Date: Sun, 14 Jul 2025 10:23:45 GMT\r\n"
                "Server: Apache/2.4.41 (Ubuntu)\r\n"
                "\r\n", header_size);

        send(fd_client_socket, buf, strlen(buf) , 0) ;

	char *base64Img = get_base64Img(img_for_resource_not_found) ;

	int fdHtml = open(html_for_resource_not_found, O_RDONLY);
	if(fdHtml < 0){
 		fprintf(stderr, "Errore %d: %s\n", errno, strerror(errno)) ;
                exit(EXIT_FAILURE);
        }
	//lettura del file html 404
        char buf_html[4096] ;
        int tot_byte_read = 0;
        int byte_read     = 0;
        while((byte_read = read(fdHtml, buf_html, sizeof(buf_html))) > 0){
                tot_byte_read += byte_read;
        }
	
	char *placeholder = strstr(buf_html, "{{placeholder}}") ;
	int new_html_size = strlen(buf_html) - strlen("{{placeholder}}") + strlen(base64Img);
	char *final_html = malloc(new_html_size * sizeof(char)) ;
	

	int prefix_pos = placeholder - buf_html ;
	strncpy(final_html, buf_html, prefix_pos) ;
	
	strcat(final_html, base64Img);
	strcat(final_html, placeholder + strlen("{{placeholder}}"));
	
	send(fd_client_socket, final_html, strlen(final_html), 0);

    	free(final_html);
	
}
void build_http_response(int fd, char * request){
	//ottieni l'url dalla richiesta
	char * requested_url  = get_requested_Url(request);	
	
	//decodifica dell'url, per permettere al server di accedere alla risors
	//char *decoded_requested_url = decode_url(requested_url);
	
	//trova il file richiesto, e l'estensione
	struct request_file requestfile ;
	get_file_fileExtension2(&requestfile, requested_url);

	//ottieni il mimetype
	char * mime_type  = get_mime_type(requestfile.ext);

	//ottieni la risorsa
	int file_descriptor_requested = 0;
	file_descriptor_requested = open(requestfile.file, O_RDONLY);
	
	//stampa info
	printf("\n\nnome file : %s -- estensione :%s -- mime type :%s -- fdRequested:%d \n\n", 
				requestfile.file, requestfile.ext, mime_type, file_descriptor_requested);
	
	//error nel file non trovato
	if(file_descriptor_requested < 0){
		file_not_found_send_html_2(fd) ;
		//if(strcmp(mime_type, "text/html") == 0)
		//	file_not_found_send_html(fd) ;
		//else
		//	file_not_found_send_img(fd) ;	
	}

	/*
	//scrittura header della risposta
	snprintf(response, 4096,
		"HTTP/1.1 200 OK\r\n"
		"Content-Type: %s\r\n"
		"Content-Length: 1234\r\n"
		"Connection: keep-alive\r\n"
		"Cache-Control: no-cache\r\n"
		"Date: Sun, 14 Jul 2025 10:23:45 GMT\r\n"
		"Server: Apache/2.4.41 (Ubuntu)\r\n"
		"\r\n",   // doppia riga vuota per separare header da corpo
		mime_type);

	//lettura e stipatura nella risposta
        int nBytesRead = sizeof(response) ;
        while(1){

                nBytesRead = read(file_descriptor_requested, response + *(response + nBytesRead),
				       	sizeof(response)) ;
                
		//printf("byte letti : %d\n\n ",nBytesRead);
                if(nBytesRead < 0){
                        fprintf(stderr, "Errore %d: %s\n", errno, strerror(errno)) ;
                        exit(EXIT_FAILURE);
                }
        }
*/
//	free(decoded_requested_url);	
	
}
