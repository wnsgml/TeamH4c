#include <stdio.h>
#include <stdlib.h>
#include <winsock2.h>
#include <sys/stat.h> 

int sendall(int s, char* buf, int* len) //데이터를 잃지않고 그대로 보내기 위한 함수 sendall작성
{
    int total;
    int bytesleft;
    int n;
    total = 0;
    bytesleft = *len;
    while (total < *len)
    {
        n = send(s, buf+total, bytesleft, 0);
        if (n == 1)
        {
            break;
        }
        total = total + n;
        bytesleft = bytesleft - n;
    }
    *len = total;
    return n == -1?-1:0;
}

void req(char* request, int accesss_sockt) //req 함수 선언
{
    char arguments[BUFSIZ];
    strcpy(arguments, "./"); //현재 경로를 인수값에 넣기

    char command[BUFSIZ];
    sscanf(request, "%s%s", command, arguments+2); //sscnaf를 이용해서 command값과 arguments값 분리
    char* extension = "text/html;";
    char* content_type = "text/plain;";
    char* body_length = "content-Length: ";

    FILE* fp = fopen(arguments, "rb"); //arguments 값.//index.html 경로에서 파일을 바이너리 읽기 형식으로 파일오픈
    char* head = "HTTP/1.1 200 OK\r\n";
    int len;
    char ctype[100] = "Content-type:text/html; charset=utf-8;\r\n"; 

    len = strlen(head);
    sendall(accesss_sockt, head, &len);
    len = strlen(ctype);
    sendall(accesss_sockt, ctype, &len);

    struct stat statbuf;
       
    char length_buf[20];
    fstat(fileno(fp), &statbuf); //fileno 함수로 위에서 불러온 rfile 디스크립터 반환 후 그 파일의 정보를 읽음
    itoa(statbuf.st_size, length_buf, 10 ); //정수를 문자열로 변환 statbuf.st_size를 length_buf길이 만큼 10진수로
    send(accesss_sockt, body_length, strlen(body_length), 0); //데이터를 보내야하는 목적지의 소켓 디스크립터, 보내야하는 내용, 내용의 길이, flags 값
    send(accesss_sockt, length_buf, strlen(length_buf), 0);
    send(accesss_sockt, "\n", 1, 0);
    send(accesss_sockt, "\r\n", 2, 0);
 

    char read_buf[1024]; 
    len = fread(read_buf, 1, statbuf.st_size, fp); //rp에서 read_buff의 하나의 크기를 1만큼 statbuf.st_size의 데이터 개수만큼  읽어온다.
    if (sendall(accesss_sockt, read_buf, &len) == -1)
    { 
    printf("error!");   
    }
    fclose(fp);
    return;
}
int main()
{
    WSADATA wsaData;
    if (WSAStartup(MAKEWORD(1,1), &wsaData) != 0)
    {
        exit(1);
    }
    int sockt = socket(PF_INET, SOCK_STREAM, 0); //PF_INET = 프로토콜 체계 SOCK_STREAM = 소켓 타입, 소켓의 타입 지정해줬기 때문에 0값을 써도 무방
    if (sockt == -1)
    {
        return -1;
    }

    struct sockaddr_in server_addr;
    memset(&server_addr,0,sizeof(server_addr)); //값 초기화
    server_addr.sin_family = AF_INET; //타입지정 주소체
    server_addr.sin_port = htons(8080); //포트
    server_addr.sin_addr.s_addr = htonl(INADDR_ANY); //IP

    if (bind(sockt,(struct sockaddr*)&server_addr,sizeof(server_addr)) == -1) //소켓이랑 서버의 정보랑 연결 (소켓 디스크립터, 서버의 IP, 주소길이)
    {
        return -1;
    }
    if (listen(sockt,10) == -1) //연결 대기상태 (소켓 디스크립터, 대기열의 크기)
    {
        return -1;
    }

    while(1)
    {
        printf("WebServer...\n");
        struct sockaddr_in client_sockt;
        int s_size = sizeof(struct sockaddr_in);
        int access_sockt = accept(sockt,(struct sockaddr*)&client_sockt, &s_size);//소켓 디스크립터, 클라이언트 주소를 담을 구조체, 구조체 크기

        char buf [1024];
        if (recv(access_sockt, buf, 1024, 0) == -1)//소켓 디스크립터, 정보를 담을 버퍼, 버퍼의 크기, 플래그값
        {
            exit(1);
        }
        req(buf, access_sockt);
    }
}