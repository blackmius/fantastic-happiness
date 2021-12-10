#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <signal.h>
#include <time.h>

#include <sys/types.h> 
#include <sys/socket.h>
#include <arpa/inet.h>

#include <gtk/gtk.h>
#include <gtk/gtkmain.h>

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

void vga(int sock) {
    FILE *p = popen("lspci | grep ' VGA ' | cut -d' ' -f 1 | xargs -i lspci -v -s {}", "r");
    char buf[32];
    int n;
    while ((n = fread(buf, 32, 1, p)) > 0)
        if (write(sock, &buf, 32) < 0)
            error("ERROR writing to socket");
}
void freehmem(int sock) {
    FILE *p = popen("free | grep Mem | awk '{print $3/$2*100}'", "r");
    float percent;
    fscanf(p, "%f", &percent);
    int ipercent = percent*100;
    if (write(sock, &ipercent, sizeof(int)) < 0) error("ERROR writing to socket");
}
void freevmem(int sock) {
    FILE *p = popen("free | grep Swap | awk '{print $3/$2*100}'", "r");
    float percent;
    fscanf(p, "%f", &percent);
    int ipercent = percent*100;
    if (write(sock, &ipercent, sizeof(int)) < 0) error("ERROR writing to socket");
}

gint timer_callback(gpointer window) {
    gtk_window_close(GTK_WINDOW(window));
    return 0;
}

static void activate (GtkApplication* app, gpointer user_data) {
    int time = GPOINTER_TO_INT(user_data);
    
    GtkWidget *window;

    window = gtk_application_window_new(app);
    gtk_window_set_title(GTK_WINDOW(window), "Window");
    gtk_window_set_default_size(GTK_WINDOW (window), 200, 200);
    gtk_widget_show_all(window);
    g_timeout_add(time, timer_callback, window);
}

void window_flick(int sock) {
    short int time;
    if (read(sock, &time, sizeof(short)) < 0) error("ERROR reading from socket");
    
    GtkApplication *app = gtk_application_new("org.gtk.example", G_APPLICATION_FLAGS_NONE);
    g_signal_connect(app, "activate", G_CALLBACK(activate), GINT_TO_POINTER(time));
    int status = g_application_run(G_APPLICATION(app), 0, NULL);
    g_object_unref(app);

    if (write(sock, &status, sizeof(int)) < 0) error("ERROR writing to socket");
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
                vga(sock);
                break;
                
            case 2:
                window_flick(sock);
                break;
                
            case 3:
                freehmem(sock);
                break;
                
            case 4:
                freevmem(sock);
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
