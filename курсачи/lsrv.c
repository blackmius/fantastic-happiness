#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <signal.h>
#include <time.h>

#include <sys/types.h> 
#include <sys/socket.h>
#include <arpa/inet.h>

void print_time() {
    char s[1000];
    time_t t = time(NULL);
    struct tm * p = localtime(&t);
    strftime(s, 1000, "%F %T", p);
    printf("%s - ", s);
}

void error(const char *msg) {
    perror(msg);
    exit(1);
}

void pid(int sock) {
    int pid = getppid();
    if (write(sock, &pid, sizeof(int)) < 0) error("ERROR writing to socket");
}
void fd(int sock) {
    if (write(sock, &sock, sizeof(int)) < 0) error("ERROR writing to socket");
}

void resolution(int sock) {
    FILE *p = popen("xdpyinfo | awk '/dimensions/ {print $2}'", "r");
    short w,h;
    fscanf(p, "%hdx%hd", &w, &h);
    if (write(sock, &w, sizeof(short)) < 0) error("ERROR writing to socket");
    if (write(sock, &h, sizeof(short)) < 0) error("ERROR writing to socket");
}

void pixel(int sock) {
    char cmd[128];
    short x,y;
    if (read(sock, &x, sizeof(short)) < 0) error("ERROR reading from socket");
    if (read(sock, &y, sizeof(short)) < 0) error("ERROR reading from socket");
    sprintf(cmd, "xwd -root -silent | convert xwd:- -depth 8 -crop \"1x1+%hd+%hd\" txt:- | grep -om1 '#\\w\\+'", x, y);
    FILE *p = popen(cmd, "r");
    char buf[7];
    fread(buf, 7, 1, p);
    if (write(sock, &buf, 7) < 0)
        error("ERROR writing to socket");
}

void serve(int sock, char *address) {
    while (1) {
        char type;
        if (read(sock, &type, sizeof(char)) < 0) error("ERROR reading from socket");
        if (type < 0) break;
        print_time();
        printf("%s -> %d\n", address, type);
        int current_time = time(NULL);
        if (write(sock, &current_time, sizeof(int)) < 0) error("ERROR writing to socket");
        switch(type) {
            case 1:
                resolution(sock);
                break;
                
            case 2:
                pixel(sock);
                break;
                
            case 3:
                pid(sock);
                break;
                
            case 4:
                fd(sock);
                break;
                
            default:
                fprintf(stderr, "ERROR unknown request type %d\n", type);
                exit(1);
        }
    }
}

int main(int argc, char *argv[]) {
    if (argc < 2) {
        fputs("ERROR no port provided\n", stderr);
        exit(1);
    }

    int port = atoi(argv[1]);
    
    struct sockaddr_in serv_addr, cli_addr;
    socklen_t clen = sizeof(cli_addr);

    memset(&serv_addr, 0, sizeof(serv_addr));
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_addr.s_addr = INADDR_ANY;
    serv_addr.sin_port = htons(port);

    int sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (sockfd < 0) error("ERROR opening socket");
    if (bind(sockfd, (struct sockaddr *) &serv_addr, sizeof(serv_addr)) < 0) {
        close(sockfd);
        error("ERROR on binding");
    }

    listen(sockfd, 5);
    signal(SIGCHLD, SIG_IGN);
    
    printf("Server has successfuly started on port %d\n", port);

    while (1) {
        int newsockfd = accept(sockfd, (struct sockaddr *) &cli_addr, &clen);
        if (newsockfd < 0) error("ERROR on accept");
        char _address[INET6_ADDRSTRLEN];
        inet_ntop(AF_INET, &cli_addr.sin_addr, _address, INET6_ADDRSTRLEN);
        char address[INET6_ADDRSTRLEN+7];
        sprintf(address, "%s:%d", _address, cli_addr.sin_port);
        print_time();
        printf("connected %s\n", address);
        int pid = fork();
        if (pid < 0) error("ERROR on fork");
        if (pid == 0)  {
            serve(newsockfd, address);
            print_time();
            printf("disconnected %s\n", address);
            exit(0);
        } else {
            close(newsockfd);
        }
    }
}
