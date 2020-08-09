#include "lib/common.h"

#define MAXLINE 1024


int main(int argc, char **argv) {
    if (argc != 2) {
        error(1, 0, "usage: select01 <IPaddress>");
    }
    int socket_fd = tcp_client(argv[1], SERV_PORT);

    char recv_line[MAXLINE], send_line[MAXLINE];
    fd_set readmask, allreads;
    FD_ZERO(&allreads);
    FD_SET(STDIN_FILENO, &allreads);
    FD_SET(socket_fd, &allreads);

    for (;;) {
        // 每次 select 调用完成后都要重置待测试集合
        readmask = allreads;

        // 待测试的描述符基数，值为最大描述符 + 1，表示为从 0 ~ socket_fd 的 Bitmap：1 为检测、0 为不检测
        if (select(socket_fd + 1, &readmask, NULL, NULL, NULL) <= 0) {
            error(1, errno, "select failed");
        }

        if (FD_ISSET(socket_fd, &readmask)) {
            int n = read(socket_fd, recv_line, MAXLINE);
            if (n < 0) {
                error(1, errno, "read error");
            } else if (n == 0) {
                error(1, 0, "server terminated \n");
            }
            recv_line[n] = 0;
            fputs(recv_line, stdout);
            fputs("\n", stdout);
        }

        if (FD_ISSET(STDIN_FILENO, &readmask)) {
            if (fgets(send_line, MAXLINE, stdin) != NULL) {
                int i = strlen(send_line);
                if (send_line[i - 1] == '\n') {
                    send_line[i - 1] = 0;
                }

                printf("now sending %s\n", send_line);
                ssize_t rt = write(socket_fd, send_line, strlen(send_line));
                if (rt < 0) {
                    error(1, errno, "write failed ");
                }
                printf("send bytes: %zu \n", rt);
            }
        }
    }

}


