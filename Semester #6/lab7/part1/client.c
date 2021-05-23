#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <unistd.h>

#define SOCKET_NAME "socket.soc"
#define MSG_MAX_LEN 256
#define MSG_PID_LEN 10

int main(int argc, char **argv)
{
    int fd_socket = socket(AF_UNIX, SOCK_DGRAM, 0); // Создает конечную тоxку для связи, 
                                                    // возвращает файловый дескриптор.
                                                    // SOCK_DGRAM поддержка UDP
                                                    // 0 - протокол по умолчанию.
    if (fd_socket < 0)
    {
        printf("Failed socket: %s\n", strerror(errno));
        return -EXIT_FAILURE;
    }

    // адрес сервера 
    struct sockaddr_un server_addr;
    server_addr.sun_family = AF_UNIX;
    strcpy(server_addr.sun_path, SOCKET_NAME);

    // get client pid to send
    pid_t pid = getpid();
    char client_msg[MSG_MAX_LEN];
    while(1)
    {
        printf("Input message from client %d: ", pid);
        fgets(client_msg, MSG_MAX_LEN, stdin);
        printf("\n");

        sendto(fd_socket, &pid, sizeof(pid_t), 0, (struct sockaddr *)&server_addr, sizeof(server_addr));
        sendto(fd_socket, client_msg, strlen(client_msg), 0, (struct sockaddr *)&server_addr, sizeof(server_addr));
    }

    return EXIT_SUCCESS;
}