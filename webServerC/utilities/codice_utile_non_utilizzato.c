void send_html(int fd_client_socket, char *mime_type, char *requested_file){
        printf("\n invio html\n");
        
	struct stat st;
        char buf_header[256] ;
        int fd_HTML = 0, tot_byte_read = 0, byte_read = 0 ;
        unsigned char html_buf = NULL ;

	//calcola dim file e invia header
        stat(html_for_resource_not_found, &st) ;
	html_buf = malloc(st.st_size * sizeof(unsigned char));


	 //scrittura header della risposta
        snprintf(buf_header_ok, 256,
                "HTTP/1.1 200 OK\r\n"
                "Content-Type: %s\r\n"
                "Content-Length: %ld\r\n"
                "Connection: keep-alive\r\n"
                "Cache-Control: no-cache\r\n"
                "Date: Sun, 14 Jul 2025 10:23:45 GMT\r\n"
                "Server: Apache/2.4.41 (Ubuntu)\r\n"
                "\r\n",   // doppia riga vuota per separare header da corpo
                mime_type, st.st_size);
        send(fd_client_socket, buf_header_ok, strlen(buf_header_ok), 0) ;



        //apertura del file html
        if((fd_HTML = open(requested_file, O_RDONLY))< 0){
                fprintf(stderr, "Errore %d: %s\n", errno, strerror(errno)) ;
                exit(EXIT_FAILURE);
        }
	//lettura del file richiesto
	while((byte_read = read(fd_html, buf_html + tot_byte_read, sizeof(buf_html))) > 0){
                tot_byte_read += byte_read;
        }
	//invio del file
	send(fd_client_socket, buf_html, strlen(buf_html) , 0) ;
        
	close(fd_HTML);
	free(buf_html);
        printf("fine invio file html");

}

void invia_solo_immagine(int fd_client_socket){
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
                while (sent < byte_read) {                                                                            ssize_t s = send(fd_client_socket, response + sent, byte_read - sent, 0);                                                                                                                   if (s < 0) {
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
