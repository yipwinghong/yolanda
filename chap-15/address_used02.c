//
// Created by shengym on 2019-07-07.
//

#include "lib/common.h"

static int count;

static void sig_int(int signo) {
    printf("\nreceived %d datagrams\n", count);
    exit(0);
}

/**
 * 区别于内核参数 tcp_tw_reuse：
 * tcp_tw_reuse 是内核选项，主要用在连接的发起方。TIME_WAIT 状态的连接创建时间超过 1 秒后，新的连接才可以被复用，注意，这里是连接的发起方；
 * SO_REUSEADDR 是用户态的选项，SO_REUSEADDR 选项用来告诉操作系统内核，如果端口已被占用，但是 TCP 连接状态位于 TIME_WAIT ，可以重用端口。
 *      如果端口忙，而 TCP 处于其他状态，重用端口时依旧得到“Address already in use”的错误信息。注意，这里一般都是连接的服务方。
 *
 * @param argc
 * @param argv
 * @return
 */
int main(int argc, char **argv) {
    int listenfd = socket(AF_INET, SOCK_STREAM, 0), connfd;
    struct sockaddr_in server_addr, client_addr;
    socklen_t client_len = sizeof(client_addr);
    bzero(&server_addr, sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = htonl(INADDR_ANY);
    server_addr.sin_port = htons(SERV_PORT);

    /* 端口重用：处于 TIME_WAIT 状态的连接可以被重用，而不会出现：Address already in use；
     * 设置 SO_REUSEADDR 选项，对于服务器存在多个地址时，允许在不同地址上使用相同的端口提供服务
     */
    int on = 1;
    setsockopt(listenfd, SOL_SOCKET, SO_REUSEADDR, &on, sizeof(on));

    if (bind(listenfd, (struct sockaddr *) &server_addr, sizeof(server_addr)) < 0) {
        error(1, errno, "bind failed ");
    }
    if (listen(listenfd, LISTENQ) < 0) {
        error(1, errno, "listen failed ");
    }
    signal(SIGPIPE, SIG_IGN);
    if ((connfd = accept(listenfd, (struct sockaddr *) &client_addr, &client_len)) < 0) {
        error(1, errno, "bind failed ");
    }

    char message[MAXLINE];
    count = 0;
    for (;;) {
        int n = read(connfd, message, MAXLINE);
        if (n < 0) {
            error(1, errno, "error read");
        } else if (n == 0) {
            error(1, 0, "client closed \n");
        }
        message[n] = 0;
        printf("received %d bytes: %s\n", n, message);
        count++;
    }

}


