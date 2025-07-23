# | binaries si assicura che binaries esistaa ovvero che la cartella sia stata creata, c'è anche il comando

all: link

link: binaries/main.o binaries/server2.o binaries/client.o binaries/utilities.o binaries/base64.o
	gcc binaries/main.o binaries/server2.o binaries/client.o binaries/utilities.o binaries/base64.o -o binaries/webServer.exe


binaries/main.o: | binaries
	gcc -c main.c -o binaries/main.o

binaries/server2.o: | binaries
	gcc -g -c server/server2.c -o binaries/server2.o

binaries/client.o: | binaries
	gcc -g -c client/client.c -o binaries/client.o

binaries/utilities.o: | binaries
	gcc -c utilities/utilities.c -o binaries/utilities.o

binaries/base64.o: | binaries
	gcc -c utilities/base64.c -o binaries/base64.o

binaries:
	mkdir -p binaries

clean:
	rm -f binaries/main.o binaries/server2.o binaries/client.o binaries/utilities.o binaries/base64.o binaries/webServer.exe

