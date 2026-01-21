#include "echo_server.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <errno.h>
#include <signal.h>

static echo_server_t *server = NULL;
static volatile int running = 1;

// 设置文件描述符为非阻塞模式
static int set_nonblocking(int fd) {
    int flags = fcntl(fd, F_GETFL, 0);
    if (flags < 0) {
        return -1;
    }
    return fcntl(fd, F_SETFL, flags | O_NONBLOCK);
}

// 处理客户端连接的协程函数
static void client_handler(void *arg) {
    client_conn_t *conn = (client_conn_t *)arg;
    int fd = conn->fd;
    
    printf("[协程] 开始处理客户端连接 fd=%d\n", fd);
    
    while (running) {
        // 接收数据
        ssize_t n = recv(fd, conn->buffer, BUFFER_SIZE - 1, 0);
        
        if (n > 0) {
            conn->buffer[n] = '\0';
            printf("[协程] 从客户端 fd=%d 接收到 %zd 字节: %.*s\n", 
                   fd, n, (int)n, conn->buffer);
            
            // 回显数据
            ssize_t sent = send(fd, conn->buffer, n, 0);
            if (sent < 0) {
                perror("send error");
                break;
            }
            printf("[协程] 向客户端 fd=%d 发送了 %zd 字节\n", fd, sent);
            
            // 让出执行权，允许其他协程运行
            coroutine_yield(coroutine_current());
        } else if (n == 0) {
            // 客户端关闭连接
            printf("[协程] 客户端 fd=%d 关闭连接\n", fd);
            break;
        } else {
            // 错误处理
            if (errno == EAGAIN || errno == EWOULDBLOCK) {
                // 没有数据可读，让出执行权
                coroutine_yield(coroutine_current());
                continue;
            } else {
                perror("recv error");
                break;
            }
        }
    }
    
    // 关闭连接
    close(fd);
    printf("[协程] 关闭客户端连接 fd=%d\n", fd);
    
    // 释放客户端连接结构
    free(conn);
}

// 接受连接的协程函数
static void accept_handler(void *arg) {
    echo_server_t *srv = (echo_server_t *)arg;
    
    printf("[协程] 开始接受连接\n");
    
    while (running) {
        struct sockaddr_in client_addr;
        socklen_t addr_len = sizeof(client_addr);
        
        int client_fd = accept(srv->listen_fd, (struct sockaddr *)&client_addr, &addr_len);
        
        if (client_fd < 0) {
            if (errno == EAGAIN || errno == EWOULDBLOCK) {
                // 没有新连接，让出执行权
                coroutine_yield(coroutine_current());
                continue;
            } else {
                perror("accept error");
                break;
            }
        }
        
        // 设置非阻塞
        if (set_nonblocking(client_fd) < 0) {
            perror("set_nonblocking error");
            close(client_fd);
            continue;
        }
        
        // 添加到 epoll
        struct epoll_event ev;
        ev.events = EPOLLIN | EPOLLET;  // 边缘触发模式
        ev.data.fd = client_fd;
        
        if (epoll_ctl(srv->epoll_fd, EPOLL_CTL_ADD, client_fd, &ev) < 0) {
            perror("epoll_ctl ADD error");
            close(client_fd);
            continue;
        }
        
        printf("[协程] 接受新连接: fd=%d, ip=%s, port=%d\n",
               client_fd, inet_ntoa(client_addr.sin_addr), ntohs(client_addr.sin_port));
        
        // 为每个客户端连接创建协程
        client_conn_t *conn = (client_conn_t *)malloc(sizeof(client_conn_t));
        if (conn == NULL) {
            perror("malloc client_conn error");
            close(client_fd);
            continue;
        }
        
        conn->fd = client_fd;
        conn->recv_len = 0;
        memset(conn->buffer, 0, BUFFER_SIZE);
        
        coroutine_t *co = coroutine_create(client_handler, conn, 64 * 1024);
        if (co == NULL) {
            perror("coroutine_create error");
            free(conn);
            close(client_fd);
            continue;
        }
        
        conn->co = co;
        
        // 启动客户端处理协程
        coroutine_resume(co);
        
        // 让出执行权
        coroutine_yield(coroutine_current());
    }
}

// 信号处理函数
static void signal_handler(int sig) {
    if (sig == SIGINT || sig == SIGTERM) {
        printf("\n收到停止信号，正在关闭服务器...\n");
        running = 0;
    }
}

int echo_server_start(int port) {
    // 创建服务器结构
    server = (echo_server_t *)malloc(sizeof(echo_server_t));
    if (server == NULL) {
        perror("malloc server error");
        return -1;
    }
    
    memset(server, 0, sizeof(echo_server_t));
    server->port = port;
    
    // 创建监听套接字
    server->listen_fd = socket(AF_INET, SOCK_STREAM, 0);
    if (server->listen_fd < 0) {
        perror("socket error");
        free(server);
        return -1;
    }
    
    // 设置 SO_REUSEADDR
    int reuse = 1;
    if (setsockopt(server->listen_fd, SOL_SOCKET, SO_REUSEADDR, &reuse, sizeof(reuse)) < 0) {
        perror("setsockopt error");
        close(server->listen_fd);
        free(server);
        return -1;
    }
    
    // 绑定地址
    struct sockaddr_in server_addr;
    memset(&server_addr, 0, sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = INADDR_ANY;
    server_addr.sin_port = htons(port);
    
    if (bind(server->listen_fd, (struct sockaddr *)&server_addr, sizeof(server_addr)) < 0) {
        perror("bind error");
        close(server->listen_fd);
        free(server);
        return -1;
    }
    
    // 设置非阻塞
    if (set_nonblocking(server->listen_fd) < 0) {
        perror("set_nonblocking error");
        close(server->listen_fd);
        free(server);
        return -1;
    }
    
    // 监听
    if (listen(server->listen_fd, DEFAULT_BACKLOG) < 0) {
        perror("listen error");
        close(server->listen_fd);
        free(server);
        return -1;
    }
    
    // 创建 epoll
    server->epoll_fd = epoll_create1(0);
    if (server->epoll_fd < 0) {
        perror("epoll_create1 error");
        close(server->listen_fd);
        free(server);
        return -1;
    }
    
    // 将监听套接字添加到 epoll
    struct epoll_event ev;
    ev.events = EPOLLIN | EPOLLET;
    ev.data.fd = server->listen_fd;
    
    if (epoll_ctl(server->epoll_fd, EPOLL_CTL_ADD, server->listen_fd, &ev) < 0) {
        perror("epoll_ctl ADD listen_fd error");
        close(server->epoll_fd);
        close(server->listen_fd);
        free(server);
        return -1;
    }
    
    // 注册信号处理
    signal(SIGINT, signal_handler);
    signal(SIGTERM, signal_handler);
    
    printf("=== Echo Server 启动 ===\n");
    printf("监听端口: %d\n", port);
    printf("按 Ctrl+C 停止服务器\n\n");
    
    // 创建接受连接的协程
    server->accept_co = coroutine_create(accept_handler, server, 64 * 1024);
    if (server->accept_co == NULL) {
        perror("coroutine_create accept_handler error");
        close(server->epoll_fd);
        close(server->listen_fd);
        free(server);
        return -1;
    }
    
    // 启动接受连接协程
    coroutine_resume(server->accept_co);
    
    // 主事件循环
    while (running) {
        int nfds = epoll_wait(server->epoll_fd, server->events, MAX_EVENTS, 100);
        
        if (nfds < 0) {
            if (errno == EINTR) {
                continue;
            }
            perror("epoll_wait error");
            break;
        }
        
        // 处理事件（这里主要是触发协程继续执行）
        // 实际的 I/O 操作在协程中进行
        for (int i = 0; i < nfds; i++) {
            if (server->events[i].data.fd == server->listen_fd) {
                // 有新连接，恢复接受连接协程
                if (server->accept_co && server->accept_co->state == COROUTINE_SUSPENDED) {
                    coroutine_resume(server->accept_co);
                }
            } else {
                // 客户端事件，这里可以触发对应的客户端协程
                // 简化实现：所有客户端协程共享事件循环
                // 在实际应用中，可以为每个客户端维护一个协程映射
            }
        }
        
        // 让所有就绪的协程运行
        // 这里简化处理：定期恢复所有挂起的协程
        // 实际应用中应该维护一个就绪协程队列
    }
    
    // 清理资源
    echo_server_stop();
    
    return 0;
}

void echo_server_stop(void) {
    if (server == NULL) {
        return;
    }
    
    printf("\n正在关闭服务器...\n");
    
    if (server->accept_co) {
        coroutine_destroy(server->accept_co);
    }
    
    if (server->epoll_fd >= 0) {
        close(server->epoll_fd);
    }
    
    if (server->listen_fd >= 0) {
        close(server->listen_fd);
    }
    
    free(server);
    server = NULL;
    
    printf("服务器已关闭\n");
}
