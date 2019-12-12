Server

$ gcc -o server server.c -lpthread
$ ./server 9999

Client

$ gcc -o client client.c
$ ./client 127.0.0.1 9999 name
