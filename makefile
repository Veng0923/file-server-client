app: server client

server: server-thread.c bao.h log.h
	gcc -g3 server-thread.c log.c -o server -lpthread

client: client.c bao.h log.h
	gcc -g3 client.c log.c -o client

log: log.c log.h
	gcc -g3 log.c -o log

.PHONY: clean
clean:
	rm -rf server client

