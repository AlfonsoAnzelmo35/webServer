#include<stdio.h>
#include <stdlib.h>
#include "server/server.h"
#include "client/client.h"

int main(int argc, char*argv[]){

	int port = atoi(argv[1]) ; 

	start_server(port);
}
