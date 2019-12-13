#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/socket.h>
#include <sys/sendfile.h>
#include <netinet/in.h>
#include <fcntl.h>

int tcp_listen(int host, int port, int backlog) {
	int sd;
	struct sockaddr_in servaddr;

	sd = socket(AF_INET, SOCK_STREAM, 0);
	if (sd == -1) {
		perror("socket fail");
		exit(1);
	}
	// servaddr 구조체의 내용 세팅
	bzero((char *)&servaddr, sizeof(servaddr));
	servaddr.sin_family = AF_INET;
	servaddr.sin_addr.s_addr = htonl(host);
	servaddr.sin_port = htons(port);
	if (bind(sd, (struct sockaddr *)&servaddr, sizeof(servaddr)) < 0) {
		perror("bind fail");  exit(1);
	}
	// 클라이언트로부터 연결요청을 기다림
	listen(sd, backlog);
	return sd;
}

int main(int argc, char *argv[])
{
	struct sockaddr_in server, client;
	struct stat obj;
	int sock1, sock2;
	char buf[100], command[5], filename[20];
	int k, i, size, len, c;
	int filehandle;

	sock1 = tcp_listen(INADDR_ANY, atoi(argv[1]), 5);

	len = sizeof(client);
	sock2 = accept(sock1, (struct sockaddr*)&client, &len);
	while (1) {
		recv(sock2, buf, 100, 0);
		sscanf(buf, "%s", command);
		if (!strcmp(command, "ls")) {//ls명령어를 입력받았다면
			system("ls >temps.txt");
			stat("temps.txt", &obj);
			size = obj.st_size;
			send(sock2, &size, sizeof(int), 0);
			filehandle = open("temps.txt", O_RDONLY);
			sendfile(sock2, filehandle, NULL, size);
		}
		else if (!strcmp(command, "get")) {//get명령어를 입력받았다면
			sscanf(buf, "%s%s", filename, filename);
			stat(filename, &obj);
			filehandle = open(filename, O_RDONLY);
			size = obj.st_size;
			if (filehandle == -1)
				size = 0;
			send(sock2, &size, sizeof(int), 0);
			if (size)
				sendfile(sock2, filehandle, NULL, size);
		}
	}
}

