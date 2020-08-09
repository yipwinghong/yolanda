#include "lib/common.h"

int main(int argc, char **argv) {
    if (argc != 2) {
        error(1, 0, "usage: batchwrite <IPaddress>");
    }


    int socket_fd = socket(AF_INET, SOCK_STREAM, 0);
    struct sockaddr_in server_addr;
    bzero(&server_addr, sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(SERV_PORT);
    inet_pton(AF_INET, argv[1], &server_addr.sin_addr);
    if (connect(socket_fd, (struct sockaddr *) &server_addr, sizeof(server_addr)) < 0) {
        error(1, errno, "connect failed ");
    }

    /* Nagle 算法：限制大批量的小数据包同时发送，在任何一个时刻，未被确认的小数据包不能超过一个（因此同一个请求得部分报文可能会被延迟发送） */
    /* 消除 Nagle 算法的副作用（时延敏感应用）：集中写，即在写数据之前，将数据合并到缓冲区，批量发送出去 */
    char buf[128];
    struct iovec iov[2];
    char *send_one = "hello,";
    iov[0].iov_base = send_one;
    iov[0].iov_len = strlen(send_one);
    iov[1].iov_base = buf;
    while (fgets(buf, sizeof(buf), stdin) != NULL) {
        iov[1].iov_len = strlen(buf);
        int n = htonl(iov[1].iov_len);
        if (writev(socket_fd, iov, 2) < 0) {
            error(1, errno, "writev failure");
        }
    }
    exit(0);
}

