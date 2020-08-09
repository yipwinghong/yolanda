#include "lib/common.h"

/* 发送的字节数 */
# define MESSAGE_SIZE 102400

/**
 *
 * @param sockfd
 */
void send_data(int sockfd) {

    /* 初始化了一个长度为 MESSAGE_SIZE 的字符串流 */
    char *query;
    query = malloc(MESSAGE_SIZE + 1);
    for (int i = 0; i < MESSAGE_SIZE; i++) {
        query[i] = 'a';
    }
    query[MESSAGE_SIZE] = '\0';

    const char *cp;
    cp = query;
    size_t remaining = strlen(query);

    /* 循环调用 send 函数将 MESSAGE_SIZE 长度的字符串流发送出去 */
    while (remaining) {
        int n_written = send(sockfd, cp, remaining, 0);
        fprintf(stdout, "send into buffer %ld \n", n_written);
        if (n_written <= 0) {
            error(1, errno, "send failed");
            return;
        }
        remaining -= n_written;
        cp += n_written;
    }
}

/**
 *
 * @param argc
 * @param argv
 * @return
 */
int main(int argc, char **argv) {

    /* 创建了 socket 套接字，调用 connect 向对应服务器端发起连接请求 */
    int sockfd;
    struct sockaddr_in servaddr;
    if (argc != 2) {
        error(1, 0, "usage: tcp_client <IPaddress>");
    }
    sockfd = socket(AF_INET, SOCK_STREAM, 0);

    bzero(&servaddr, sizeof(servaddr));
    servaddr.sin_family = AF_INET;
    servaddr.sin_port = htons(12345);
    inet_pton(AF_INET, argv[1], &servaddr.sin_addr);

    /* 连接建立成功后，调用 send_data 发送数据 */
    int connect_rt = connect(sockfd, (struct sockaddr *) &servaddr, sizeof(servaddr));
    if (connect_rt < 0) {
        error(1, errno, "connect failed ");
    }
    send_data(sockfd);
    exit(0);
}

