#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <unistd.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <signal.h>

#define SOCK_NAME "socket.soc"
#define MSG_MAX_LEN 256
#define PID_MAX_LEN 10

int fd_socket;
void sigint_catcher(int signum)
{
    printf("Ctrl+C caught - closing socket \n");
    close(fd_socket);
    unlink(SOCK_NAME);
    exit(0);
}

int main()
{
    // returns file descriptor
    fd_socket = socket(AF_UNIX, SOCK_DGRAM, 0);
    if (fd_socket < 0)
    {
        printf("Failed socket: %s\n", strerror(errno));
        return -EXIT_FAILURE;
    }
    
    // если ctrl+с, то удалить файл сокета
    signal(SIGINT, sigint_catcher);

    // спец структура для сокетов UNIX
    struct sockaddr_un server_addr;
    server_addr.sun_family = AF_UNIX;
    strcpy(server_addr.sun_path, SOCK_NAME);

    // bind используется сервером для присваивания сокету имени.
    if (bind(fd_socket, (struct sockaddr *)&server_addr, sizeof(server_addr)) < 0)
    {
        printf("Failed bind socket: %s\n", strerror(errno));

        close(fd_socket);
        unlink(SOCK_NAME);
        return -EXIT_FAILURE;
    }

    pid_t client_pid;
    char client_msg[MSG_MAX_LEN];

    int code_error = 0;
    int msg_len = 0;
    while (!code_error)
    {
        if (recvfrom(fd_socket, &client_pid, sizeof(pid_t), 0, NULL, NULL) < 0 ||
            (msg_len = recvfrom(fd_socket, client_msg, MSG_MAX_LEN, 0, NULL, NULL)) < 0)
        {
            printf("Failed receive message: %s\n", strerror(errno));
            code_error = -EXIT_FAILURE;
        }
        else
        {
            client_msg[msg_len] = 0;
            printf("Message received from client %d: %s\n", client_pid, client_msg);
        }
    }
    
    close(fd_socket);
    unlink(SOCK_NAME);
    return code_error;
}