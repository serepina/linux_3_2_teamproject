#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/socket.h>
#include <sys/sendfile.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <fcntl.h>

#define MAXLINE  511

int tcp_connect(int af, char *servip, unsigned short port) {
	struct sockaddr_in servaddr;
	int  s;
	// 소켓 생성
	if ((s = socket(af, SOCK_STREAM, 0)) < 0)
		return -1;
	// 채팅 서버의 소켓주소 구조체 servaddr 초기화
	bzero((char *)&servaddr, sizeof(servaddr));
	servaddr.sin_family = af;
	inet_pton(AF_INET, servip, &servaddr.sin_addr);
	servaddr.sin_port = htons(port);

	// 연결요청
	if (connect(s, (struct sockaddr *)&servaddr, sizeof(servaddr))
		< 0)
		return -1;
	return s;
}

int main(int argc, char *argv[])
{
	struct sockaddr_in server;
	struct stat obj;
	int sock;
	char bufmsg[MAXLINE];
	char buf[100], command[5], filename[MAXLINE], *f;
	char temp[20];
	int k, size, status;
	int filehandle;

	if (argc != 3) {
		printf("사용법 : %s server_ip port\n", argv[0]);
		exit(1);
	}

	sock = tcp_connect(AF_INET, argv[1], atoi(argv[2]));
	if (sock == -1) {
		printf("tcp_connect fail");
		exit(1);
	}

	while (1) {
		printf("\033[1;33m명령어 : get, put, pwd, ls, cd, quit\n");
		printf("\033[1;32mclient> ");
		fgets(bufmsg, MAXLINE, stdin); //명령어 입력
		fprintf(stderr, "\033[97m");   //글자색을 흰색으로 변경
		if (!strcmp(bufmsg, "get\n")) {//get명령어를 입력받았다면
			printf("다운로드할 파일 : ");
			scanf("%s", filename);       //파일 이름 입력
			fgets(temp, MAXLINE, stdin); //버퍼에 남은 엔터 제거
			strcpy(buf, "get ");
			strcat(buf, filename);
			send(sock, buf, 100, 0);         //명령어 전송
			recv(sock, &size, sizeof(int), 0);
			if (!size) {//파일이 없다면
				printf("파일이 없습니다\n");
				continue;
			}
			f = malloc(size);
			recv(sock, f, size, 0);
			while (1) {
				filehandle = open(filename, O_CREAT | O_EXCL | O_WRONLY, 0666);
				if (filehandle == -1) //같은 이름이 있다면 이름 끝에 _1 추가
					sprintf(filename + strlen(filename), "_1");
				else break;
			}
			write(filehandle, f, size, 0);
			close(filehandle);
			printf("다운로드 완료\n");//전송이 잘 되었다면

		}
	}
}
