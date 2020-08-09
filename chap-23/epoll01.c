#include  <sys/epoll.h>
#include "lib/common.h"

#define MAXEVENTS 128

char rot13_char(char c) {
    if ((c >= 'a' && c <= 'm') || (c >= 'A' && c <= 'M'))
        return c + 13;
    else if ((c >= 'n' && c <= 'z') || (c >= 'N' && c <= 'Z'))
        return c - 13;
    else
        return c;
}

int main(int argc, char **argv) {
    int listen_fd, socket_fd;
    int n, i;
    int efd;
    struct epoll_event event;
    struct epoll_event *events;

    listen_fd = tcp_nonblocking_server_listen(SERV_PORT);

    // 创建一个 epoll 实例
    efd = epoll_create1(0);
    if (efd == -1) {
        error(1, errno, "epoll create failed");
    }

    event.data.fd = listen_fd;
    // 使用 edge-triggered（边缘触发），默认则为 EPOLLLT（水平触发），select 和 poll 都是水平触发
    // https://github.com/YoungYo/yolanda/blob/master/chap-20/select02.c
    // https://github.com/YoungYo/yolanda/blob/master/chap-21/pollserver03.c
    // 条件触发：只要满足事件的条件，就一直不断地把这个事件传递给用户；
    // 边缘触发：只有第一次满足条件的时候才触发，之后就不会再传递同样的事件。
    // 比如有数据需要读，（只要你一直不调用 read），对于 epoll_wait，前者只 wake 一次，后者会不断触发 wake
    // 由于边缘触发只通知一次，所以应用程序必须把数据读取完，否则会一直收到可读事件。
    event.events = EPOLLIN | EPOLLET;

    // 将监听套接字对应的 I/O 事件进行了注册，在有新的连接建立之后就可以感知到
    if (epoll_ctl(efd, EPOLL_CTL_ADD, listen_fd, &event) == -1) {
        error(1, errno, "epoll_ctl add listen fd failed");
    }

    /* Buffer where events are returned */
    // 为 event 数组分配内存
    events = calloc(MAXEVENTS, sizeof(event));

    // 主循环调用 epoll_wait 函数分发 I/O 事件，epoll_wait 成功返回时通过遍历返回的 event 数组，就直接知道发生的 I/O 事件
    // 由于epoll 返回的是有事件发生的数组，相比起 poll 返回的是准备好的个数，每次返回都要遍历注册的描述符结合数组，尤其是数量越大遍历次数就越多，
    // 抛开阻塞和阻塞 I/O 层面，性能也有明显差异
    while (1) {
        n = epoll_wait(efd, events, MAXEVENTS, -1);
        printf("epoll_wait wakeup\n");
        for (i = 0; i < n; i++) {

            // 处理异常
            if ((events[i].events & EPOLLERR) || (events[i].events & EPOLLHUP) || (!(events[i].events & EPOLLIN))) {
                fprintf(stderr, "epoll error\n");
                close(events[i].data.fd);
                continue;
            }
            // 监听套接字上有事件发生
            else if (listen_fd == events[i].data.fd) {
                struct sockaddr_storage ss;
                socklen_t slen = sizeof(ss);
                // 调用 accept 获取已建立连接
                int fd = accept(listen_fd, (struct sockaddr *) &ss, &slen);
                if (fd < 0) {
                    error(1, errno, "accept failed");
                } else {
                    // 将该连接设置为非阻塞
                    make_nonblocking(fd);
                    event.data.fd = fd;
                    event.events = EPOLLIN | EPOLLET; //edge-triggered
                    // 把已连接套接字的可读事件注册到 epoll 中：使用 event_data 的 fd 字段，将连接套接字存储其中
                    if (epoll_ctl(efd, EPOLL_CTL_ADD, fd, &event) == -1) {
                        error(1, errno, "epoll_ctl add connection fd failed");
                    }
                }
                continue;
            }
            // 处理已连接套接字上的可读事件，读取字节流，编码后回应给客户端
            else {
                socket_fd = events[i].data.fd;
                printf("get event on socket fd == %d \n", socket_fd);
                while (1) {
                    char buf[512];
                    // 由于前面设置了 make_nonblocking，socket_fd 没有数据可读会直接返回 -1，且 errno = EAGAIN
                    if ((n = read(socket_fd, buf, sizeof(buf))) < 0) {
                        if (errno != EAGAIN) {
                            error(1, errno, "read error");
                            close(socket_fd);
                        }
                        // 此处跳出了 while(1) 循环
                        break;
                    } else if (n == 0) {
                        close(socket_fd);
                        break;
                    } else {
                        for (i = 0; i < n; ++i) {
                            buf[i] = rot13_char(buf[i]);
                        }
                        if (write(socket_fd, buf, n) < 0) {
                            error(1, errno, "write error");
                        }
                    }
                }
            }
        }
    }

    free(events);
    close(listen_fd);
}
