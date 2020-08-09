//
// Created by shengym on 2019-07-12.
//

#include "lib/common.h"


# define NDG 2000    /* datagrams to send */
# define DGLEN 1400    /* length of each datagram */
# define MAXLINE 4096

/**
 * UDP connect 操作：不是通过三次握手连接，而是在应用程序与操作系统内核建立联系（套接字——目标地址+端口）；
 * 当收到一个 ICMP 不可达报文时，操作系统内核可以从映射表中找出拥有该目的地址和端口的套接字，使之可收到“Connection Refused”的信息
 *
 * UDP 套接字每次收发数据都会建立套接字连接：连接套接字->发送报文->断开套接字->连接套接字->发送报文...
 * 如果使用 connect，则每次发送成功后都不需要断开、重建连接：连接套接字->发送报文->发送报文->...->最后断开套接字
 *
 * @param argc
 * @param argv
 * @return
 */
int main(int argc, char **argv) {
    if (argc != 2) {
        error(1, 0, "usage: udpclient2 <IPaddress>");
    }

    int socket_fd = socket(AF_INET, SOCK_DGRAM, 0);

    struct sockaddr_in server_addr;
    bzero(&server_addr, sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(SERV_PORT);
    inet_pton(AF_INET, argv[1], &server_addr.sin_addr);

    /* 将 UDP 套接字和 IPv4 地址绑定 */
    if (connect(socket_fd, (struct sockaddr *) &server_addr, sizeof(server_addr))) {
        error(1, errno, "connect failed");
    }

    char send_line[MAXLINE], recv_line[MAXLINE + 1];
    int n;
    while (fgets(send_line, MAXLINE, stdin) != NULL) {
        int i = strlen(send_line);
        if (send_line[i - 1] == '\n') {
            send_line[i - 1] = 0;
        }

        printf("now sending %s\n", send_line);
        size_t rt = send(socket_fd, send_line, strlen(send_line), 0);
        if (rt < 0) {
            error(1, errno, "send failed ");
        }
        printf("send bytes: %zu \n", rt);

        recv_line[0] = 0;
        n = recv(socket_fd, recv_line, MAXLINE, 0);
        if (n < 0)
            error(1, errno, "recv failed");
        recv_line[n] = 0;
        fputs(recv_line, stdout);
        fputs("\n", stdout);
    }

    exit(0);
}


