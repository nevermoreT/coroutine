#ifndef ECHO_SERVER_H
#define ECHO_SERVER_H

#include "coroutine.h"
#include <sys/epoll.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

// 服务器配置
#define MAX_EVENTS 1024
#define BUFFER_SIZE 4096
#define DEFAULT_PORT 8888
#define DEFAULT_BACKLOG 128

// 客户端连接信息
typedef struct client_conn {
    int fd;                      // 文件描述符
    char buffer[BUFFER_SIZE];    // 接收缓冲区
    ssize_t recv_len;            // 接收到的数据长度
    coroutine_t *co;             // 处理该连接的协程
} client_conn_t;

// 服务器结构
typedef struct echo_server {
    int listen_fd;               // 监听套接字
    int epoll_fd;                // epoll 文件描述符
    int port;                    // 监听端口
    struct epoll_event events[MAX_EVENTS];  // epoll 事件数组
    coroutine_t *accept_co;      // 接受连接的协程
} echo_server_t;

/**
 * 创建并启动 echo server
 * @param port 监听端口
 * @return 0 成功，-1 失败
 */
int echo_server_start(int port);

/**
 * 停止 echo server
 */
void echo_server_stop(void);

#endif // ECHO_SERVER_H
