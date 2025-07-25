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


char img_for_resource_not_found[]= "/home/alfonso/webserver_git_C/webServerC/immagini/Grump_GattoIncazzato.jpg" ;
char html_for_resource_not_found[] = "/home/alfonso/webserver_git_C/webServerC/htmls/resource_not_found.html" ;
char file_txt[] = "/home/alfonso/webserver_git_C/webServerC/file.txt" ;


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

char *get_base64Img(const char* path) {
    int fd = open(path, O_RDONLY);
    if (fd < 0) {
        perror("Errore apertura file immagine");
        exit(EXIT_FAILURE);
    }

    // Ottieni la dimensione del file
    off_t file_size = lseek(fd, 0, SEEK_END);
    lseek(fd, 0, SEEK_SET);  // ritorna all'inizio

    // Alloca il buffer dinamico per contenere tutti i byte dell'immagine
    unsigned char *buffer = malloc(file_size);
    if (!buffer) {
        perror("Errore malloc");
        close(fd);
        exit(EXIT_FAILURE);
    }

    // Legge il contenuto dell'immagine
    ssize_t total_read = read(fd, buffer, file_size);
    close(fd);

    if (total_read != file_size) {
        fprintf(stderr, "Errore nella lettura completa del file\n");
        free(buffer);
        exit(EXIT_FAILURE);
    }

    // Codifica in base64
    size_t output_len = 0;
    char *encoded = base64_encode(buffer, file_size, &output_len);

    free(buffer);
    return encoded;
}
//invio della risposta 200
void send_html(int fd_client_socket, char *mime_type, char *requested_file){        
	printf("\n invio html\n");
	struct stat st;
	char buf_header[256] ;
	int fd_HTML = 0, tot_byte_read = 0, byte_read = 0 ;
	unsigned char * html_buf = NULL ;
	char  *buf_html= NULL;

	stat(requested_file, &st) ;
	buf_html = malloc(st.st_size * sizeof(unsigned char));

	snprintf(buf_header, 256,
			"HTTP/1.1 200 OK\r\n"
			"Content-Type: %s\r\n"
			"Content-Length: %ld\r\n"
        	        "Connection: keep-alive\r\n"
                	"Cache-Control: no-cache\r\n"
                	"Date: Sun, 14 Jul 2025 10:23:45 GMT\r\n"
                	"Server: Apache/2.4.41 (Ubuntu)\r\n"
                	"\r\n",   // doppia riga vuota per separare header da corpo
                mime_type, st.st_size);
	send(fd_client_socket, buf_header, strlen(buf_header) , 0) ;
        //apertura del file html
        if((fd_HTML = open(requested_file, O_RDONLY))< 0){
                fprintf(stderr, "Errore %d: %s\n", errno, strerror(errno)) ;
                exit(EXIT_FAILURE);
        }
        //lettura del file richiesto
        while((byte_read = read(fd_HTML, buf_html + tot_byte_read, sizeof(buf_html))) > 0){
                tot_byte_read += byte_read;
	}
	printf("\n%s\n", buf_html) ;
	//invio del file
        send(fd_client_socket, buf_html, strlen(buf_html) , 0) ;

        close(fd_HTML);
        free(buf_html);
        printf("fine invio file html\n");

}

//invio della risposta 404 con immagine
void file_not_found_send_html_2(int fd_client_socket){

	char *base64Img = get_base64Img(img_for_resource_not_found) ;
	printf("\n base 64Img %s\n", base64Img) ;

	int fdHtml = open(html_for_resource_not_found, O_RDONLY);
	if(fdHtml < 0){
 		fprintf(stderr, "Errore %d: %s\n", errno, strerror(errno)) ;
                exit(EXIT_FAILURE);
        }
	//lettura del file html 404
        char buf_html[4096] ;
        int tot_byte_read = 0;
        int byte_read     = 0;
        while((byte_read = read(fdHtml, buf_html + tot_byte_read, sizeof(buf_html))) > 0){
                tot_byte_read += byte_read;
        }
	buf_html[tot_byte_read] = '\0';	
	
	char *placeholder = strstr(buf_html, "{{placeholder}}") ;
	if (!placeholder) {
    		fprintf(stderr, "Placeholder non trovato\n");
		exit(1);
	}


	size_t new_html_size = strlen(buf_html) - strlen("{{placeholder}}") + strlen(base64Img);
	char *final_html = malloc(new_html_size * sizeof(char) + 1) ;
	

	size_t prefix_pos = placeholder - buf_html ;
	//printf("\n prefix_pos - %ld\n", prefix_pos);
	strncpy(final_html, buf_html, prefix_pos) ;
	
	strcat(final_html, base64Img);
	strcat(final_html, placeholder + strlen("{{placeholder}}"));
	
	printf("\n final_html %s\n", final_html) ;	

	//invio header della risposta, con la size del corpo
        struct stat st;
        char buf[256] ;
	long int header_size = 0 ;	
        
	//stat(img_for_resource_not_found, &st) ;
	//header_size += st.st_size;
	
	size_t content_length = strlen(final_html);

	snprintf(buf, 256,
                "HTTP/1.1 404 NOT FOUND\r\n"
                "Content-Type: text/html\r\n"
                "Content-Length: %ld\r\n"
                "Connection: keep-alive\r\n"
                "Cache-Control: no-cache\r\n"
                "Date: Sun, 14 Jul 2025 10:23:45 GMT\r\n"
                "Server: Apache/2.4.41 (Ubuntu)\r\n"
                "\r\n", content_length);

        send(fd_client_socket, buf, strlen(buf) , 0) ;
	
	//invio file html
	send(fd_client_socket, final_html, strlen(final_html), 0);

    	free(final_html);
	

}

int build_http_response(int fd_client_socket, char * request){
	char buf_header_ok[256];
	char * requested_url = NULL, *mime_type = NULL;	
	struct request_file requestfile ;
	int file_descriptor_requested = 0;
	struct stat st;

	//ottieni l'url dalla richiesta
	requested_url = get_requested_Url(request);	
	
	//decodifica dell'url, per permettere al server di accedere alla risors
	//char *decoded_requested_url = decode_url(requested_url);
	
	//trova il file richiesto, e l'estensione
	get_file_fileExtension2(&requestfile, requested_url);
	
	//ottieni il mimetype
	mime_type  = get_mime_type(requestfile.ext);

	//ottieni la risorsa
	file_descriptor_requested = open(requestfile.file, O_RDONLY);
	
	//stampa info
	printf("\n\nnome file : %s -- estensione :%s -- mime type :%s -- fdRequested:%d \n\n", 
				requestfile.file, requestfile.ext, mime_type, file_descriptor_requested);
	
	//error nel file non trovato
	if(file_descriptor_requested < 0)
		file_not_found_send_html_2(fd_client_socket);
	
	//togli queste 2 linee	
	stat(file_txt, &st) ;
	printf("st size:%ld", st.st_size);
	

	//invio della risora richiesta
	send_html(fd_client_socket, mime_type, file_txt) ;
	return 0;	
}
